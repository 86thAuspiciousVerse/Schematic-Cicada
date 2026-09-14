/**
 * Status-window bridge (docs/04 §2.1-2): the renderer is a plain page with no
 * Node access; this CommonJS preload exposes exactly the four actions the
 * status window needs. Sandboxed preloads cannot use ESM, hence `.cjs`.
 */

const { contextBridge, ipcRenderer } = require('electron')

contextBridge.exposeInMainWorld('cicada', {
  /** Subscribe once to status updates pushed from the main process. */
  onState: (listener) => { ipcRenderer.on('shell:state', (_event, state) => listener(state)) },
  /** Open the product page in the default browser (Edge-missing fallback). */
  open: () => ipcRenderer.send('shell:open'),
  /** Reveal `logs/` in Explorer. */
  logs: () => ipcRenderer.send('shell:logs'),
  /** Quit the shell (kills the supervised stack). */
  quit: () => ipcRenderer.send('shell:quit'),
})
