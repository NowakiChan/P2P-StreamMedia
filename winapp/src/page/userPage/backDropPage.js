import Backdrop from "@mui/material/Backdrop";
import Dialog from "@mui/material/Dialog";
import DialogTitle from "@mui/material/DialogTitle";
import Stack from "@mui/material/Stack";
import Button from "@mui/material/Button";
import Tab from "@mui/material/Tab";
import Tabs from "@mui/material/Tabs";
import CloseSharpIcon from '@mui/icons-material/CloseSharp';
import PasswordSharpIcon from '@mui/icons-material/PasswordSharp';
import Box from "@mui/material/Box";
import TextField from "@mui/material/TextField";
import Snackbar from "@mui/material/Snackbar";
import Alert from "@mui/material/Alert";
import FormGroup from "@mui/material/FormGroup";
import FormControlLabel from "@mui/material/FormControlLabel";
import Checkbox from "@mui/material/Checkbox";
import Tooltip from "@mui/material/Tooltip";
import IconButton from "@mui/material/IconButton";
import { useContext, useEffect, useState } from "react";
import { LoginStatusContext, WindowSizeContext, IDContext } from "../home";
import { LoginHintPage } from "./userPage";
import './user.css'
import Card from "@mui/material/Card";
import CardHeader from "@mui/material/CardHeader";
import LogoutSharpIcon from '@mui/icons-material/LogoutSharp';
import CardContent from "@mui/material/CardContent";
import Typography from "@mui/material/Typography";
import Avatar from "@mui/material/Avatar";
import { accountLogin, getSHA256,verifyPwdFormat } from "../function";


export function LogoutPage({flag,handleClose}){
    // const refresh = useContext(RefreshContext)
    const [windowSizeFlag,windowSize] = useContext(WindowSizeContext)
    const [loginFlag,setFlag] = useContext(LoginStatusContext)
    const [uid,setUID] = useContext(IDContext)
    const logOutHandler = () => {
        setFlag(false)
        setUID(0)
        window.fileAPI.saveSetting({
            token: null,
            tokenUID: null
        })
        handleClose()
    }

    // 用于切换到登录页面
    const [nextPage,openNextPage] = useState(false)
    // 用于重置界面
    useEffect(() => {
        if(!flag){
            setTimeout(() => {
                openNextPage(false)
                setData({
                    identity:'',
                    pwd:'',
                    keepAlive:true
                })
            },500)

        }
    },[flag])
    // 用于提示信息
    const [hint,setHint] = useState({
        type:'info',
        text:'',
        openNew:false
    })
    const setNewHint = (newType = 'info',newText = '') => {
        setHint({
            type: newType,
            text: newText,
            openNew: !hint.openNew
        })
    }
    // 用于记录输入信息
    const handleCheck = () => { setData({ ...inputData,keepAlive: !inputData.keepAlive }) }
    const inputChange = (key) => (e) => { setData({ ...inputData, [key]: e.target.value }) }
    const [inputData,setData] = useState({
        identity:'',
        pwd:'',
        keepAlive: true
    })
    const trySwitchAccount = async () => {
        if(inputData.identity.length === 0 || inputData.pwd.length === 0){
            setNewHint('warning','请完整输入所有信息')
            return
        }

        const res = await accountLogin(inputData)
        console.log("Switch Account -> ",res)
        if(res.reqStatus === 200){
            if(res.status === 100){
                if(res.userid === uid){
                    handleClose()
                    return // 如果登录的是相同账号，不做处理
                }

                setUID(res.userid)
        
                // 存储token
                let settings = {
                    token: null,
                    tokenUID: null
                }
                if(res.token){
                    settings = {
                        token: res.token,
                        tokenUID: res.userid
                    }
                }
                window.fileAPI.saveSetting(settings)

                setNewHint('success','切换账号成功')
                setTimeout(handleClose,1500)
            }
            else if(res.status === 104) setNewHint('error',`用户名或密码错误`)
            else setNewHint('error',`服务器处理错误(${res.status})`)
        }
        else setNewHint('error',`服务器请求出错(${res.reqStatus})`)
    }
    const pageSelector = () => {
        if(nextPage){
            return(
                <>
                    <Stack direction={'column'} alignItems={'center'} spacing={1} justifyContent={'center'}>
                        <Typography variant='body1' align='center' gutterBottom>
                            切换到新账号
                        </Typography>
                        <TextField required id='new_pwd' variant='standard' label='账户名/UID' helperText='UID为#号开头的6位数字'
                                   value={inputData.identity} onChange={inputChange('identity')}/>
                        <TextField required id='new_pwd' variant='standard' label='密码' type='password' 
                                   value={inputData.pwd} onChange={inputChange('pwd')}/>
                        <FormControlLabel control={<Checkbox size='small' checked={inputData.keepAlive} onClick={handleCheck}/>} 
                                        label='保持登录'/>
                        <Button variant='text' color='primary' sx={{width:100}} onClick={trySwitchAccount}>切换账号</Button>
                    </Stack>
                </>
            )
        }
        else{
            return(
                <>
                    <Typography variant='body1' align='center' gutterBottom>
                        退出账号后, 您仍可以浏览资源, 但无法进行互动或上传新内容
                    </Typography>
                    <Typography variant='body1' align='center' gutterBottom>
                        您也可以选择切换账号来快速登录另一个账号
                    </Typography>        
                    <Stack direction={'column'} spacing={1.5} justifyContent={'center'} alignItems={'center'}>
                        <Button variant='text' color='success' sx={{width:100}} onClick={() => openNextPage(true)}>切换账号</Button>
                        <Button variant='text' color='primary' sx={{width:100}} onClick={logOutHandler}>退出登录</Button>
                    </Stack>
                </>
            )
        }
    }

    
    return (loginFlag) ?
        <Backdrop
        sx={(theme) => ({ color: '#fff', zIndex: theme.zIndex.drawer + 1 })}
        open={flag}
        >
            <Card sx={{width:(windowSizeFlag) ? '30%' : '15%'}}>
                <CardHeader avatar={<LogoutSharpIcon />} title={(nextPage) ? '切换账号' : '退出账号'} 
                            action={<CloseIcon handleClose={handleClose} Icon={<CloseSharpIcon />}/>}/>
                <CardContent>
                    {pageSelector()}
                </CardContent>
            </Card>
            <PageAlert type={hint.type} info={hint.text} open={hint.openNew} clearItem={flag}/>
        </Backdrop>
        : <LoginHintPage />
    
    
}

export function AccountOperatePage({flag,handleClose}){
    // 用于标签页切换
    const [tabValue, setTabValue] = useState(0)
    const handleChange = (event, newValue) => {
        setTabValue(newValue);
    };
    const [alertSettings,setAlert] = useState({
        type: 'info',
        text: ''
    })

    // 用于设置提示信息
    const [newAlert,setNewAlert] = useState(false)
    const handleSnackbarOpen = (new_type,new_text) => {
        setAlert({
            type: new_type,
            text: new_text
        })
        setNewAlert(!newAlert)
    }

    const handleMainPageClose = () => {
        // setSnackbar(false)
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
                    <LoginAndRegisterPage select={tabValue} alertHandle={handleSnackbarOpen} open={flag}/>
                </Stack>
            </Box>
            <PageAlert clearItem={!flag} type={alertSettings.type} info={alertSettings.text} open={newAlert} />
        </Backdrop>
    )
}

export function LoginAndRegisterPage({select,alertHandle,open}){
    // 用于设置保持登录状态
    // const [keepLoginFlag,setKeepLoginFlag] = useState(true)
    const [isLogin,setLoginStatus] = useContext(LoginStatusContext)
    const [userID,setID] = useContext(IDContext)
    
    // 用于记录各项输入值
    const [loginInput,setLoginInput] = useState({
        identity: '',
        pwd: '',
        keepAlive: true
    })
    const handleChange = () => { setLoginInput({...loginInput,keepAlive: !loginInput.keepAlive}) }
    const getLoginInput = (key) => (e) => {
        setLoginInput({
            ...loginInput,
            [key] : e.target.value
        })
    }

    // 用于记录注册的值
    const [regInput,setRegInput] = useState({
        username: '',
        pwd: '',
        confirm: ''
    })
    const getRegInput = (key) => (e) => {
        setRegInput({
            ...regInput,
            [key] : e.target.value
        })
    }
    // 在退出时，清空所有的输入内容
    useEffect(() => {
        if(!open){
            setTimeout(() => {
                setLoginInput({
                    identity: '',
                    pwd: '',
                    keepAlive: true
                })
                setRegInput({
                    username: '',
                    pwd: '',
                    confirm: ''
                })
            },500)
            // clearInput()
            // clearTimeout(clearInput)
        }

    },[open])

    // 登陆请求处理函数
    const tryLogin = async () => {
        if(loginInput.identity.length === 0 || loginInput.pwd.length === 0){
            alertHandle('warning','请完整输入所有信息')
        }
        else{
            const res = await accountLogin({
                identity: loginInput.identity.trim(),
                pwd: loginInput.pwd.trim(),
                keepAlive: loginInput.keepAlive
            })
            console.log("res ->",res)
            if(res.reqStatus === 200){
                loginResHandle(res)
            }
            else alertHandle('error',`服务器请求错误(${res.reqStatus})`)
              
        }
    }
    const loginResHandle = (res) => {
        if(res.status === 100){
            alertHandle('success','登陆成功')
            // 更改登陆状态并存储用户id
            setLoginStatus(true)
            setID(res.userid)
            if(res.token){
                // 存储token
                const settings = {
                    token: res.token,
                    tokenUID: res.userid
                }
                window.fileAPI.saveSetting(settings)
            }
        }
        else if(res.status === 104){
            alertHandle('error','用户名或密码错误')
        }
        else{
            alertHandle('error',`服务器处理错误(${res.status})`)
        }
    }

    // 用户名查重函数
    const checkDuplicate = async () => {
        if(regInput.username.length === 0) return false

        const res = await window.svrAPI.request({
            url:window.svrAPI.url.concat('/user/infocheck'),
            method:'POST',
            headers:{
                    'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                name: regInput.username.trim()
            })
        })
        if(res.reqStatus === 200){
            return res.duplicate;
        }
        else alertHandle('error','服务器请求出错, 请稍后重试')

        return false;
    }
    // 注册处理函数
    const tryReg = async () => { 

        if(regInput.confirm.length === 0 || regInput.pwd.length === 0 || regInput.username.length === 0){
            alertHandle('warning','请完整填写所有信息')
        }
        else if(regInput.pwd !== regInput.confirm){
            alertHandle('warning','两次密码输入不一致')
        }
        else{
            const flag = await checkDuplicate()
            if(flag){
                alertHandle('warning','该用户名已存在')
                return
            }

            const pwd_sha256 = await getSHA256(regInput.pwd)
            const res = await window.svrAPI.request({
                url:window.svrAPI.url.concat('/user/register'),
                method:'POST',
                headers:{
                    'Content-Type': 'application/json'
                },
                body : JSON.stringify({
                    username: regInput.username.trim(),
                    pwd: pwd_sha256
                })
            })
            console.log(res)
            if(res.reqStatus === 200 && res.status === 100){
                alertHandle('success','注册成功')
            }
            else alertHandle('error','服务器发生错误, 请稍后重试')
        }
        
    }

    const pageGenerator = () => {
        if(select === 0){
            return(
                <Stack direction='column' spacing={2} sx={{justifyContent:'center',alignItems:'center'}}>
                    <TextField id='login_username' label='用户名或UID' variant='outlined' helperText='UID为#号开头的6位数字'
                               value={loginInput.identity} size='small' onChange={getLoginInput('identity')}/>
                    <TextField id='login_pwd' label='密码' variant='outlined' size='small' type='password'
                               value={loginInput.pwd} onChange={getLoginInput('pwd')}/>
                    <FormGroup>
                        <FormControlLabel control={<Checkbox checked={loginInput.keepAlive} onChange={handleChange}/>} label="自动登录"/>
                    </FormGroup>
                    <Button variant='text' size='medium' sx={{maxWidth:'50%'}} onClick={tryLogin}>登录</Button>
                </Stack>
            )
        }
        else{
            return(
                <Stack direction='column' spacing={2} sx={{justifyContent:'center',alignItems:'center'}}>
                    <TextField id='reg_username' label='用户名' variant='outlined' size='small' 
                               value={regInput.username} onChange={getRegInput('username')}/>
                    <TextField id='reg_pwd' label='密码' variant='outlined' size='small' type='password'
                               value={regInput.pwd} onChange={getRegInput('pwd')}/>
                    <TextField id='pwd_confirm' label='确认密码' variant='outlined' size='small' type='password'
                               value={regInput.confirm} onChange={getRegInput('confirm')}/>
                    <Button variant='text' size='medium' sx={{maxWidth:'50%'}} onClick={tryReg}>注册</Button>
                </Stack>
            )
        }
    }

    return pageGenerator() 
}

export function CloseIcon({handleClose,Icon}){
    return(
        <Tooltip title='关闭'>
            <IconButton aria-label='close' onClick={handleClose} size='small' sx={{transform:'scale(0.9)'}}>
                {Icon}
            </IconButton>
        </Tooltip>
    )
}

export function PageAlert({type,info,open,location = {v:'top',h:'center'},clearItem = false}){
    // 用于记录每一个信息
    const [msg,setMsg] = useState([])
    

    // 用于退出
    const msgClose = (mid) => (event,reason) => {
        if(reason === 'clickaway') return

        setMsg((oldValue) => oldValue.map((value) => value.mid === mid ? {...value,status : false} : value))
    }
    // 用于完全退出的组件
    const fullExit = (mid) => {
        setMsg((oldValue) => oldValue.filter((value) => value.mid !== mid))
    }
    // 用于检测info和type的变化，若有新的内容则增添一条信息
    useEffect(() => {
        if(!info || !type) return
        
        // 用于增加一条信息
        const id = crypto.randomUUID()
        setMsg((oldValue) => [
            ...oldValue,
            {
                mtype : type,
                minfo : info,
                mid : id,
                status : true
            }
        ])
    },[open])

    useEffect(() => {
        if(clearItem){
            // 在主组件退出时，清空消息列表，防止下一次打开消息仍然滞留
            // console.log('clear list')
            setMsg([])
        }
    },[clearItem])

    return(
        <>
            {msg.map((value,index) => (
                <Snackbar key={value.mid} open={value.status}
                          autoHideDuration={5000} onClose={msgClose(value.mid)}
                          anchorOrigin={{vertical:location.v,horizontal:location.h}}
                          slotProps={{
                            transition: {
                                onExited: () => fullExit(value.mid)
                            }
                          }}
                >
                    <Alert severity={value.mtype} onClose={msgClose(value.mid)}>{value.minfo}</Alert>
                </Snackbar>
            ))}
        </>
    )
}

export function PwdModifyPage({info,flag,handleClose}){
    
    // 用于在flag变化后处理，重设所有输入值
    useEffect(() => {
        if(!flag){
            setTimeout(() => {
                setInputData({
                oldPwd:'',
                newPwd:'',
                confirmPwd:''
                })
            },500)
        }

    },[flag])

    // 用于处理数据
    const handleInputDataChange = (key) => (e) => {
        setInputData({ ...inputData, [key] : e.target.value})
    }
    const [inputData,setInputData] = useState({
        oldPwd : '',
        newPwd : '',
        confirmPwd : ''
    }) // 用于记录输出值
    // 用于打开提示
    const [openHint,setHint] = useState({
        type : 'info',
        info : '',
        display : false
    })
    const setNewHint = (newType = 'info',newInfo = '') => {
        setHint({
            type : newType,
            info : newInfo,
            display : !openHint.display
        })
    }
    // 用于尝试重设密码
    const tryModifyPwd = async () => {
        if(inputData.oldPwd.length === 0 || inputData.newPwd.length === 0 ||
           inputData.confirmPwd.length === 0){
            setNewHint('warning','请完整输入所有信息')
        }
        else if(!verifyPwdFormat(inputData.newPwd)){
            setNewHint('warning','密码必须包括至少一个字母和数字, 长度大于8位')
        }
        else if(inputData.confirmPwd !== inputData.newPwd){
            setNewHint('warning','两次密码输入不一致')
        }
        else{
            const pwd_sha256 = await getSHA256(inputData.oldPwd)
            const new_sha256 = await getSHA256(inputData.newPwd)
            const res = await window.svrAPI.request({
                url: `${window.svrAPI.url}/user/pwdmodify`,
                method:'POST',
                headers:{
                    'Content-Type': 'application/json'
                },
                body : JSON.stringify({
                    oldPwd: pwd_sha256,
                    newPwd: new_sha256,
                    userid: uid
                })
            })

            if(res.reqStatus === 200){
                if(res.status === 100){
                    setNewHint('success','修改密码成功')
                    setTimeout(handleClose,1000)
                }
                else if(res.status === 104) setNewHint('error','原密码输入错误')
                else setNewHint('error',`服务器处理错误(${res.status})`)
            }
            else setNewHint('error',`服务器请求失败(${res.reqStatus})`)
        }
    }
    
   

    // 用于读取头像
    const [avatarSrc,setSrc] = useState(null)
    const [uid,setUID] = useContext(IDContext)

    return(
        <Backdrop open={flag} sx={(theme) => ({zIndex: theme.zIndex.drawer + 1 })}>
            <Card>
                <CardHeader avatar={<PasswordSharpIcon />} title='修改密码' 
                            action={<CloseIcon handleClose={handleClose} Icon={<CloseSharpIcon />}/>}/>
                <CardContent>
                    <Stack direction={'column'} spacing={1.5} alignItems={'center'}>
                        <Stack direction={'column'} spacing={0.5} alignItems={'center'}>
                            {(info.avatar_file_link) ? <Avatar alt="avator" src={info.avatar_file_link} sx={{height:70,width:70}}/> : 
                                        <Avatar sx={{bgcolor:'#427aa1',color:'white',height:70,width:70}}>{info.username.substring(0,2)}</Avatar>}
                            <Typography variant='caption'>{info.username}</Typography>
                        </Stack>
                            <TextField required id='confirm_old_pwd' variant='standard' type='password' value={inputData.oldPwd}
                                    label='原密码' onChange={handleInputDataChange('oldPwd')}/>
                            <TextField required id='new_pwd' variant='standard' label='新密码' type='password' 
                                    value={inputData.newPwd} onChange={handleInputDataChange('newPwd')}/>
                            <TextField required id='confirm_new_pwd' variant='standard' label='确认新密码' type='password'
                                    value={inputData.confirmPwd} onChange={handleInputDataChange('confirmPwd')}/>
                            <Button variant='text' sx={{width:100}} color='primary' onClick={tryModifyPwd}>修改密码</Button>
                    </Stack>
                </CardContent>
            </Card>
            <PageAlert type={openHint.type} info={openHint.info} open={openHint.display} clearItem={!flag}/>
        </Backdrop>
    )
}