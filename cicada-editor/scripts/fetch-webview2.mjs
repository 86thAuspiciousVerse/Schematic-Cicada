// K6 构建前置：拉取 Microsoft.Web.WebView2 SDK（NuGet）→ third_party/webview2（include+lib）。
// 用法：cd cicada-editor && node scripts/fetch-webview2.mjs   （需网络；已存在则跳过）
import fs from 'node:fs'
import path from 'node:path'
import { execSync } from 'node:child_process'

const here = process.cwd()
const VERSION = '1.0.2792.45'
const dest = path.join(here, 'third_party', 'webview2')
const nupkg = path.join(dest, 'sdk.nupkg')

if (fs.existsSync(path.join(dest, 'include', 'WebView2.h'))) {
  console.log('[webview2] already present:', dest)
  process.exit(0)
}
fs.mkdirSync(path.join(dest, 'include'), { recursive: true })
fs.mkdirSync(path.join(dest, 'lib'), { recursive: true })
execSync(`curl -sL -o "${nupkg}" "https://www.nuget.org/api/v2/package/Microsoft.Web.WebView2/${VERSION}"`)
console.log('[webview2] downloaded', nupkg, fs.statSync(nupkg).size, 'bytes')
execSync(`tar -xf "${nupkg}" -C "${dest}" build/native/include/WebView2.h build/native/include/WebView2EnvironmentOptions.h build/native/x64/WebView2LoaderStatic.lib`)
fs.renameSync(path.join(dest, 'build', 'native', 'include', 'WebView2.h'), path.join(dest, 'include', 'WebView2.h'))
fs.renameSync(path.join(dest, 'build', 'native', 'include', 'WebView2EnvironmentOptions.h'), path.join(dest, 'include', 'WebView2EnvironmentOptions.h'))
fs.renameSync(path.join(dest, 'build', 'native', 'x64', 'WebView2LoaderStatic.lib'), path.join(dest, 'lib', 'WebView2LoaderStatic.lib'))
fs.rmSync(path.join(dest, 'build'), { recursive: true, force: true })
fs.rmSync(nupkg)
console.log('[webview2] ready:', dest)
