import Box from "@mui/material/Box";
import { UserToolBar } from "./userPage/userPage";
import { useState } from "react";

export async function getSHA256(text) {
    const encoder = new TextEncoder();
    const data = encoder.encode(text);

    const hashBuffer = await crypto.subtle.digest('SHA-256', data);

    const hashArray = Array.from(new Uint8Array(hashBuffer));
    const hashHex = hashArray
        .map(b => b.toString(16).padStart(2, '0'))
        .join('');

  return hashHex.trim(); // 64位hex字符串
}

export function getUID(id){
    if(typeof(id) == 'number'){
        const id_str = id.toString()
        return '#000000'.substring(0,7 - id_str.length).concat(id_str)
    }
    else return ''      
}

export function verifyPwdFormat(password = ''){
    if (password.length < 8) return false;
    // 至少一个字母
    if (!/[a-zA-Z]/.test(password)) return false;
    // 至少一个数字
    if (!/\d/.test(password)) return false;
    // 不允许空格
    if (/\s/.test(password)) return false;
    return true;
}

export async function accountLogin(info = {identity: '',pwd: '',keepAlive:false}){
    const pwd_sha256 = await getSHA256(info.pwd)
    const alter_identity = (info.identity.at(0) === '#') 
                            ? info.identity.replace(/^#?0+/, '')
                            : info.identity
    const res = await window.svrAPI.request({
        url: `${window.svrAPI.url}/user/login`,
        method:'POST',
        headers:{
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({
            identity : alter_identity,
            keepAlive : info.keepAlive,
            pwd : pwd_sha256
        })
    })
    return res;
}