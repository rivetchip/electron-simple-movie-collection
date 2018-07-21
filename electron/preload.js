'use strict';

// Electron flow
const {remote} = require('electron')
const {dialog} = remote

const remoteWindow = remote.getCurrentWindow()

const {join: pathjoin} = require('path')
const {
    access: fsaccess,
    readFile: fsreadFile,
    writeFile: fswriteFile,
    constants: fsconstants
} = require('fs')

let storageFilename // current opened file

const bridge = {  // native bridge

    applicationClose(status) {
        remoteWindow.close()
    },

    applicationMinimize() {
        remoteWindow.minimize()
    },

    applicationMaximize() {
        remoteWindow.isMaximized() ? remoteWindow.unmaximize() : remoteWindow.maximize()
    },

    async openCollection() {
        let filename = await showOpenDialog({
            properties: ['openFile'],
            filters: [
                {name: 'Movie Collection', extensions: ['json']}
            ]
        })

        return readFile(filename, JSON.parse)
    },

    async saveCollection(storage) {
        let filename = await showSaveDialog({
            properties: ['openFile'],
            filters: [
                {name: 'Movie Collection', extensions: ['json']}
            ]
        })

        return writeFile(filename, storage, JSON.stringify)

        // no file set, prompt todo

        if(storageFilename) {
            
        }
    },

    async getPoster(filename) {

    },

    async savePoster(filename, content) {
        
    }
}

window['ElectronInterface'] = bridge


// bridge.openCollection







/**
 * Using async, await, with error handling
 * return [error, resolve datas]
 * 
 * @param {Promise} promise 
 */
function to(promise) {
    return promise.then((response) => {
        return [null, response];
    })
    .catch(error => [error]);
}

async function fileExist(filename) {
    return new Promise((resolve, reject) => {
        fsaccess(filename, fsconstants.F_OK, (error) => {
            //FIXME check error for parser
            return error ? reject(error) : resolve(data)
        })
    })
}

async function readFile(filename, parser) {
    return new Promise((resolve, reject) => {
        fsreadFile(filename, 'utf8', (error, content) => {
            //FIXME check error for parser
            return error ? reject(error) : resolve(parser && parser(content) || content)
        })
    })
}

async function writeFile(filename, content, parser) {
    return new Promise((resolve, reject) => {
        fswriteFile(filename, parser && parser(content) || content, 'utf8', (error) => {
            return error ? reject(error) : resolve()
        })
    })
}

async function showOpenDialog(options) {
    return new Promise((resolve, reject) => {
        dialog.showOpenDialog(options, (filenames) => {
            return filenames ? resolve(filenames[0]) : reject()
        })
    })
}

async function showSaveDialog(options) {
    return new Promise((resolve, reject) => {
        dialog.showSaveDialog(options, (filename) => {
            return filename ? resolve(filename) : reject()
        })
    })
}













// window.send = ( channel, args ) => {
//     return ipcRenderer.send(channel, args)
// }

// window.receive = ( channel, listener ) => {
//     return ipcRenderer.on(channel, listener)
// }

/**
 * Send/receive events communications with IPC Main
 * eg: send 'hello', Main reply with 'hello' and get the results as a promise
 * 
 * @param {String} channel 
 * @param {Object} args 
 */
// window.ipc = (channel, args) => {
//     // console.log('window.ipc', channel, args)

//     const responseChannel = 'id-'+Math.random()+Date.now() // on the fly

//     return new Promise((resolve, reject) => {

//         ipcRenderer.once(responseChannel, (event, message) => {
//             const {error, args} = message

//             return error ? reject(error) : resolve(args)
//         })

//         ipcRenderer.send(channel, {
//             responseChannel, args
//         })
//     })
// }






console.log('preload.js')