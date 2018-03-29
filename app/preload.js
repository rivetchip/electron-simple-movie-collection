'use strict';

const {remote, ipcRenderer} = require('electron')

//const win = remote.getCurrentWindow()


// IPC main/renderer communication

window.send = function send( channel, args ) {
    ipcRenderer.send(channel, args)
}

window.receive = function receive( channel, listener ) {
    ipcRenderer.on(channel, listener)
}


// disable eval

window.eval = global.eval = () => {
    throw new Error('no eval')
}




console.log('preload.js')
