import Stack from '@mui/material/Stack';
import CloseSharpIcon from '@mui/icons-material/CloseSharp';
import RemoveSharpIcon from '@mui/icons-material/RemoveSharp';
import StorageSharpIcon from '@mui/icons-material/StorageSharp';
import SearchSharpIcon from '@mui/icons-material/SearchSharp';
import AccountCircleSharpIcon from '@mui/icons-material/AccountCircleSharp';
import SettingsSharpIcon from '@mui/icons-material/SettingsSharp';
import FileUploadSharpIcon from '@mui/icons-material/FileUploadSharp';
import SettingsInputAntennaSharpIcon from '@mui/icons-material/SettingsInputAntennaSharp';
import ChevronRightSharpIcon from '@mui/icons-material/ChevronRightSharp';
// import '../function/window.js'
import './home.css';
import { useState } from 'react';

export function WindowNavigate(){
    // const sendCloseInfo = () => { ipcRenderer.send('window-api','window-close') }
    // const sendMininumInfo = () => { ipcRenderer.send('window-api','window-mininum') }

    return(
        <Stack direction='row' spacing={1.5} sx={{top:'50%',transform:'translate(0,50%)',cursor:'pointer'}}>
            <RemoveSharpIcon className='item_hover' onClick={() => window.windowAPI.setMsg('mininum')}/>
            <CloseSharpIcon className='item_hover' onClick={() => window.windowAPI.setMsg('close')}/>
        </Stack>
    )
}

export function MenuNavigate(){
    const [navi_pos,setPos] = useState(164) // default 160, +100 for each

    return(
        <>
            <Stack direction='row' spacing={5} sx={{textAlign:'center'}} className='menu'>
                <Stack direction='row' spacing={0.8} sx={{top:'50%',transform:'translate(0,50%)'}} onClick={() => setPos(164)}>
                    <StorageSharpIcon />
                    <p>资源</p>
                </Stack>
                {/* <Stack direction='row' spacing={0.8} sx={{top:'50%',transform:'translate(0,50%)'}} onClick={() => setPos(268)}>
                    <SearchSharpIcon />
                    <p>搜索</p>
                </Stack> */}
                <Stack direction='row' spacing={0.8} sx={{top:'50%',transform:'translate(0,50%)'}} onClick={() => setPos(270)}>
                    <FileUploadSharpIcon />
                    <p>上传</p>
                </Stack>
                <Stack direction='row' spacing={0.8} sx={{top:'50%',transform:'translate(0,50%)'}} onClick={() => setPos(370)}>
                    <AccountCircleSharpIcon />
                    <p>账号</p>
                </Stack>
                <Stack direction='row' spacing={0.8} sx={{top:'50%',transform:'translate(0,50%)'}} onClick={() => setPos(472)}>
                    <SettingsSharpIcon />
                    <p>设置</p>
                </Stack>
            </Stack>
            <ChevronRightSharpIcon sx={{position:'fixed',top:12,left:navi_pos,filter:'invert(100%)',transition:'all 0.3s ease-in-out'}}/>
        </>
    )
}

export function NavigateLogo(){
    return(
        <Stack direction='row' spacing={2} sx={{top:'50%',transform:'translate(0,50%)'}}>
            <SettingsInputAntennaSharpIcon sx={{transform:'scale(1.3)',filter:'invert(100%)'}}/>
            <h2 style={{fontWeight:'bold',letterSpacing:'6px',color:'white',fontSize:'18px'}}>Cloud</h2>
        </Stack>
    )
}

