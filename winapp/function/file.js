const { dialog, app } = require('electron')
const FormData = require('form-data')
const axios = require('axios')
const isDev = require('electron-is-dev')
const path = require('node:path')
const fs = require('fs')

const localCacheFile = '.settings.json'
let localSettings = null

async function openFileDialog(){
    const {canceled,filePaths} = await dialog.showOpenDialog({title:'选择图片',
                                                            filters:[{name:'Images',extensions:['jpg','png','gif']}],
                                                            properties:['openFile']})

    
    if(!canceled){
        const replacepath = 'local:///'.concat(filePaths[0].replaceAll('\\','/'))
        return replacepath;
    }
    else return '';
}

async function readCacheFile(){
    const dataPath = (isDev) ? path.join(__dirname,`../testfile/${localCacheFile}`)
                         : path.join(app.getPath('userData'),localCacheFile)
    
    if(!fs.existsSync(dataPath)){
        console.log('No cache file found,create new')
        fs.writeFileSync(dataPath,JSON.stringify({}),'utf-8')
        return null
    }
    
    console.log('Read current cache file')
    const data = fs.readFileSync(dataPath,'utf-8')
    localSettings = JSON.parse(data)
    return localSettings
}

function setLocalSettings(_,data){
    localSettings = {
        ...localSettings,
        ...data
    }
}

async function setCacheFile(){
    const dataPath = (isDev) ? path.join(__dirname,`../testfile/${localCacheFile}`)
                         : path.join(app.getPath('userData'),localCacheFile)
    console.log('Read and Set Cache File With -> ',localSettings)
    // const cache = readCacheFile()
    // const newCache = {
    //     ...cache,
    //     ...data
    // }
    try{
        fs.writeFileSync(dataPath,JSON.stringify(localSettings),'utf-8')
        return true
    }
    catch(e){
        console.log(e)
    }

    return false
}

async function downLoadFile(event,url = ''){
    const res = await axios.get(url,{
        responseType:'arraybuffer'
    })

    return {
        buffer: res.data,
        status: res.status,
        type: res.headers['content-type']
    }
}

async function uploadFile(url,buffer){
    const form = new FormData()
    form.append('avatar_file',Buffer.from(buffer),{
        filename:'avatar.png',
        contentType: 'image/png'
    })

    const res = await axios.post(url,form,{
        headers:{
            ...form.getHeaders()
        }
    })

    return {
        ...res.data,
        reqStatus: res.status
    }
}

function getSettings(...params){
    if(!localSettings) return null

    return params.reduce((res,current) => {
        if(typeof(current) === 'string' && current in localSettings){
            return {
                ...res,
                [current]: localSettings[current]
            }
        }
        
        return { ...res }
    },{})
}

module.exports = { openFileDialog,downLoadFile,setCacheFile,uploadFile,readCacheFile,getSettings,setLocalSettings }