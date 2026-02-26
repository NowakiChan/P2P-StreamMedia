const { contextBridge, ipcRenderer } = require('electron/renderer')
// const { svrURL } = require('./request.js')

contextBridge.exposeInMainWorld('fileAPI',{
    openFileDialog : () => ipcRenderer.invoke('operation:openfile'),
    saveSetting: (settings) => ipcRenderer.invoke('localdata:savesetting',settings),
})

contextBridge.exposeInMainWorld('windowAPI', {
  setMsg: (msg) => ipcRenderer.send('window-api', msg),
  getWindowHeight: (callback) => ipcRenderer.once('window-height',(_event,value) => callback(value)),
  getWindowSize: (callback) => ipcRenderer.once('window-size',(_,size) => callback(size))
})

contextBridge.exposeInMainWorld('svrAPI',{
  url: 'http://101.132.162.15:6001',
  request: (option) => ipcRenderer.invoke('operation:netRequest',option),
  download: (url) => ipcRenderer.invoke('operation:download',url),
  upload: (url,data) => ipcRenderer.invoke('operation:upload',{url,data}),
  tokenVerify: (callback) => {
    ipcRenderer.once('token-verify',(_,data) => { callback(data) })
  }
})