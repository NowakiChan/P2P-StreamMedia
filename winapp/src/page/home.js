import './home.css';
import Box from '@mui/material/Box';
import Grid from '@mui/material/Grid';
import Container from '@mui/material/Container';
import { WindowNavigate, MenuNavigate, NavigateLogo } from './windowNavigate';
import { UserPanel } from './userPage/userPage';
import { LoadingPage } from './errorPage';
import { createContext, useContext, useState } from 'react';

// main structure:
// top navigate bar : window operation
// main left : function bar
// main right : display box
//              display box left : info display area
//              display box right : tool box

export const PageIndexContext = createContext(null)
export const LoginStatusContext = createContext(null)

function HomePage(){
    const [fullscreenflag,setFullScreenFlag] = useState(true)
    const [mainAreaHeight,setMainAreaHeight] = useState(800)
    const [pageIndex,setPageIndex] = useState('loading')
    const [isLogin,setLoginStatus] = useState(false) //后续需要从本地读取
    
    const pageSelector = () => {
        if(pageIndex === 'browser'){
            return(<h1>Browser Page</h1>)
        }
        else if(pageIndex === 'upload'){
            return(<h1>Upload Page</h1>)
        }
        else if(pageIndex === 'account'){
            return( <UserPanel /> )
        }
        else if(pageIndex === 'setting'){
            return(<h1>Setting Page</h1>)
        }

        return( <LoadingPage /> )
    }

    return(
        <>
            <Box sx={{width:'100%',padding:'0 0',margin:'0 0',
                      height:'100%',display:'flex',flexDirection:'column'}} >
                <Container maxWidth='100%' sx={{bgcolor:'#1976d2',height:50,margin:'0 0',padding:'0 0'}} className='main'>
                    {/* <h1>top navigate</h1> */}
                    <Grid container spacing={(fullscreenflag) ? 1 : 20} sx={{padding:'0 0',margin:'0 0'}} >
                        <Grid size={(fullscreenflag) ? 2 : 1} >
                            <Container className='main' maxWidth='sm' sx={{position:'absolute',left:3}}>
                                <NavigateLogo />
                            </Container>
                        </Grid>
                        <Grid size={9} >
                            <MenuNavigate setIndex={setPageIndex}/>
                        </Grid>
                        <Grid size={1}  sx={{justifyContent:'flex-end',alignItems:'center',display:'flex'}}>
                            <WindowNavigate flag={fullscreenflag} setFlag={setFullScreenFlag} sendSizeChange={setMainAreaHeight}/>
                        </Grid>
                    </Grid>
                </Container>
                <Box sx={{height:mainAreaHeight - 50,width:'100%',margin:'0 0',padding:'0 0'}}>
                    {/* <h2>main display area</h2> */}
                    <PageIndexContext value={setPageIndex}>
                        <LoginStatusContext value={setLoginStatus}>
                            {pageSelector()}
                        </LoginStatusContext>
                    </PageIndexContext>
                </Box>
            </Box>
           
        </>
    )
}

export default HomePage;