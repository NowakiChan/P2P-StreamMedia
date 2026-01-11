import '../home.css';
import Box from '@mui/material/Box';
import BottomNavigation from '@mui/material/BottomNavigation';
import BottomNavigationAction from '@mui/material/BottomNavigationAction';
import PersonSharpIcon from '@mui/icons-material/PersonSharp';
import LogoutSharpIcon from '@mui/icons-material/LogoutSharp';
import BorderColorSharpIcon from '@mui/icons-material/BorderColorSharp';
import NoAccountsSharpIcon from '@mui/icons-material/NoAccountsSharp';
import Stack from '@mui/material/Stack';
import Button from '@mui/material/Button';
import { LogoutPage,AccountOperatePage } from './backDropPage';
import { useContext, useState } from 'react';
import { LoginStatusContext } from '../home';
import '../errorPage.css';

export function UserPanel(){
    const [value, setValue] = useState(0);
    const [openDialog,setOpen] = useState(false)
    const [openLoginDialog,setLoginDialog] = useState(false)
    const [flag,setFlag] = useContext(LoginStatusContext)
    const handleClose = () => setOpen(false)
    const handleLoginClose = () => setLoginDialog(false)
    const handleLoginOpen = () => setLoginDialog(true)
    const handleOpen = () => {
        if(flag) setOpen(true)
    }
    const pageSelector = (title) => {
        if(!flag) return (<LoginHintPage setLoginPageOpen={handleLoginOpen}/>)

        if(title === 'account_main_page'){
            return (<></>)
        }
        else if(title === 'account_settings'){
            return (<></>)
        }

        return <></>
    }

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
                    <BottomNavigationAction label='个人资料' icon={<PersonSharpIcon color='#EBF2FA'/>} 
                                            onClick={() => pageSelector('account_main_page')}/>
                    <BottomNavigationAction label='修改账户' icon={<BorderColorSharpIcon color='#EBF2FA'/>}
                                            onClick={() => pageSelector('account_settings')}/>
                    <BottomNavigationAction label='登出' icon={<LogoutSharpIcon color='#EBF2FA'/>} onClick={handleOpen}/>
                </BottomNavigation>
            </Box>
            <Box>
                {pageSelector()}
                {(flag) ? <></> : <LogoutPage flag={openDialog} handleClose={handleClose}/>}
                {(flag) ? <></> : <AccountOperatePage flag={openLoginDialog} handleClose={handleLoginClose}/>}
            </Box>
        </>
    )
}

export function LoginHintPage({setLoginPageOpen}){
    return(
        <Stack direction='column' spacing={2} className='main_hint_page'>
            <Stack direction='row' spacing={1.5} sx={{alignItems:'center',justifyContent:'center'}}>
                <NoAccountsSharpIcon sx={{transform:'scale(1.4)'}}/>
                <h3 className='main_hint_title'>您还没有登陆</h3>
            </Stack>
            <p className='main_hint_text'>如没有账号, 您可以先创建账号, 登录后您可以使用评论, 上传等功能</p>
            <Button variant='text' size='medium' sx={{maxWidth:'50%'}} onClick={setLoginPageOpen}>登录账号</Button>
        </Stack>
    )
}