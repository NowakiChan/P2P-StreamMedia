import './home.css';
import Box from '@mui/material/Box';
import Stack from '@mui/material/Stack';
import { WindowNavigate, MenuNavigate } from './windowNavigate';
import { UserPanel } from './userPage/userPage';
import { UploadPage } from './uploadPage/upload';
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
export const PageHintContext = createContext(null)
export const UserInfoContext = createContext(null)

function HomePage(){
    // 用于设置窗口数据
    const [mainAreaSize,setMainAreaSize] = useState({smallScreen:true,width:1000,height:800})

    const [isLogin,setLoginStatus] = useState(false) //后续需要从本地读取
    const [isConnect,setConnectStatus] = useState(true) // 验证与服务器的连接状况
    const [mainAreaIndex,setMainAreaIndex] = useState('browser')
    const [userInfo,setUserInfo] = useState({
        avatar_link : null,
        userid : 0,
        username : 'Unknown',
        autograph : '',
        last_login_time : '1990-01-01 00:00:00',
        register_time : '1990-01-01 00:00:00',
        works : 0,
        likes : 0,
        comments : 0,
        avatar_file_link : null
    })
    


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
                    // setUserId(data.uid)
                    setUserInfo({
                        ...userInfo,
                        userid: data.uid
                    })
                }
            })
        }
    },[])

    return(
        <>
            <Box sx={{width:mainAreaSize.width,padding:'0 0',margin:'0 0',height:mainAreaSize.height,
                      display:'flex',flexDirection:'column'}} >
                
                <Box className='main' sx={{width:'100%',bgcolor:'#ebf2fa',height:30,display:'flex',
                                       flexDirection:'row',alignItems:'center',borderBottom:'1px solid white'}}>
                    {/* 用于顶端的窗口导航栏 */}
                    <WindowNavigate windowSize={mainAreaSize} sendSizeChange={setMainAreaSize}/>
                </Box>
                <Stack direction={'row'} spacing={0} width={'100%'} height={'100%'} sx={{overflowX:'hidden'}}>
                    <Stack direction={'column'} alignItems={'center'} spacing={1} sx={{bgcolor:'#ebf2fa',borderRight:'1px solid white'}}>
                        {/* 左侧竖向导航栏部分 */}
                        <MenuNavigate setIndex={setMainAreaIndex}/>
                    </Stack>
                    <PageIndexContext value={setMainAreaIndex}>
                        <LoginStatusContext value={[isLogin,setLoginStatus]}>
                            <WindowSizeContext value={[mainAreaSize,setMainAreaSize]}>
                                <UserInfoContext value={[userInfo,setUserInfo]}>
                                <PageHintContext value={setNewMainPageHint}>
                                    <PageAlert type={pageHint.type} info={pageHint.text} 
                                            open={pageHint.status} location={{v:'bottom',h:'right'}}/>
                                    <MainPage select={mainAreaIndex} isLogin={isLogin} isConnect={isConnect}/>
                                </PageHintContext>
                                </UserInfoContext>
                            </WindowSizeContext>
                        </LoginStatusContext>
                    </PageIndexContext>
                </Stack>
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
                <UploadPage show={pageIndex.upload}/>
                <UserPanel show={pageIndex.account}/>
                <h1 style={{display:(pageIndex.setting) ? 'block' : 'none'}}>Setting</h1>
            </>
        )
    }
}

export default HomePage;