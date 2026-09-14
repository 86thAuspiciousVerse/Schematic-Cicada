import { test } from 'node:test'
import assert from 'node:assert/strict'
import { assessUiSmoke, UI_PAGES } from '../src/ui-smoke.js'

const filled = () => Object.fromEntries(UI_PAGES.map((spec) => [spec.page, { text: 'x'.repeat(40) }]))

test('every page rendering content passes', () => {
  assert.deepEqual(assessUiSmoke(filled()), { ok: true, problems: [] })
})

test('an empty page fails with its name (the blank 运行 page case)', () => {
  const pages = filled()
  pages.run = { text: '运行\n启动阶段、引擎端口与日志尾部。' }
  const verdict = assessUiSmoke(pages)
  assert.equal(verdict.ok, false)
  assert.match(verdict.problems[0], /^run: 渲染为空/)
})

test('a page that threw while rendering reports the error, and missing pages are named', () => {
  const pages = filled()
  pages.symbols = { text: '', error: 'TypeError: dataset is read-only' }
  delete pages.settings
  const verdict = assessUiSmoke(pages)
  assert.deepEqual(verdict.problems, [
    'symbols: TypeError: dataset is read-only',
    'settings: 未采集到',
  ])
})
