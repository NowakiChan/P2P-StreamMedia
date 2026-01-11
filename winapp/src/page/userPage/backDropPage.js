import Backdrop from "@mui/material/Backdrop";
import Dialog from "@mui/material/Dialog";
import DialogTitle from "@mui/material/DialogTitle";
import Stack from "@mui/material/Stack";
import Button from "@mui/material/Button";
import Tab from "@mui/material/Tab";
import Tabs from "@mui/material/Tabs";
import CloseSharpIcon from '@mui/icons-material/CloseSharp';
import Box from "@mui/material/Box";
import TextField from "@mui/material/TextField";
import Snackbar from "@mui/material/Snackbar";
import Alert from "@mui/material/Alert";
import { useContext, useEffect, useRef, useState } from "react";
import { PageIndexContext,LoginStatusContext } from "../home";
import { LoginFunc,RegisterFunc } from "./userInteract";
import './user.css'


export function LogoutPage({flag,handleClose}){
    const handler = useContext(PageIndexContext)
    const [loginFlag,setFlag] = useContext(LoginStatusContext)
    const logOutHandler = () => {
        //... handler
        // handler('browser')
        setFlag(false)
        handleClose()
    }

    
    return(
        <Dialog open={flag} onClick={handleClose}>
            <Stack direction='row' spacing={0.5}>
                <DialogTitle sx={{marginRight:0,paddingRight:0}}>确定退出账号?</DialogTitle>
                <Button onClick={logOutHandler}>是</Button>
                <Button onClick={handleClose}>否</Button>
            </Stack>
        </Dialog>
    )
}

export function AccountOperatePage({flag,handleClose}){
    const [tabValue, setTabValue] = useState(0)
    const [snackbarOpen,setSnackbar] = useState(false)
    const handleChange = (event, newValue) => {
        setTabValue(newValue);
    };
    const [alertSettings,setAlert] = useState({
        type: 'info',
        text: ''
    })

    const handleSnackbarOpen = (new_type,new_text) => {
        setAlert({
            type: new_type,
            text: new_text
        })
        setSnackbar(true)
    }
    const handleSnackbarClose = (event,reason) => {
        if(reason === 'clickaway') return

        setSnackbar(false)
    }
    const handleMainPageClose = () => {
        setSnackbar(false)
        handleClose()
    }
    return(
        <Backdrop open={flag} sx={(theme) => ({zIndex: theme.zIndex.drawer + 1 })}>
            <Box sx={{bgcolor:'white',width:250,height:350,overflow:'hidden',
                      transition:'height 0.4s ease-in-out',borderRadius:'5px'}}>
                <Box sx={{width:'100%',height:20,justifyContent:'flex-end',flexDirection:'row',
                          bgcolor:'#1976d2',display:'flex',alignItems:'center',
                          borderTopLeftRadius:'5px',borderTopRightRadius:'5px'}}>
                    <CloseSharpIcon onClick={handleMainPageClose} className='dialog_item'/>
                </Box>
                <Stack direction='column' spacing={4.5} sx={{justifyContent:'center',alignItems:'center'}}>
                    <Tabs value={tabValue} onChange={handleChange} 
                          aria-label='basic tabs example'>
                        <Tab label='登录' />
                        <Tab label='注册' />
                    </Tabs>
                    <LoginAndRegisterPage select={tabValue} alertHandle={handleSnackbarOpen}/>
                </Stack>
            </Box>
            <Snackbar
                anchorOrigin={{ horizontal:'center',vertical:'top'  }}
                open={snackbarOpen}
                onClose={handleSnackbarClose}
                autoHideDuration={5000} 
            >
                <Alert severity={alertSettings.type}>{alertSettings.text}</Alert>
            </Snackbar>
        </Backdrop>
    )
}

export function LoginAndRegisterPage({select,alertHandle}){

    const tryLogin = () => {
        let usrname = document.getElementById('login_username').value
        let pwd = document.getElementById('login_pwd').value
        console.log(usrname,pwd)
        if(usrname === ''){
            return
        }
        else {}

        if(pwd === ''){
            return
        }
        else {}
        let result = LoginFunc(usrname,pwd)
        alertHandle('success','this is a login test')
    }

    const tryRegister = () => {
        let usrname = document.getElementById('reg_username').value
        let pwd = document.getElementById('reg_pwd').value
        let confirm_pwd = document.getElementById('pwd_confirm').value
        let result = RegisterFunc(usrname,pwd)
        alertHandle('error','this is a register test')
    }

    const pageGenerator = () => {
        if(select == 0){
            return(
                <Stack direction='column' spacing={2} sx={{justifyContent:'center',alignItems:'center'}}>
                    <TextField id='login_username' label='用户名或UID' variant='outlined' size='small'/>
                    <TextField id='login_pwd' label='密码' variant='outlined' size='small' />
                    <Button variant='text' size='medium' sx={{maxWidth:'50%'}} onClick={tryLogin}>登录</Button>
                </Stack>
            )
        }
        else{
            return(
                <Stack direction='column' spacing={2} sx={{justifyContent:'center',alignItems:'center'}}>
                    <TextField id='reg_username' label='用户名' variant='outlined' size='small' />
                    <TextField id='reg_pwd' label='密码' variant='outlined' size='small' />
                    <TextField id='pwd_confirm' label='确认密码' variant='outlined' size='small' />
                    <Button variant='text' size='medium' sx={{maxWidth:'50%'}} onClick={tryRegister}>注册</Button>
                </Stack>
            )
        }
    }

    return pageGenerator() 
}