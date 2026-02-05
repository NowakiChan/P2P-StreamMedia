const { default: axios } = require("axios")

const svrURL = 'http://101.132.162.15:6001'

async function svrRequest(event,option){
    console.log(option)
    // try{
    //     const res = fetch(option.url,{
    //         method: option.method,
    //         headers: option.headers,
    //         body: option.body
    //     })
    //     const data = await res.json()
    //     if(res.status === 200)
    //         return { ...data,reqStatus: res.status}
    //     else
    //         return { reqStatus: res.status }
    // }
    // catch(e){
    //     console.log("Main Thread : Request error -> ",e.message)
    //     return {
    //         reqStatus: -1,
    //         info: e.message
    //     }
    // }
    if(option.method.toUpperCase() === 'POST'){
        try{
            const res = await axios.post(option.url,option.body,{headers:{...option.headers}})
            return {
                reqStatus: res.status,
                ...res.data
            }
        }
        catch(err){
            return{
                reqStatus: -1,
                info: err.message
            }
        }
    }
    else if(option.method.toUpperCase() === 'GET'){
        try{
            const res = await axios.get(option.url,{params:{...option.params}})
            return {
                reqStatus: res.status,
                ...res.data
            }
        }
        catch(err){
            return{
                reqStatus: -1,
                info: err.message
            }
        }
    }

}

module.exports = {svrRequest,svrURL}