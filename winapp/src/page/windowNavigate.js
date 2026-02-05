import Stack from '@mui/material/Stack';
import Box from '@mui/material/Box';
import CloseSharpIcon from '@mui/icons-material/CloseSharp';
import RemoveSharpIcon from '@mui/icons-material/RemoveSharp';
import StorageSharpIcon from '@mui/icons-material/StorageSharp';
import AccountCircleSharpIcon from '@mui/icons-material/AccountCircleSharp';
import SettingsSharpIcon from '@mui/icons-material/SettingsSharp';
import FileUploadSharpIcon from '@mui/icons-material/FileUploadSharp';
import ArrowDropUpSharpIcon from '@mui/icons-material/ArrowDropUpSharp';
import FullscreenSharpIcon from '@mui/icons-material/FullscreenSharp';
import FullscreenExitSharpIcon from '@mui/icons-material/FullscreenExitSharp';
import PolylineSharpIcon from '@mui/icons-material/PolylineSharp';
import './home.css';
import './navigate.css';
import { useState } from 'react';
import Tabs from '@mui/material/Tabs';
import Tab from '@mui/material/Tab';

export function WindowNavigate({flag,setFlag,sendSizeChange}){
    const fullScreen = () => {
        setFlag(false)
        window.windowAPI.setMsg('fullscreen')
        window.windowAPI.getWindowHeight(sendSizeChange)
    }
    const normalScreen = () => {
        setFlag(true)
        window.windowAPI.setMsg('normalsize')
        window.windowAPI.getWindowHeight(sendSizeChange)
    }

    return(
        <Stack direction='row' spacing={1.5} className='main' sx={{position:'absolute',right:0,paddingRight:2}}>
            <RemoveSharpIcon className='item_hover' onClick={() => window.windowAPI.setMsg('mininum')}/>
            {(flag) ? <FullscreenSharpIcon className='item_hover' onClick={fullScreen}/>
                          : <FullscreenExitSharpIcon className='item_hover' onClick={normalScreen}/>}
            <CloseSharpIcon className='item_hover' onClick={() => window.windowAPI.setMsg('close')}/>
        </Stack>
    )
}

export function MenuNavigate({setIndex}){
    // 用于tab切页
    const [index,setPageIndex] = useState(0)

    return(
        <Tabs value={index} onChange={(event,newIndex) => setPageIndex(newIndex)} 
            className='sub' textColor='white' indicatorColor='white'>
            <Tab icon={<StorageSharpIcon />} label='资源' iconPosition='start' onClick={() => setIndex('browser')} sx={{color:'white'}}/>
            <Tab icon={<FileUploadSharpIcon />} label='上传' iconPosition='start' onClick={() => setIndex('upload')} sx={{color:'white'}}/>
            <Tab icon={<AccountCircleSharpIcon />} label='个人' iconPosition='start' onClick={() => setIndex('account')} sx={{color:'white'}}/>
            <Tab icon={<SettingsSharpIcon />} label='设置' iconPosition='start' onClick={() => setIndex('setting')} sx={{color:'white'}}/>
        </Tabs>
    )
}

export function NavigateLogo(){
    return(
        <Stack direction='row' spacing={1} >
            <PolylineSharpIcon sx={{transform:'scale(1.3)',filter:'invert(100%)'}}/>
            <h2 style={{fontWeight:'bold',letterSpacing:'6px',color:'white',fontSize:'18px'}} className='main'>Cloud</h2>
        </Stack>
    )
}

