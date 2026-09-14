/**
 * Print the MinerU API token from the gitignored credential file.
 *
 * The product reads `mineru.token` from the settings document (the plugin now
 * registers that namespace), but the launcher also exports MINERU_TOKEN so the
 * channel survives a missing/renamed settings entry — the same belt-and-braces
 * the probe launcher has always used. Prints nothing when the file has no token.
 *
 * usage: node mineru-token.mjs <MinerU/api_token.md>
 */
import { readFileSync } from 'node:fs'

const file = process.argv[2]
if (file === undefined) process.exit(0)
try {
  const match = /eyJ[A-Za-z0-9_-]+\.[A-Za-z0-9_-]+\.[A-Za-z0-9_-]+/.exec(readFileSync(file, 'utf8'))
  if (match !== null) process.stdout.write(match[0])
} catch {
  // Missing credential file = the env fallback simply stays unset.
}
