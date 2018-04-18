'use strict';

const electron = require('electron')
const {remote, ipcRenderer, webFrame} = electron

const remoteWindow = remote.getCurrentWindow()



// set custom scheme handled by chrome as privileged for fetch

webFrame.registerURLSchemeAsPrivileged('moviesapi', {
    secure: true, // do not trigger mixed content warning
    bypassCSP: false, // bypass content
    allowServiceWorkers: false,
    supportFetchAPI: true
})

// IPC main/renderer communication

window.send = ( channel, args ) => {
    return ipcRenderer.send(channel, args)
}

window.receive = ( channel, listener ) => {
    return ipcRenderer.on(channel, listener)
}

/**
 * Send/receive events communications with Main
 * eg: send 'hello', Main reply with 'hello' and get the results as a promise
 * 
 * @param {String} channel 
 * @param {Object} args 
 */
window.ipc = (channel, args) => {
    return new Promise((resolve, reject) => {
        ipcRenderer.send(channel, args) // send event
        ipcRenderer.once(channel, (event, result) => { // reply event   // TODO: max event listener            
            resolve(result)
        })
    })
}




// disable eval

// window.eval = global.eval = () => {
//     throw new Error('no eval')
// }




console.log('preload.js')
