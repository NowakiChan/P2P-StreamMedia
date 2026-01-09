const { contextBridge, ipcRenderer } = require('electron/renderer')
const { BaseWindow } = require('electron')

contextBridge.exposeInMainWorld('windowAPI', {
  setMsg: (msg) => ipcRenderer.send('window-api', msg),
  getWindowHeight: (callback) => ipcRenderer.on('window-height',(_event,value) => callback(value))
})