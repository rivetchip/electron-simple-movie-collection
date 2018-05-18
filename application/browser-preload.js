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

// IPC main/renderer communication OLD DEPRECATED TODO use ipc()

window.send = ( channel, args ) => {
    return ipcRenderer.send(channel, args)
}

window.receive = ( channel, listener ) => {
    return ipcRenderer.on(channel, listener)
}

/**
 * Send/receive events communications with IPC Main
 * eg: send 'hello', Main reply with 'hello' and get the results as a promise
 * 
 * @param {String} channel 
 * @param {Object} args 
 */
window.ipc = (channel, args) => {

    const responseChannel = 'id-'+Math.random()+Date.now() // on the fly

    return new Promise((resolve, reject) => {

        ipcRenderer.once(responseChannel, (event, message) => {
            const {error, args} = message

            return error ? reject(error) : resolve(args)
        })

        ipcRenderer.send(channel, {
            responseChannel, args
        })
    })
}




// disable eval

window.eval = global.eval = () => {
    throw new Error('no eval')
}




console.log('preload.js')
