/**
 * Launcher UI smoke (docs/04 §5.1): load the real launcher window OFF-SCREEN in
 * Electron, click through every page and read back what the DOM actually
 * rendered. The unit suite runs the renderer against a DOM stub, which cannot
 * catch a violation of a real-only rule — the empty 运行 page (assigning the
 * read-only `dataset` accessor) was exactly such a case. This closes that gap
 * without showing a window.
 */

import { mkdirSync, writeFileSync } from 'node:fs'
import { dirname, join } from 'node:path'

/** Pages that must render content, with a minimum text length. */
export const UI_PAGES = Object.freeze([
  { page: 'home', min: 20 },
  { page: 'projects', min: 20 },
  { page: 'datasheets', min: 20 },
  { page: 'symbols', min: 20 },
  { page: 'run', min: 20 },
  { page: 'settings', min: 20 },
])

/**
 * Judge the collected page texts.
 * @param pages - `{ [page]: { text, error } }` as read from the window.
 * @returns `{ ok, problems }`; `problems` names every page that rendered nothing.
 */
export function assessUiSmoke(pages) {
  const problems = []
  for (const spec of UI_PAGES) {
    const entry = pages[spec.page]
    if (entry === undefined) {
      problems.push(`${spec.page}: 未采集到`)
      continue
    }
    if (entry.error !== undefined && entry.error !== '') {
      problems.push(`${spec.page}: ${entry.error}`)
      continue
    }
    if (entry.text.trim().length < spec.min) problems.push(`${spec.page}: 渲染为空（${entry.text.trim().length} 字）`)
  }
  return { ok: problems.length === 0, problems }
}

/**
 * Drive one hidden launcher window through every page.
 * @param createWindow - async factory returning `{ window, clickPage, readPage, close }`.
 * @param reportPath - JSON report destination.
 * @param pages - page list (defaults to {@link UI_PAGES}).
 * @returns the report object.
 */
export async function collectUiSmoke(createWindow, reportPath, pages = UI_PAGES) {
  const driver = await createWindow()
  const collected = {}
  try {
    for (const spec of pages) {
      try {
        await driver.clickPage(spec.page)
        collected[spec.page] = { text: await driver.readPage() }
      } catch (error) {
        collected[spec.page] = { text: '', error: String(error?.message ?? error) }
      }
    }
  } finally {
    driver.close()
  }
  const verdict = assessUiSmoke(collected)
  const report = {
    ok: verdict.ok,
    problems: verdict.problems,
    pages: Object.fromEntries(Object.entries(collected).map(([page, entry]) => [
      page,
      { length: entry.text.trim().length, head: entry.text.trim().slice(0, 160), ...(entry.error === undefined ? {} : { error: entry.error }) },
    ])),
  }
  try {
    mkdirSync(dirname(reportPath), { recursive: true })
    writeFileSync(reportPath, `${JSON.stringify(report, null, 2)}\n`)
  } catch {
    // Diagnostics only: a write failure must not mask the verdict.
  }
  return report
}

/** Default report path for a shell directory. */
export function defaultUiReportPath(shellDir) {
  return join(shellDir, 'logs', 'shell-ui-smoke.json')
}
