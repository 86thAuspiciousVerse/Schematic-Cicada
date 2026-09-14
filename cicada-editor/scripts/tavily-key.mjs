/**
 * Print the Tavily API key from the gitignored credential file.
 *
 * The file holds the full MCP URL (with `tavilyApiKey=…`); the launcher reads it
 * through this script so the key reaches the MCP client as an ENV VAR and never
 * appears in a tracked cordis.yml (AGENTS §1 red line: credentials never in
 * source, logs or chat). Prints nothing when the file has no key.
 *
 * usage: node tavily-key.mjs <tavily-mcp.txt>
 */
import { readFileSync } from 'node:fs'

const file = process.argv[2]
if (file === undefined) process.exit(0)
try {
  const match = /tvly-[A-Za-z0-9_-]+/.exec(readFileSync(file, 'utf8'))
  if (match !== null) process.stdout.write(match[0])
} catch {
  // Missing credential file = the optional Tavily channel stays off; the
  // product must still start, so this is not an error.
}
