import './home.css';
import Box from '@mui/material/Box';
import Grid from '@mui/material/Grid';
import Container from '@mui/material/Container';
import { WindowNavigate, MenuNavigate, NavigateLogo } from './windowNavigate';

// main structure:
// top navigate bar : window operation
// main left : function bar
// main right : display box
//              display box left : info display area
//              display box right : tool box
function HomePage(){
    return(
        <>
            <Box sx={{width:1000,height:800,display:'flex',flexDirection:'column'}} >
                <Container maxWidth='lg' sx={{bgcolor:'#050517',height:50,margin:'0 0',padding:'0 0'}} className='main'>
                    {/* <h1>top navigate</h1> */}
                    <Grid container spacing={1} sx={{padding:'0 0',margin:'0 0'}}>
                        <Grid size={2}>
                            <NavigateLogo />
                        </Grid>
                        <Grid size={6} sx={{textAlign:'center'}}>
                            <MenuNavigate />
                        </Grid>
                        <Grid size={4} sx={{justifyContent:'flex-end',alignItems:'center',display:'flex'}}>
                            <WindowNavigate />
                        </Grid>
                    </Grid>
                </Container>
                <Container maxWidth='lg' sx={{height:750,margin:'0 0',padding:'0 0'}}>
                    {/* <h2>main display area</h2> */}
                </Container>
            </Box>
           
        </>
    )
}

export default HomePage;