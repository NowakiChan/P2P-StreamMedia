import Box from "@mui/material/Box";
import Stack from "@mui/material/Stack";
import Tab from "@mui/material/Tab";
import Tabs from "@mui/material/Tabs";
import { useContext, useState } from "react";
import { LoginStatusContext } from "../home";
import { LoginHintPage } from "../userPage/userPage";
import Badge from "@mui/material/Badge";
import Typography from "@mui/material/Typography";


export function UploadPage({show}){
    const [loginStatus,setLoginStatus] = useContext(LoginStatusContext)

    if(loginStatus){
        return(
            <Box sx={{height:'100%',width:'100%',overflowX:'hidden',overflowY:'scroll',
                      display:(show) ? 'flex' : 'none',flexDirection:'column',margin:0,padding:0}}>
                <Stack direction={'column'} spacing={0}>
                    <UploadPageNavigate />
                </Stack>
            </Box>
        )
    }
    else return((show) ? <LoginHintPage /> : <></>)
}

export function UploadPageNavigate(){
    // 用于切换标签页
    const [pageIndex,setIndex] = useState(0)
    // 用于切换显示是否有正在进行的任务
    const [newProgress,setNewProgress] = useState({
        sync : false,
        transmit : false,
        create: false
    })
    const setProgress = (key,flag = false) => {
        setNewProgress({
            ...newProgress,
            [key] : flag
        })
    }

    return(
        <Tabs value={pageIndex} onChange={(sevent,newValue) => setIndex(newValue)} 
              sx={{bgcolor:'#ebf2fa', '& .MuiTabs-indicator': {
                   backgroundColor: '#427aa1'}}} >
            <Tab icon={(newProgress.sync) ? <Badge color='primary' badgeContent='' variant='dot'>
                <Typography variant='button'>同步中</Typography>
            </Badge> : <Typography variant='button'>同步中</Typography>}/>
            <Tab icon={(newProgress.transmit) ? <Badge color='primary' badgeContent='' variant='dot'>
                <Typography variant='button'>传输中</Typography>
            </Badge> : <Typography variant='button'>传输中</Typography>}/>
            <Tab label='已缓存'/> 
            <Tab icon={(newProgress.create) ? <Badge color='primary' badgeContent='' variant='dot'>
                <Typography variant='button'>新建任务</Typography>
            </Badge> : <Typography variant='button'>新建任务</Typography>}/>
        </Tabs>
    )
}