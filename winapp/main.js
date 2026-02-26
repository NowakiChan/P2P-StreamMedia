const { app, BrowserWindow, ipcMain, protocol, net} = require('electron/main')
const isDev = require('electron-is-dev')
const path = require('node:path')
const { webContents,screen,dialog } = require('electron')
const { openFileDialog,downLoadFile,uploadFile,readCacheFile,setCacheFile,setLocalSettings, getSettings } = require('./function/file.js')
const { svrRequest,svrURL } = require('./function/request.js')


const mainWindow = () => {
    const win = new BrowserWindow(
        {
            width: 1000,
            height: 800,
            frame: false,
            resizable: false,
            webPreferences:{
              preload: path.join(__dirname, './function/preload.js'),
              // contextIsolation: true,
              // nodeIntegration: false,
              // sandbox: false
            }
        }
    )

    // const loadUrl = (isDev) ? "http://localhost:3000" : ""
    win.loadURL('http://localhost:3000')
    // win.loadFile('./test.html')

    // if(isDev)
    //     win.loadURL('http://localhost:3000') // main loading entrance
    // else{
    //     // const url = new URL()
    //     win.loadURL(
    //       url.format({
    //         pathname: path.join(__dirname,'./build/index.html'),
    //         protocol: 'file',
    //         slashes: true
    //       }))
    // }
    // webContents.add
    win.webContents.once('did-finish-load', async () => {
      const settings = getSettings('token','tokenUID')
      console.log('Main Thread : Try read token -> ',settings)
      if(settings && settings.token && settings.tokenUID){
        const res = await svrRequest(null,{
          url: `${svrURL}/user/tokenverify`,
          method: 'POST',
          headers:{
            'Content-Type': 'application/json'
          },
          body: JSON.stringify({
            token: settings.token
          })
        })

        win.webContents.send('token-verify',{
          ...res,
          uid: settings.tokenUID
        })
      }
    })

    ipcMain.on('window-api',(event,msg) => {
      console.log(msg)
      if(msg === 'mininum'){
        win.minimize();
        // win.setSkipTaskbar(false)
      }
      else if(msg === 'close'){
        win.close()
      }
      else if(msg === 'fullscreen'){
        const maxwidth = screen.getPrimaryDisplay().workAreaSize.width;
        const maxheight = screen.getPrimaryDisplay().workAreaSize.height;
        win.setResizable(true)
        win.setSize(maxwidth,maxheight)
        win.webContents.send('window-size',{width:maxwidth,height:maxheight})
        win.center()
        win.setResizable(false)
      }
      else if(msg === 'normalsize'){
        win.setResizable(true)
        win.setSize(1000,800)
        win.webContents.send('window-size',{width:1000,height:800})
        win.center()
        win.setResizable(false)
      }
    })
  
    
}


app.whenReady().then(() => {
    // 主进程handle
    ipcMain.handle('operation:openfile', openFileDialog)
    ipcMain.handle('operation:netRequest',svrRequest)
    ipcMain.handle('operation:download',downLoadFile)
    ipcMain.handle('operation:upload',async (_,{url,data}) => {
      return uploadFile(url,data)
    })
    ipcMain.handle('localdata:savesetting',setLocalSettings)
    // 读取设置并检索本地目录
    readCacheFile().then(setting => console.log("Main Thread : Reading Local settings -> ",setting))

    // 注册协议用于处理本地资源
    protocol.registerFileProtocol('local', (request, callback) => {
      const filePath = decodeURIComponent(
        request.url.replace('local://', '')
      );
      console.log(filePath)
      callback({ path: filePath });
    })

    // 启动应用
    mainWindow()

    app.on('activate', () => {
    if (BrowserWindow.getAllWindows().length === 0) {
      createWindow()
    }

  })
})

app.on('before-quit',() => {
  setCacheFile().then(res => {
    if(res) console.log('Main Thread : Successfully save settings')
    else console.log('Main Thread : Failed in save settings')
  })
})

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit()
  }
})

