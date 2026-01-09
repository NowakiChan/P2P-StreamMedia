const { app, BrowserWindow, ipcMain} = require('electron/main')
const isDev = require('electron-is-dev')
const path = require('node:path')
const { webContents,screen } = require('electron')


const mainWindow = () => {
    const win = new BrowserWindow(
        {
            width: 1000,
            height: 800,
            frame: false,
            resizable: false,
            webPreferences:{
              preload: path.join(__dirname, './function/window.js'),
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
    webContents.add

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
        win.webContents.send('window-height',maxheight)
        win.center()
        win.setResizable(false)
      }
      else if(msg === 'normalsize'){
        win.setResizable(true)
        win.setSize(1000,800)
        win.webContents.send('window-height',800)
        win.center()
        win.setResizable(false)
      }
    })
}


app.whenReady().then(() => {
    mainWindow()

    app.on('activate', () => {
    if (BrowserWindow.getAllWindows().length === 0) {
      createWindow()
    }
  })
})

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit()
  }
})

