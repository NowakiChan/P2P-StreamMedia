import '../home.css';
import '../errorPage.css';
import './user.css';
import LogoutSharpIcon from '@mui/icons-material/LogoutSharp';
import NoAccountsSharpIcon from '@mui/icons-material/NoAccountsSharp';
import EqualizerSharpIcon from '@mui/icons-material/EqualizerSharp';
import PersonOutlineSharpIcon from '@mui/icons-material/PersonOutlineSharp';
import MoreHorizSharpIcon from '@mui/icons-material/MoreHorizSharp';
import PasswordSharpIcon from '@mui/icons-material/PasswordSharp';
import EditSharpIcon from '@mui/icons-material/EditSharp';
import DoneSharpIcon from '@mui/icons-material/DoneSharp';
import CloseSharpIcon from '@mui/icons-material/CloseSharp';
import PortraitSharpIcon from '@mui/icons-material/PortraitSharp';
import FileUploadSharpIcon from '@mui/icons-material/FileUploadSharp';
import FolderSharpIcon from '@mui/icons-material/FolderSharp';
import ExpandMoreSharpIcon from '@mui/icons-material/ExpandMoreSharp';
import ManageAccountsSharpIcon from '@mui/icons-material/ManageAccountsSharp';
import Stack from '@mui/material/Stack';
import Button from '@mui/material/Button';
import Box from '@mui/material/Box';
import Divider from '@mui/material/Divider';
import List from '@mui/material/List';
import ListItemButton from '@mui/material/ListItemButton';
import ListItemText from '@mui/material/ListItemText';
import ListItemIcon from '@mui/material/ListItemIcon';
import Container from '@mui/material/Container';
import Avatar from '@mui/material/Avatar';
import Card from '@mui/material/Card';
import CardContent from '@mui/material/CardContent';
import { AccountOperatePage, LogoutPage, CloseIcon, PwdModifyPage, PageAlert } from './backDropPage';
import { createContext, useContext, useEffect, useRef, useState } from 'react';
import { LoginStatusContext,WindowSizeContext,PageHintContext, UserInfoContext } from '../home';
import { getUID } from '../function';
import CardHeader from '@mui/material/CardHeader';
import Typography from '@mui/material/Typography';
import CardActions from '@mui/material/CardActions';
import IconButton from '@mui/material/IconButton';
import Tooltip from '@mui/material/Tooltip';
import TextField from '@mui/material/TextField';
import Backdrop from '@mui/material/Backdrop';
import ToggleButtonGroup from '@mui/material/ToggleButtonGroup';
import ToggleButton from '@mui/material/ToggleButton';
import AvatarEditor from 'react-avatar-editor'
import Slider from '@mui/material/Slider';
import Skeleton from '@mui/material/Skeleton';
import Collapse from '@mui/material/Collapse';

// export const RefreshContext = createContext(null)

export function UserPanel({show = true}){
    const [loginStatus,setLoginStatus] = useContext(LoginStatusContext)
    // const [windowSizeFlag,windowSize] = useContext(WindowSizeContext)
    // const [userId,setId] = useContext(IDContext)
    const [userInfo,setUserInfo] = useContext(UserInfoContext)
    const setHint = useContext(PageHintContext)
    const [infoReady,setReady] = useState(true) // 用于在信息加载完成后再显示组件，避免用户看到原始数据
    const refreshAvatar = async () => {
        const {buffer,status,contentType} = await window.svrAPI.download(`${window.svrAPI.url}/user/avator?userid=${userInfo.userid}`)
        // console.log('status=',status,' : type=',contentType)
        if(status === 200){
            const blob = new Blob([buffer],{type: contentType})
            let avatarUrl = URL.createObjectURL(blob)
            // console.log('url=',avatarUrl)
            return avatarUrl
        }
        setHint('error',`获取用户头像数据失败(${status})`)
        return null
    } // 用于刷新头像及头像获取

    useEffect(() =>{
        if(!loginStatus || !userInfo.userid || userInfo.userid === 0) return

        setReady(false)
        const tryGetInfo = async () => {
            const res = await window.svrAPI.request({
                url:`${window.svrAPI.url}/user/info`,
                method:'GET',
                headers:{},
                params:{ id: userInfo.userid}
            })
            const url = await refreshAvatar()
            if(res.reqStatus === 200 && res.status === 100){
                setUserInfo({
                    ...userInfo,
                    ...res.user_info,
                    avatar_file_link: url,
                    // 目前没有以下返回结果，故暂时填上
                    works : 0,
                    likes : 0,
                    comments : 0,

                })
                console.log(userInfo)
                // setHint('info','测试内容')
            }
            else setHint('error','获取用户信息失败')
            setReady(true)
        }

        tryGetInfo()
        

    },[loginStatus,userInfo.userid])
    // 用于呼出退出账号确认面板
    const [backDrop,setBackDrop] = useState(false)
    // 用于呼出密码修改面板
    const [pwdPage,setPwdPage] = useState(false)

    if(loginStatus){

        return(
            <Box sx={{width:'100%',height:'100%',display:(show && loginStatus) ? 'flex' : 'none',flexDirection:'row',
                margin:'0 0',overflowX:'hidden'}}>
                <Box sx={{height:'100%',minWidth:'100%',margin:'0 0'}}>
                    <UserInfoPage avatarUpdate={refreshAvatar} ready={infoReady} userInfo={userInfo}
                                  setUserInfo={setUserInfo} setPwdPage={setPwdPage} setLogoutPage={setBackDrop}/>
                    <LogoutPage flag={backDrop} handleClose={() => setBackDrop(false)} />
                    <PwdModifyPage info={userInfo} flag={pwdPage} handleClose={() => setPwdPage(false)}/>
                </Box>
            </Box>
        )
    }
    else {
        // return <></>
        return ((show) ? <LoginHintPage /> : <></>)
    }
   
}

export function LoginHintPage(){
    const [openDialog,setDialog] = useState(false)

    const handleClose = () => setDialog(false)
    const handleOpen = () => setDialog(true)
    return(
        <>
            <Stack direction='column' spacing={2} className='main_hint_page'>
                <Stack direction='row' spacing={1.5} sx={{alignItems:'center',justifyContent:'center'}}>
                    <NoAccountsSharpIcon sx={{transform:'scale(1.4)'}}/>
                    <h3 className='main_hint_title'>您还没有登陆</h3>
                </Stack>
                <p className='main_hint_text'>如没有账号, 您可以先创建账号, 登录后您可以使用评论, 上传等功能</p>
                <Button variant='text' size='medium' sx={{maxWidth:'50%'}} onClick={handleOpen}>登录账号</Button>
            </Stack>
            <AccountOperatePage flag={openDialog} handleClose={handleClose}/>                
        </>
    )
}

/*  info --- 用户信息变量 
    setInfo --- 设置用户信息变量
    avatarUpdate --- 用于更新头像(下载文件并生成链接)的函数
    ready --- 用于指示数据是否准备好，false时组件内容全部显示为Skeleton
    setPwdPage,setLogoutPage --- 用于打开账号操作界面
*/
export function UserInfoPage({avatarUpdate,ready = false,setPwdPage,setLogoutPage,userInfo,setUserInfo}){
    // 用于左侧视频列表数据绑定
    const [resInfo,setResInfo] = useState([])

    // 用于控制编辑面板的扩展
    const [expandFold,setExpandFold] = useState(false) // 用于展开更多账户操作选项
    const [openEditor,setEditorOpen] = useState(false)
    const setHint = useContext(PageHintContext)
    // 用于上传新个人信息
    const [inputData,setData] = useState({
        username: userInfo.username,
        autograph: userInfo.autograph
    })
    // 用于同步信息
    const [allowEdit,setEdit] = useState(true)
    // const refreshInfo = useContext(RefreshContext)
    // const [avatarSrc,setSrc] = useState(null)
    useEffect(() => {
        setData({
            username: userInfo.username,
            autograph: userInfo.autograph
        })
    },[userInfo])

    const setInputData = (key) => (e) => {
        setData({
            ...inputData,
            [key]: e.target.value
        })
    }
    // const [uid,setUID] = useContext(IDContext)
    // const [userInfo,setUserInfo] = useContext(UserInfoContext)
    const tryUpdateInfo = async () => {
        console.log(inputData)
        if(!openEditor){
            setEditorOpen(true)
            return
        }
        else if(inputData.username.length === 0 || inputData.autograph.length === 0){
            setHint('warning','请完整填写所有信息')
            return
        }
        else if(inputData.username === userInfo.username && inputData.autograph === userInfo.autograph){
            setEditorOpen(false)
            return
        }

        if(inputData.username !== userInfo.username){
            const duplicate_check = await window.svrAPI.request({
                url:`${window.svrAPI.url}/user/infocheck`,
                method:'POST',
                headers:{
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({
                    name: inputData.username
                })
            })
            if(duplicate_check.reqStatus === 200){
                if(duplicate_check.duplicate){
                    setHint('warning','该用户名已被使用')
                    return
                }
            }
            else setHint('error',`无法连接服务器进行验证, 请稍后重试(${duplicate_check.reqStatus})`)
        }
        
        setEdit(false) // 上传时不允许编辑
        const res = await window.svrAPI.request({
            url:window.svrAPI.url.concat('/user/changeinfo'),
            method:'POST',
            headers:{
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                username: inputData.username,
                autograph: inputData.autograph,
                userid: userInfo.userid
            })
        })

        if(res.reqStatus === 200){
            setHint('success','修改个人资料成功')
            // refreshInfo() 不需要更新信息，只需要将现有信息填入变量即可
            setUserInfo({
                ...userInfo,
                username: inputData.username,
                autograph: inputData.autograph
            })
            setEditorOpen(false)
        }
        else setHint('error',`服务器请求出错(${res.reqStatus})`)
        setEdit(true) // 上传结束后允许编辑
    }

    // 用于开启头像上传界面
    const [openUploader,setUploader] = useState(false)

    const [windowSize,setWindowSize] = useContext(WindowSizeContext)

    const avatarGetter = () => {
        if(!ready) return (<Skeleton variant='circular' height={70} width={70}/>)

        if(userInfo.avatar_file_link) return(<Avatar alt="avator" src={userInfo.avatar_file_link} sx={{height:70,width:70}}/>)
        else return(<Avatar sx={{bgcolor:'#427aa1',color:'white',height:70,width:70}}>{userInfo.username.substring(0,2)}</Avatar>)
    } // 用于生成avatar的元素，有图片用图片，没图片用文字，信息没准备好就是Skeleton

    return(
        <Box sx={{width:'100%',height:'100%',overflowY:'scroll',overflowX:'hidden'}}>
            <Container sx={{minWidth:'100%',height:120,bgcolor:'#ebf2fa',display:'flex'}}>
                    <Stack sx={{justifyContent:'center',alignItems:'center'}} spacing={4} direction='row'>
                        {avatarGetter()}
                        <Stack direction='column' spacing={0.2}>
                            {(ready) ? <h2 className='user_title'>{userInfo.username}</h2> : <Skeleton variant='text' height={30} width={60}/>}
                            {(ready) ? <h5 className='user_id_title'>{'UID:'.concat(getUID(userInfo.userid))}</h5> : <Skeleton variant='text' height={20} width={40}/>}
                        </Stack>
                    </Stack>
            </Container>
            <Box sx={{minWidth:'100%',height:windowSize - 170,paddingLeft:2,paddingTop:2}}>
                <Stack direction='row' spacing={1}>
                    <Box sx={{width:(windowSize.smallScreen) ? '65%' : '67%',height:'100%',textAlign:'center'}}>
                        {(resInfo.length === 0) ? <Typography variant='overline' gutterBottom>- 没有更多内容了 -</Typography>   
                                               : <></>}
                    </Box>
                    <Stack direction='column' sx={{width:'30%',minHeight:'100%'}} spacing={2}>
                        <Card sx={{width:'100%',cursor:'default'}}>
                            <CardHeader title='基本信息' avatar={<PersonOutlineSharpIcon />}/>
                            <Divider flexItem/>
                            <CardContent >
                                {(ready) ? 
                                        <Stack direction='column' spacing={2} >
                                            <Stack direction={(windowSize.smallScreen && openEditor) ? 'column' : 'row'} spacing={0.5} 
                                                alignItems={(windowSize.smallScreen && openEditor) ? 'flex-start' : 'center'}>
                                                <p className='user_autograph_title' >用户名:</p>
                                                {(openEditor) ? <TextField required value={inputData.username} disabled={!allowEdit}
                                                                        size='small' onChange={setInputData('username')}/> 
                                                            : <p className='user_autograph'>{userInfo.username}</p>}
                                            </Stack>
                                            <Stack direction='row' spacing={0.5} alignItems={'center'} sx={{maxHeight:300}}>
                                                <p className='user_autograph_title' >用户UID:</p>
                                                <p className='user_autograph'>{getUID(userInfo.userid)}</p>
                                            </Stack>
                                            <Stack direction={(windowSize.smallScreen) ? 'column' : 'row'} spacing={0.5} 
                                                alignItems={(windowSize.smallScreen || openEditor) ? 'flex-start' : 'center'} sx={{maxHeight:300}}>
                                                <p className='user_autograph_title' style={{width:(windowSize.smallScreen) ? 70 : 'auto'}}>个性签名:</p>
                                                {(openEditor) ? <TextField required value={inputData.autograph} size='small' disabled={!allowEdit}
                                                                            multiline rows={4} onChange={setInputData('autograph')}/> 
                                                            : <p className='user_autograph'>{userInfo.autograph}</p>}
                                            </Stack>
                                        </Stack>
                                        :
                                        <Skeleton variant='rectangular' />
                                }
                            </CardContent>
                            <CardHeader title='其它信息' avatar={<MoreHorizSharpIcon />}/>
                            <Divider flexItem/>
                            <CardContent sx={{marginBottom:0,paddingBottom:0}}>
                                {(ready) ?
                                        <Stack direction='column' spacing={2} width={'100%'}>
                                            <Stack direction='row' spacing={0.5} alignItems={'center'}>
                                                <p className='user_autograph_title' >注册时间:</p>
                                                <p className='user_autograph'>{userInfo.register_time.split(' ')[0]}</p>
                                            </Stack>
                                            <Stack direction='row' spacing={0.5} alignItems={'center'} sx={{maxHeight:300}}>
                                                <p className='user_autograph_title' style={{width:(windowSize.smallScreen) ? 65 : 'auto'}}>上次登录:</p>
                                                <p className='user_autograph'>{userInfo.last_login_time}</p>
                                            </Stack>
                                        </Stack>
                                        :
                                        <Skeleton variant='rectangular' />
                                }
                            </CardContent>
                            <CardActions sx={{justifyContent:'flex-end',margin:0}}>
                                <Tooltip title='上传头像'>
                                    <IconButton aria-label='头像' onClick={() => setUploader(true)}>
                                        <PortraitSharpIcon />
                                    </IconButton>
                                </Tooltip>
                                <Tooltip title={(openEditor) ? '保存' : '编辑'}>
                                    <IconButton aria-label={(openEditor) ? '保存信息' : '编辑信息'} onClick={tryUpdateInfo}>
                                        {(openEditor) ? <DoneSharpIcon /> : <EditSharpIcon />}
                                    </IconButton>
                                </Tooltip>
                                {(openEditor) ?
                                <Tooltip title={'取消编辑'}>
                                    <IconButton aria-label='exit' onClick={() => setEditorOpen(false)}>
                                        <CloseSharpIcon />
                                    </IconButton>
                                </Tooltip> : <></>}
                                <Tooltip title={(expandFold) ? '收起面板' : '更多操作'}>
                                    <IconButton aria-label='more' onClick={() => setExpandFold(!expandFold)}>
                                        <ExpandMoreSharpIcon sx={{transition:'all 0.3s ease-in-out',transform:(expandFold) ? 'rotate(180deg)' : ''}}/>
                                    </IconButton>
                                </Tooltip>
                            </CardActions>
                            <Collapse in={expandFold} timeout={'auto'} unmountOnExit>
                                <CardHeader title='账户操作' avatar={<ManageAccountsSharpIcon />}/>
                                <Divider orientation={'horizontal'} flexItem/>
                                    <List sx={{padding:0,margin:0}}>
                                        <ListItemButton onClick={() => setPwdPage(true)}>
                                            <ListItemText primary="修改密码"/>
                                            <ListItemIcon>
                                                <PasswordSharpIcon />
                                            </ListItemIcon>
                                        </ListItemButton>
                                        <ListItemButton onClick={() => setLogoutPage(true)}>
                                            <ListItemText primary="退出账号" />
                                            <ListItemIcon>
                                                <LogoutSharpIcon />
                                            </ListItemIcon>
                                        </ListItemButton>
                                    </List>
                            </Collapse>
                        </Card>
                        <Card sx={{width:'100%',cursor:'default'}}>
                            <CardHeader avatar={<EqualizerSharpIcon />} title='上传统计'/>
                            <CardContent >
                                {(ready) ?
                                        <Stack direction={(windowSize.smallScreen) ? 'column' : 'row'} spacing={(windowSize.smallScreen) ? 1 : 5}
                                            sx={{alignItems:'center',justifyContent:'center'}}>
                                            <Stack direction={(windowSize.smallScreen) ? 'row' : 'column'} 
                                                className='data_box' spacing={(windowSize.smallScreen) ? 2 : 0.5}>
                                                <h4>上传总数</h4>
                                                <h2>{userInfo.works}</h2>
                                            </Stack>
                                            {(windowSize.smallScreen) ? <></> : <Divider orientation='vertical' flexItem />}
                                            <Stack direction={(windowSize.smallScreen) ? 'row' : 'column'}
                                                className='data_box' spacing={(windowSize.smallScreen) ? 2 : 0.5}>
                                                <h4>点赞总数</h4>
                                                <h2>{userInfo.likes}</h2>
                                            </Stack>
                                            {(windowSize.smallScreen) ? <></> : <Divider orientation='vertical' flexItem />}
                                            <Stack direction={(windowSize.smallScreen) ? 'row' : 'column'}
                                                className='data_box' spacing={(windowSize.smallScreen) ? 2 : 0.5}>
                                                <h4>评论总数</h4>
                                                <h2>{userInfo.comments}</h2>
                                            </Stack>
                                        </Stack>
                                        :
                                        <Skeleton variant='rectangular' />
                                }
                            </CardContent>
                        </Card>
                    </Stack>
                </Stack>
                <AvatorUploadPage open={openUploader} setClose={() => setUploader(false)} 
                                  info={userInfo} setInfo={setUserInfo} avatarUpdate={avatarUpdate}/>
            </Box>
        </Box>
    )
}

export function AvatorUploadPage({open,setClose,setInfo,info,avatarUpdate}){
    const [windowFlag,windowSize] = useContext(WindowSizeContext)

    // 用于工具栏选择   
    const [selectValue,setValue] = useState(0)
    const handleClose = () => {
        setValue(0)
        setClose()
    }

    // 用于文件选择
    const [actual_link,setLink] = useState(null)
    // 用于设置提示
    const [hint,setHint] = useState({
        type:null,
        text:null,
        show:false
    })
    const setNewHint = (msg = {type:null,text:null}) => {
        setHint({
            type: msg.type,
            text: msg.text,
            show: !hint.show
        })
    }

    // 用于获取裁剪的图片
    // const [uid,setUID] = useContext(IDContext) //获取id
    // const refreshInfo = useContext(RefreshContext) // 上传成功后使用这个函数刷新用户信息
    const editor = useRef(null)
    const [zoomValue,setZoomValue] = useState(1)
    const [allowUpload,setAllow] = useState(true)
    const sendNewAvatar = async () => {
        if(!actual_link){
            setNewHint({type:'error',text:'请选取图片进行截取'})
            return
        }

        setAllow(false)
        const avatar = editor.current.getImage()
        // const avatarData = new FormData()
        console.log(avatar)
        avatar.toBlob(async (data) => {
            if(!data){
                setNewHint({type:'error',text:'获取图片数据时失败'})
                return
            }
            const arrayBuffer = await data.arrayBuffer()
            // const buffer = new Uint8Array(arrayBuffer)
            const url = window.svrAPI.url.concat(`/user/avator?userid=${info.userid}`)
            console.log(url)
            const res = await window.svrAPI.upload(url,arrayBuffer)
            console.log(res)
            if(res.reqStatus === 200){
                if(res.status === 100){
                    const url = await avatarUpdate()
                    setInfo({
                        ...info,
                        avatar_file_link: url
                    })
                    setNewHint({type:'success',text:'上传头像成功'})
                    

                }
                else
                    setNewHint({type:'error',text:`服务器处理错误(${res.status})`})
            }
            else
                setNewHint({type:'error',text:`服务器发生错误(${res.reqStatus})`})
        },'image/png')

        setAllow(true)
    }

    // 用于打开文件夹对话
    const openDialog = async () => {
        const link = await window.fileAPI.openFileDialog()
        console.log(link)
        setLink(link)
    }


    return(
        <Backdrop
        sx={(theme) => ({ color: '#fff', zIndex: theme.zIndex.drawer + 1 })}
        open={open}
        >
            <Card sx={{width:(windowFlag.smallScreen) ? '30%' : '20%',height:(windowFlag.smallScreen) ? '45%' : '35%'}}>
                <CardHeader avatar={<PortraitSharpIcon />} title='上传头像' 
                            action={<CloseIcon handleClose={handleClose} Icon={<CloseSharpIcon />}/>}/>
                <CardContent >
                    <Stack direction='column' spacing={(windowFlag.smallScreen) ? 1.5 : 2} alignItems='center'>
                        <AvatarEditor image={actual_link} width={180} height={180} borderRadius={180}
                                      border={2} color={[0,0,0,0.7]} ref={editor} scale={zoomValue}/>
                        <Slider size='small' value={zoomValue} onChange={(event,value) => setZoomValue(value)}
                                sx={{width:200}} disabled={!actual_link || actual_link.length === 0} min={0.5} max={3} step={0.1}/>
                        <ToggleButtonGroup size='small' color='primary' value={selectValue} exclusive
                                           onChange={(event,newValue) => setValue(newValue)}>
                            <ToggleButton value='upload' disabled={!allowUpload || actual_link === null || actual_link.length === 0}>
                                <Tooltip title='上传' onClick={sendNewAvatar}>
                                    <FileUploadSharpIcon />
                                </Tooltip>
                            </ToggleButton>
                            <ToggleButton value='select' onClick={openDialog}>
                                <Tooltip title='选择文件'>
                                    <FolderSharpIcon />
                                </Tooltip>
                            </ToggleButton>
                        </ToggleButtonGroup>
                    </Stack>
                </CardContent>      
            </Card>
            <PageAlert type={hint.type} info={hint.text} open={hint.show} clearItem={!open}/>
        </Backdrop>
    )
}