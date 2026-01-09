import Backdrop from "@mui/material/Backdrop";
import Dialog from "@mui/material/Dialog";
import DialogTitle from "@mui/material/DialogTitle";
import Stack from "@mui/material/Stack";
import Button from "@mui/material/Button";
import { useContext, useState } from "react";
import { PageIndexContext } from "../home";


export function LogoutPage({flag,handleClose}){
    const handler = useContext(PageIndexContext)
    const logOutHandler = () => {
        //... handler
        handler('browser')
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