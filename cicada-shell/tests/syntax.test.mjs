import { test } from 'node:test'
import assert from 'node:assert/strict'
import { execFileSync } from 'node:child_process'
import { readdirSync, readFileSync } from 'node:fs'
import { join } from 'node:path'
import { SHELL_DIR } from '../src/config.js'

/**
 * The Electron main process and the renderer are never imported by the other
 * suites (they need Electron / a DOM), so a syntax error there would only show
 * up when a user double-clicks the shell — `node --check` closes that gap.
 */
const SRC = join(SHELL_DIR, 'src')

test('every shell source file parses', () => {
  const files = readdirSync(SRC).filter((name) => /\.(js|cjs|mjs)$/.test(name))
  assert.ok(files.length >= 6, `expected the shell sources, saw ${files.join(', ')}`)
  for (const name of files) {
    assert.doesNotThrow(
      () => execFileSync(process.execPath, ['--check', join(SRC, name)], { stdio: 'pipe' }),
      `${name} must parse`,
    )
  }
})

test('the launcher window loads files that exist', () => {
  const html = readFileSync(join(SRC, 'launcher.html'), 'utf8')
  const referenced = [...html.matchAll(/(?:src|href)="\.\/([^"]+)"/g)].map((match) => match[1])
  assert.ok(referenced.length > 0, 'the launcher page must reference its renderer')
  const present = new Set(readdirSync(SRC))
  for (const file of referenced) assert.ok(present.has(file), `launcher.html references missing ${file}`)
})

test('every enabled nav entry has a renderer branch, every placeholder is marked', () => {
  const html = readFileSync(join(SRC, 'launcher.html'), 'utf8')
  const ui = readFileSync(join(SRC, 'launcher-ui.js'), 'utf8')
  const entries = [...html.matchAll(/<button data-page="([a-z]+)"([^>]*)>/g)]
    .map((match) => ({ page: match[1], disabled: match[2].includes('disabled') }))
  const renderPages = [...ui.matchAll(/page === '([a-z]+)'/g)].map((match) => match[1])
  assert.ok(entries.length >= 6, 'the launcher nav must list every planned page')
  for (const entry of entries) {
    const rendered = renderPages.includes(entry.page)
    // A page that is neither rendered nor marked as a placeholder is a silent
    // dead button — exactly what the L2/L3 rows must not become.
    assert.ok(rendered || entry.disabled,
      `nav entry "${entry.page}" neither renders nor is marked disabled`)
    if (rendered) assert.ok(!entry.disabled, `nav entry "${entry.page}" renders but is disabled`)
  }
  for (const page of renderPages) {
    assert.ok(entries.some((entry) => entry.page === page), `renderer renders "${page}" with no nav entry`)
  }
})
