const { contextBridge, ipcRenderer } = require('electron/renderer')

contextBridge.exposeInMainWorld('windowAPI', {
  setMsg: (msg) => ipcRenderer.send('window-api', msg)
})