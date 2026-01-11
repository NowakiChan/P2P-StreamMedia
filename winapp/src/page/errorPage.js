import CloudOffSharpIcon from '@mui/icons-material/CloudOffSharp';
import CircularProgress from '@mui/material/CircularProgress';
import WarningAmberSharpIcon from '@mui/icons-material/WarningAmberSharp';
import Stack from '@mui/material/Stack';
import './home.css';
import './errorPage.css';
import Button from '@mui/material/Button';

export function DisconnectErrorPage(eventHandler){
    return(
        <Stack direction='column' spacing={2} className='main_hint_page'>
            <Stack direction='row' spacing={1.5} sx={{alignItems:'center',justifyContent:'center'}}>
                <CloudOffSharpIcon sx={{transform:'scale(1.4)'}}/>
                <h3 className='main_hint_title'>无法连接到中央服务器</h3>
            </Stack>
            <p className='main_hint_text'>您依旧可以使用离线下载的资源, 如要使用完整功能, 请尝试重新连接</p>
            <Button variant='text' size='medium' sx={{maxWidth:'50%'}} onClick={() => eventHandler()}>重新连接</Button>
        </Stack>
    )
}

export function ErrorPage(eventHandler,props){
    return(
        <Stack direction='column' spacing={2} className='main_hint_page'>
            <Stack direction='row' spacing={1.5} sx={{alignItems:'center',justifyContent:'center'}}>
                <WarningAmberSharpIcon sx={{transform:'scale(1.4)'}}/>
                <h3 className='main_hint_title'>似乎发生了错误</h3>
            </Stack>
            <p className='main_hint_text'>错误信息: {props.text}</p>
            {(props.type === 'handler') ? 
                <Button variant='text' size='medium' sx={{maxWidth:'50%'}} onClick={() => eventHandler()}>{props.title}</Button>
                : <></>
            }
        </Stack>
    )
}

export function LoadingPage(){
    return(
        <CircularProgress className='main_hint_page'/>
    )
}

