'use strict';

const electron = require('electron')
const {remote, ipcRenderer, webFrame} = electron

//const win = remote.getCurrentWindow()


// set custom scheme handled by chrome as privileged for fetch

webFrame.registerURLSchemeAsPrivileged('moviesapi', {
    secure: true, // do not trigger mixed content warning
    bypassCSP: false, // bypass content
    allowServiceWorkers: false,
    supportFetchAPI: true
})

// IPC main/renderer communication

window.send = function send( channel, args ) {
    ipcRenderer.send(channel, args)
}

window.receive = function receive( channel, listener ) {
    ipcRenderer.on(channel, listener)
}


// disable eval

window.eval = () => {
    throw new Error('no eval')
}




console.log('preload.js')
