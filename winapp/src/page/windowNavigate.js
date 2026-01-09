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
        <Stack direction='row' spacing={1.5} sx={{position:'relative',top:12,left:(flag) ? 5 : 130}}className='main'>
            <RemoveSharpIcon className='item_hover' onClick={() => window.windowAPI.setMsg('mininum')}/>
            {(flag) ? <FullscreenSharpIcon className='item_hover' onClick={fullScreen}/>
                          : <FullscreenExitSharpIcon className='item_hover' onClick={normalScreen}/>}
            <CloseSharpIcon className='item_hover' onClick={() => window.windowAPI.setMsg('close')}/>
        </Stack>
    )
}

export function MenuNavigate({setIndex,children}){
    const [navi_pos,setPos] = useState(183) // default 160, +100 for each

    return(
        <>
            <Stack direction='row' spacing={5} sx={{textAlign:'center'}} className='main'>
                <Stack direction='row' spacing={0.8} className='menu' sx={{top:'50%',transform:'translate(0,50%)'}} 
                       onClick={() => {
                            setPos(183)
                            setIndex('browser')
                        }}>
                    <StorageSharpIcon />
                    <p>资源</p>
                </Stack>
                <Stack direction='row' spacing={0.8} className='menu' sx={{top:'50%',transform:'translate(0,50%)'}}
                       onClick={() => {
                            setPos(287)
                            setIndex('upload')
                        }}>
                    <FileUploadSharpIcon />
                    <p>上传</p>
                </Stack>
                <Stack direction='row' spacing={0.8} className='menu' sx={{top:'50%',transform:'translate(0,50%)'}} 
                       onClick={() => {
                            setPos(388)
                            setIndex('account')
                        }}>
                    <AccountCircleSharpIcon />
                    <p>账号</p>
                </Stack>
                <Stack direction='row' spacing={0.8} className='menu' sx={{top:'50%',transform:'translate(0,50%)'}} 
                       onClick={() => {
                            setPos(490)
                            setIndex('setting')
                        }}>
                    <SettingsSharpIcon />
                    <p>设置</p>
                </Stack>
            </Stack>
            <Box sx={{width:65,height:48,position:'fixed',
                      top:0,left:navi_pos,
                      borderBottom:'4px solid white',
                      transition:'all 0.3s ease-in-out'}}>
                <ArrowDropUpSharpIcon sx={{filter:'invert(100%)',
                                           position:'absolute',bottom:-10,
                                           left:'50%',transform:'translateX(-50%)'}}/>
            </Box>
        </>
    )
}

export function NavigateLogo(){
    return(
        <Stack direction='row' spacing={2} sx={{top:'50%',transform:'translate(0,50%)'}}>
            <PolylineSharpIcon sx={{transform:'scale(1.3)',filter:'invert(100%)'}}/>
            <h2 style={{fontWeight:'bold',letterSpacing:'6px',color:'white',fontSize:'18px'}} className='main'>Cloud</h2>
        </Stack>
    )
}

