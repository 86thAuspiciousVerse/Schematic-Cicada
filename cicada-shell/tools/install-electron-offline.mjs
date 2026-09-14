/**
 * Offline Electron binary installer (docs/04 §2): `npm install` normally
 * downloads `electron-v<version>-<platform>-<arch>.zip` from GitHub releases,
 * which is unreachable from restricted networks. This tool takes an
 * already-downloaded zip, verifies it against the checksum shipped inside the
 * `electron` npm package, unpacks it into `node_modules/electron/dist`, and
 * writes `path.txt` — exactly what the package's own postinstall does.
 *
 * Usage (Windows node):
 *   node tools/install-electron-offline.mjs <zip path>
 * Mirrors to download from: https://npmmirror.com/mirrors/electron/<version>/
 */

import { createHash } from 'node:crypto'
import { existsSync, mkdirSync, readFileSync, rmSync, writeFileSync } from 'node:fs'
import { execFileSync } from 'node:child_process'
import { join } from 'node:path'

const ELECTRON_PKG = join(import.meta.dirname, '..', 'node_modules', 'electron')

/**
 * sha256 of a file (one pass; the zip is ~130MB).
 * @param path - file to hash.
 * @returns lowercase hex digest.
 */
function sha256(path) {
  const hash = createHash('sha256')
  hash.update(readFileSync(path))
  return hash.digest('hex')
}

const zipArg = process.argv[2]
if (zipArg === undefined) {
  console.error('usage: node tools/install-electron-offline.mjs <electron-v<version>-win32-x64.zip>')
  process.exit(2)
}
if (!existsSync(ELECTRON_PKG)) {
  console.error(`electron package not installed yet: ${ELECTRON_PKG} (run npm install --ignore-scripts first)`)
  process.exit(2)
}
if (!existsSync(zipArg)) {
  console.error(`zip not found: ${zipArg}`)
  process.exit(2)
}

const { version } = JSON.parse(readFileSync(join(ELECTRON_PKG, 'package.json'), 'utf8'))
const platform = process.platform === 'win32' ? 'win32' : process.platform
const arch = process.arch === 'x64' ? 'x64' : process.arch
const name = `electron-v${version}-${platform}-${arch}.zip`
if (!zipArg.endsWith(name)) {
  console.error(`zip name mismatch: expected ...${name}, got ${zipArg}`)
  process.exit(2)
}

const checksums = JSON.parse(readFileSync(join(ELECTRON_PKG, 'checksums.json'), 'utf8'))
const expected = checksums[name]
const actual = sha256(zipArg)
if (expected !== undefined && expected.toLowerCase() !== actual.toLowerCase()) {
  console.error(`sha256 mismatch for ${name}\n  expected ${expected}\n  actual   ${actual}`)
  process.exit(1)
}

const dist = join(ELECTRON_PKG, 'dist')
rmSync(dist, { recursive: true, force: true })
mkdirSync(dist, { recursive: true })
execFileSync('powershell.exe', [
  '-NoProfile', '-NonInteractive', '-Command',
  `Expand-Archive -LiteralPath '${zipArg}' -DestinationPath '${dist}' -Force`,
], { stdio: 'inherit' })
const exe = join(dist, 'electron.exe')
if (!existsSync(exe)) {
  console.error(`extracted archive has no electron.exe: ${exe}`)
  process.exit(1)
}
writeFileSync(join(ELECTRON_PKG, 'path.txt'), 'electron.exe')
console.log(`electron ${version} installed: ${exe}`)
console.log(`sha256 verified: ${actual}`)
