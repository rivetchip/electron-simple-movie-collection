'use strict';

const {remote, ipcRenderer} = require('electron')

//const win = remote.getCurrentWindow()



window.send = function send( channel, args ) {
    ipcRenderer.send(channel, args)
}

window.receive = function receive( channel, listener ) {
    ipcRenderer.on(channel, listener)
}




console.log('preload.js')
