import './home.css';
import Box from '@mui/material/Box';
import Grid from '@mui/material/Grid';
import Container from '@mui/material/Container';
import Stack from '@mui/material/Stack';
import { WindowNavigate, MenuNavigate, NavigateLogo } from './windowNavigate';
import { LoginHintPage, UserPanel } from './userPage/userPage';
import { createContext, useEffect, useState } from 'react';
import { PageAlert } from './userPage/backDropPage';
import { DisconnectErrorPage } from './errorPage';

// main structure:
// top navigate bar : window operation
// main left : function bar
// main right : display box
//              display box left : info display area
//              display box right : tool box

export const PageIndexContext = createContext(null)
export const LoginStatusContext = createContext(null)
export const WindowSizeContext = createContext(null)
export const IDContext = createContext(null)
export const PageHintContext = createContext(null)

function HomePage(){
    const [fullscreenflag,setFullScreenFlag] = useState(true)
    const [mainAreaHeight,setMainAreaHeight] = useState(800)
    const [isLogin,setLoginStatus] = useState(false) //后续需要从本地读取
    const [isConnect,setConnectStatus] = useState(true) // 验证与服务器的连接状况
    const [userID,setUserId] = useState(null) // 记录用户id，而不是直接用浏览器存储
    const [mainAreaIndex,setMainAreaIndex] = useState('browser')


    // 用于页面提示(右下角)的开关和信息设置
    const [pageHint,setPageHint] = useState({
        type:'info',
        text:'',
        status:false
    })
    const setNewMainPageHint = (newType = 'info',newText = '') => {
        setPageHint({
            type: newType,
            text: newText,
            status: !pageHint.status
        })
    }

    // 用于在初始加载各项数据
    useEffect(() => {
        // console.log('Try verify token')
        if(!isLogin){
            window.svrAPI.tokenVerify((data) => {
                if(data.reqStatus === 200 && data.status === 100){
                    setLoginStatus(true)
                    setUserId(data.uid)
                }
            })
        }
    },[])

    return(
        <>
            <Box sx={{width:'100%',padding:'0 0',margin:'0 0',
                      height:'100%',display:'flex',flexDirection:'column'}} >
                <Box  sx={{bgcolor:'#1976d2',height:50,display:'flex',justifyContent:'flex-start',alignItems:'center',paddingLeft:2,paddingRight:2}} 
                        className='main'>
                    {/* <h1>top navigate</h1> */}
                    <NavigateLogo />
                    <MenuNavigate setIndex={setMainAreaIndex}/>
                    <WindowNavigate flag={fullscreenflag} setFlag={setFullScreenFlag} sendSizeChange={setMainAreaHeight}/>
                </Box>
                <Box sx={{height:mainAreaHeight - 50,width:'100%',margin:'0 0',padding:'0 0',overflowX:'hidden'}}>
                    {/* <h2>main display area</h2> */}
                    <PageIndexContext value={setMainAreaIndex}>
                        <LoginStatusContext value={[isLogin,setLoginStatus]}>
                            <WindowSizeContext value={[fullscreenflag,mainAreaHeight]}>
                                <IDContext value={[userID,setUserId]}>
                                    <PageHintContext value={setNewMainPageHint}>
                                        <PageAlert type={pageHint.type} info={pageHint.text} 
                                                   open={pageHint.status} location={{v:'bottom',h:'right'}}/>
                                        <MainPage select={mainAreaIndex} isLogin={isLogin} isConnect={isConnect}/>
                                    </PageHintContext>
                                </IDContext>
                            </WindowSizeContext>
                        </LoginStatusContext>
                    </PageIndexContext>
                </Box>
            </Box>
        </>
    )
}

export function MainPage({select = '',isLogin = false,isConnect = false}){
    // 用于设置各个页面
    const [lastIndex,setLastIndex] = useState('browser')
    const [pageIndex,setIndex] = useState({
        browser: true,
        upload: false,
        account: false,
        setting: false
    })
    useEffect(() => {
        if(isConnect && select in pageIndex){
            setIndex({
                ...pageIndex,
                [lastIndex]: false,
                [select]: true
            })
            setLastIndex(select)
        }

    },[select])

    if(!isConnect) return (<DisconnectErrorPage />)
    else{
        return(
            <>
                <h1 style={{display:(pageIndex.browser) ? 'block' : 'none'}}>Browser</h1>
                <h1 style={{display:(pageIndex.upload) ? 'block' : 'none'}}>Upload</h1>
                <UserPanel show={pageIndex.account}/>
                <h1 style={{display:(pageIndex.setting) ? 'block' : 'none'}}>Setting</h1>
            </>
        )
    }

    switch(select) {
        case 'browser' :
            return <h1>browser</h1>
        case 'upload' :
            return <h1>upload</h1>
        case 'setting' :
            return <h1>setting</h1>
        default :
            return <h1>Unknown</h1>
    }
}

export default HomePage;