import '../home.css';
import Box from '@mui/material/Box';
import BottomNavigation from '@mui/material/BottomNavigation';
import BottomNavigationAction from '@mui/material/BottomNavigationAction';
import PersonSharpIcon from '@mui/icons-material/PersonSharp';
import LogoutSharpIcon from '@mui/icons-material/LogoutSharp';
import BorderColorSharpIcon from '@mui/icons-material/BorderColorSharp';
import { LogoutPage } from './backDropPage';
import { useState } from 'react';

export function UserPanel(){
    const [value, setValue] = useState(0);
    const [openDialog,setOpen] = useState(false)
    const handleClose = () => setOpen(false)

    return(
        <>
            <Box sx={{width:'100%',margin:'0 0',padding:'0 0'}}>
                <BottomNavigation
                    showLabels
                    value={value}
                    onChange={(event, newValue) => {
                    setValue(newValue);
                    }}
                    sx={{borderBottom:'1px solid rgba(211,213,215,0.5)'}}
                >
                    <BottomNavigationAction label='个人资料' icon={<PersonSharpIcon color='#EBF2FA'/>}/>
                    <BottomNavigationAction label='修改账户' icon={<BorderColorSharpIcon color='#EBF2FA'/>} />
                    <BottomNavigationAction label='登出' icon={<LogoutSharpIcon color='#EBF2FA'/>} onClick={() => setOpen(true)}/>
                </BottomNavigation>
            </Box>
            <Box>
                <LogoutPage flag={openDialog} handleClose={handleClose}/>
            </Box>
        </>
    )
}