/**
 * Launcher window preload (docs/04 §5.1): the only bridge between the renderer
 * and the main process. Context isolation stays on, Node integration off — the
 * renderer gets exactly this surface and nothing else.
 */

const { contextBridge, ipcRenderer } = require('electron')

contextBridge.exposeInMainWorld('cicada', {
  /** Brand mark as a data URL (no file:// access from the renderer). */
  markSrc: process.argv.find((arg) => arg.startsWith('--cicada-mark='))?.slice('--cicada-mark='.length) ?? '',
  /** `{ projects, problems, config }` — projects merged from the DSH registry and `<home>/projects`. */
  listProjects: () => ipcRenderer.invoke('launcher:list'),
  /** Create `<home>/projects/<name>/`; registration happens on first start. */
  createProject: (name) => ipcRenderer.invoke('launcher:create', name),
  /** Resolved configuration shown on the settings page. */
  settings: () => ipcRenderer.invoke('launcher:settings'),
  /** Start the supervised stack for one project directory. */
  startStack: (path) => ipcRenderer.invoke('launcher:start', path),
  /** Kill the whole child tree. */
  stopStack: () => ipcRenderer.invoke('launcher:stop'),
  /** Focus the Edge product window (or fall back to the page URL). */
  openProduct: () => ipcRenderer.send('launcher:product'),
  /** Reveal a path in Explorer. */
  reveal: (path) => ipcRenderer.send('launcher:reveal', path),
  /** Reveal the launcher log directory. */
  openLogs: () => ipcRenderer.send('launcher:logs'),
  /** Native directory picker → registers the chosen directory as a project. */
  openPicker: () => ipcRenderer.invoke('launcher:pick'),
  /** Datasheet library list (starts the manager service on first call). */
  datasheets: () => ipcRenderer.invoke('launcher:datasheets'),
  /** One datasheet entry: groups, pins, claims, shape and the full.md preview. */
  datasheet: (part) => ipcRenderer.invoke('launcher:datasheet', part),
  /** Move one entry to `<home>/trash/…` (confirmed in the main process). */
  trashDatasheet: (part) => ipcRenderer.invoke('launcher:datasheet-trash', part),
  /** Copy one entry out of the library (destination chosen by the user). */
  exportDatasheet: (part) => ipcRenderer.invoke('launcher:datasheet-export', part),
  /** Reveal the trash directory. */
  openTrash: () => ipcRenderer.send('launcher:trash-open'),
  /** Drop the run console's buffered lines. */
  clearConsole: () => ipcRenderer.send('launcher:console-clear'),
  /** Symbol libraries (curated + user) with engine pin counts. */
  symbols: () => ipcRenderer.invoke('launcher:symbols'),
  /** One symbol: pin geometry from the engine. */
  symbol: (source, key) => ipcRenderer.invoke('launcher:symbol', source, key),
  /** Move a user-library symbol to the trash (curated is read-only). */
  trashSymbol: (key) => ipcRenderer.invoke('launcher:symbol-trash', key),
  /** Copy one symbol file out of a library. */
  exportSymbol: (source, key) => ipcRenderer.invoke('launcher:symbol-export', source, key),
  /** Copy a curated symbol into the user library. */
  saveSymbolToUser: (key) => ipcRenderer.invoke('launcher:symbol-save-to-user', key),
  /** Import a `.kicad_sym` into the user library under the given category. */
  importSymbol: (category) => ipcRenderer.invoke('launcher:symbol-import', category),
  /** Rebuild one datasheet part's symbol from its shape block (engine /lib/synthesize). */
  rebuildSymbol: (part) => ipcRenderer.invoke('launcher:symbol-rebuild', part),
  /** Subscribe to launcher state pushes; returns nothing. */
  onState: (handler) => {
    ipcRenderer.on('shell:state', (_event, state) => handler(state))
  },
})
