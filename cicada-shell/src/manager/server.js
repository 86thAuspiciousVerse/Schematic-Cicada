/**
 * Datasheet manager service (docs/04 §5.5): a short-lived local HTTP process the
 * launcher window spawns on demand.
 *
 * Why a process at all: the library reader must stay the SINGLE authority
 * (`GlobalDatasheetDb` in `@deepseek-ai/dsh-cicada-knowledge`), and that module
 * is TypeScript in the harness workspace — so the manager runs on the real Node
 * with the tsx loader (never Electron's Node, docs/04 §2.1) and imports it.
 *
 * argv: --home <dir> --knowledge <file-url> --token <t> [--port 0]
 *        [--lib-dir <dir>] [--user-lib-dir <dir>] [--engine <exe>] [--electrical <file-url>]
 * The library paths / engine binary are optional: without them the symbol page
 * reports "not configured" instead of guessing.
 * stdout: one `cicada-manager: 127.0.0.1:<port>` line once listening.
 * Security: loopback only, random port, one token per launch, and the entry name
 * from the wire is validated as a single path segment (core.checkPartName).
 */

import { createServer } from 'node:http'
import { join } from 'node:path'
import { pathToFileURL } from 'node:url'

import { checkPartName, exportDatasheet, listDatasheets, readDatasheet, trashDatasheet } from './core.js'
import {
  checkSymbolKey, exportSymbol, findSymbol, importSymbol, listSymbols, rebuildBlock, saveSymbolToUserLib, trashSymbol,
} from './symbols.js'
import { createEngineSession } from '../engine-client.js'

/**
 * Read one `--flag value` pair from argv.
 * @param flag - flag name including dashes.
 * @param fallback - value when the flag is absent.
 * @returns the value.
 */
function arg(flag, fallback = '') {
  const index = process.argv.indexOf(flag)
  if (index < 0 || index + 1 >= process.argv.length) return fallback
  return process.argv[index + 1]
}

const home = arg('--home')
const knowledgeUrl = arg('--knowledge')
const token = arg('--token')
const wantPort = Number(arg('--port', '0'))
const libDir = arg('--lib-dir')
const userLibDir = arg('--user-lib-dir')
const engineExe = arg('--engine')
const electricalUrl = arg('--electrical')

if (home === '' || knowledgeUrl === '' || token === '') {
  process.stderr.write('manager: --home, --knowledge and --token are required\n')
  process.exit(2)
}

/**
 * Load the datasheet database reader from the harness workspace. The URL is
 * computed from the shell's root (never a machine literal) and passed in argv.
 * @returns the `GlobalDatasheetDb` instance.
 */
async function loadDb() {
  const module = await import(knowledgeUrl)
  if (typeof module.GlobalDatasheetDb !== 'function') {
    // A harness that no longer exports the class would silently downgrade this
    // service to "no library"; fail loudly instead.
    throw new Error('GlobalDatasheetDb is not exported by the knowledge package')
  }
  return new module.GlobalDatasheetDb(home)
}

const db = await loadDb()
const trashDir = join(home, 'trash')

// The datasheet→engine electrical mapping has one implementation
// (`cicada-symbols`); this service imports it rather than restating the rules.
const { electricalToEngine } = electricalUrl === ''
  ? { electricalToEngine: (value) => value ?? 'passive' }
  : await import(electricalUrl)

// Symbol geometry is engine-owned (docs/04 §5.4): the browse engine starts on
// the first request that needs pins and stops again after an idle period.
const engine = engineExe === '' || libDir === ''
  ? null
  : createEngineSession({ engineExe, engineDir: join(engineExe, '..'), libDir, userLibDir: userLibDir === '' ? join(home, 'cicada-user-lib') : userLibDir })

/**
 * Merge the file-level library listing with the engine's view: the filesystem
 * says which files exist (and their size/time), the engine says how many pins a
 * symbol has and what its canonical key is.
 * @returns the symbol page payload.
 */
async function symbolListing() {
  if (libDir === '') {
    return { ok: false, curated: [], user: [], problems: ['管理服务未配置符号库路径（--lib-dir）'], engine: false }
  }
  if (userLibDir === '') {
    return { ok: false, curated: [], user: [], problems: ['管理服务未配置用户库路径（--user-lib-dir）'], engine: false }
  }
  const listed = listSymbols(libDir, userLibDir)
  const catalog = engine === null ? { ok: false, data: undefined } : await engine.request('/lib/list')
  const pinsByKey = new Map()
  for (const item of catalog.data?.symbols ?? []) {
    if (typeof item.libId === 'string') pinsByKey.set(item.libId, item.pins ?? 0)
  }
  const decorate = (entry) => ({
    ...entry,
    pins: pinsByKey.get(entry.key) ?? null,
    registered: pinsByKey.has(entry.key),
  })
  return {
    ok: true,
    curated: listed.curated.map(decorate),
    user: listed.user.map(decorate),
    problems: listed.problems,
    engine: catalog.ok === true,
  }
}

/** Write one JSON response. */
function send(res, status, payload) {
  const body = JSON.stringify(payload)
  res.writeHead(status, { 'Content-Type': 'application/json; charset=utf-8', 'Content-Length': Buffer.byteLength(body) })
  res.end(body)
}

/** Read a JSON request body (empty body → `{}`). */
async function body(req) {
  const chunks = []
  for await (const chunk of req) chunks.push(chunk)
  if (chunks.length === 0) return {}
  return JSON.parse(Buffer.concat(chunks).toString('utf8'))
}

const server = createServer((req, res) => {
  void (async () => {
    if (req.headers['x-cicada-manager-token'] !== token) {
      send(res, 403, { error: 'forbidden' })
      return
    }
    const url = new URL(req.url ?? '/', 'http://127.0.0.1')
    const segments = url.pathname.split('/').filter((part) => part !== '')
    try {
      if (req.method === 'GET' && url.pathname === '/health') {
        send(res, 200, { ok: true })
        return
      }
      if (req.method === 'GET' && url.pathname === '/symbols') {
        send(res, 200, await symbolListing())
        return
      }
      if (req.method === 'POST' && url.pathname === '/symbols/import') {
        const payload = await body(req)
        const imported = importSymbol(String(payload.sourceFile ?? ''), userLibDir, String(payload.category ?? ''), {
          overwrite: payload.overwrite === true,
        })
        if (!imported.ok) {
          send(res, imported.conflict ? 409 : 400, { error: imported.conflict ? 'conflict' : 'bad_request', message: imported.problem, key: imported.key })
          return
        }
        // The browse engine read the library at boot: restart it so the imported
        // symbol is visible without reopening the window.
        engine?.stop()
        send(res, 200, { ok: true, key: imported.key, target: imported.target })
        return
      }
      if (segments[0] === 'symbols' && segments.length >= 3) {
        const source = segments[1]
        if (source !== 'curated' && source !== 'user') {
          send(res, 400, { error: 'bad_request', message: 'source 必须是 curated 或 user' })
          return
        }
        const key = decodeURIComponent(segments[2])
        const problem = checkSymbolKey(key)
        if (problem !== '') {
          send(res, 400, { error: 'bad_request', message: problem })
          return
        }
        const entry = findSymbol(libDir, userLibDir, source, key)
        if (req.method === 'GET' && segments.length === 3) {
          if (entry === undefined) {
            send(res, 404, { error: 'not_found', message: `符号不存在：${key}` })
            return
          }
          const detail = engine === null ? { ok: false } : await engine.request('/lib/get', { libId: key })
          send(res, 200, {
            symbol: { ...entry, pins: (detail.data?.pins ?? []).length, registered: detail.ok === true },
            pins: (detail.data?.pins ?? []).map((pin) => ({
              number: pin.number ?? '', name: pin.name ?? '',
              x: pin.x ?? 0, y: pin.y ?? 0, angle: pin.angle ?? 0,
            })),
            engineKey: detail.data?.libId ?? '',
            engineProblem: detail.ok ? '' : detail.problem,
          })
          return
        }
        if (req.method === 'POST' && segments.length === 4 && segments[3] === 'trash') {
          if (source !== 'user') {
            send(res, 400, { error: 'bad_request', message: '精选库是只读的，不能删除' })
            return
          }
          send(res, 200, trashSymbol(userLibDir, trashDir, key))
          return
        }
        if (req.method === 'POST' && segments.length === 4 && segments[3] === 'export') {
          const payload = await body(req)
          send(res, 200, exportSymbol(entry, String(payload.targetDir ?? '')))
          return
        }
        if (req.method === 'POST' && segments.length === 4 && segments[3] === 'save-to-user') {
          send(res, 200, source === 'curated'
            ? saveSymbolToUserLib(entry, userLibDir)
            : { ok: false, target: '', problem: '该符号已在用户库中' })
          return
        }
        send(res, 404, { error: 'not_found' })
        return
      }
      if (req.method === 'GET' && url.pathname === '/datasheets') {
        send(res, 200, listDatasheets(home, db))
        return
      }
      if (segments[0] !== 'datasheets' || segments.length < 2) {
        send(res, 404, { error: 'not_found' })
        return
      }
      const part = decodeURIComponent(segments[1])
      const problem = checkPartName(part)
      if (problem !== '') {
        send(res, 400, { error: 'bad_request', message: problem })
        return
      }
      if (req.method === 'GET' && segments.length === 2) {
        const view = readDatasheet(home, db, part)
        if (view === undefined) {
          send(res, 404, { error: 'not_found', message: `条目不存在：${part}` })
          return
        }
        send(res, 200, view)
        return
      }
      if (req.method === 'POST' && segments.length === 3 && segments[2] === 'trash') {
        send(res, 200, trashDatasheet(home, trashDir, part))
        return
      }
      if (req.method === 'POST' && segments.length === 3 && segments[2] === 'rebuild') {
        // Datasheet shape block → engine `/lib/synthesize` → user library. The
        // symbol's key is the engine's rule (`IC:<part>`), not something the
        // shell may invent.
        const built = rebuildBlock(part, db, electricalToEngine)
        if (!built.ok) {
          send(res, 400, { error: 'bad_request', message: built.problem })
          return
        }
        const synthesized = engine === null
          ? { ok: false, problem: '管理服务未配置引擎（--engine）' }
          : await engine.request('/lib/synthesize', built.block)
        if (!synthesized.ok) {
          send(res, 502, { error: 'engine', message: synthesized.problem })
          return
        }
        send(res, 200, { ok: true, key: synthesized.data?.libId ?? '', warnings: synthesized.data?.warnings ?? [] })
        return
      }
      if (req.method === 'POST' && segments.length === 3 && segments[2] === 'export') {
        const payload = await body(req)
        send(res, 200, exportDatasheet(home, String(payload.targetDir ?? ''), part))
        return
      }
      send(res, 404, { error: 'not_found' })
    } catch (error) {
      send(res, 500, { error: 'internal', message: String(error?.message ?? error) })
    }
  })()
})

server.listen(wantPort, '127.0.0.1', () => {
  const address = server.address()
  const port = typeof address === 'object' && address !== null ? address.port : 0
  process.stdout.write(`cicada-manager: 127.0.0.1:${port}\n`)
})

// The shell owns this process: when it goes away our stdin closes, and the
// service must not outlive it (an orphan would hold the library read lock on
// Windows and confuse the next launch).
process.stdin.on('end', () => { engine?.stop(); server.close(() => process.exit(0)) })
process.stdin.on('close', () => { engine?.stop(); server.close(() => process.exit(0)) })
process.stdin.resume()
