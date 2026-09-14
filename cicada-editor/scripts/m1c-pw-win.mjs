// M1c verify: Playwright (Windows node) drives system Edge headless against
// the cicada host. Run:
//   C:\Program Files\nodejs\node.exe C:\dsh\Schematic-Cicada\cicada-editor\scripts\m1c-pw-win.mjs
import { createRequire } from 'node:module'
import fs from 'node:fs'

const require = createRequire('C:/dsh/Schematic-Cicada/cicada-harness/package.json')
const { chromium } = require('playwright')

const WEB = fs.readFileSync('C:/dsh/Schematic-Cicada/cicada-editor/build-msvc-kicad/web-url.txt', 'utf8').trim()
const OUT = 'C:/dsh/Schematic-Cicada/cicada-editor/build-msvc-kicad/m1c-pw.png'
const EDGE = 'C:/Program Files (x86)/Microsoft/Edge/Application/msedge.exe'
const PROFILE = 'C:/dsh/Schematic-Cicada/cicada-editor/build-msvc-kicad/pw-profile'

const context = await chromium.launchPersistentContext(PROFILE, {
  executablePath: EDGE,
  headless: true,
  viewport: { width: 1500, height: 950 },
  args: ['--disable-gpu', '--no-first-run', '--disable-extensions'],
})
const page = context.pages()[0] ?? await context.newPage()
await page.goto(`${WEB}&layout=workspace`, { waitUntil: 'load', timeout: 30000 })
await page.waitForTimeout(4000)
await page.screenshot({ path: OUT })
await context.close()
console.log('SHOT-OK ' + OUT)
