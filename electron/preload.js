'use strict';

// Electron flow
const {remote} = require('electron')
const {dialog} = remote

const remoteWindow = remote.getCurrentWindow()

const {dirname} = require('path')
const {readFile: fsreadFile, writeFile: fswriteFile} = require('fs')


const state = {
    storageFilename: null // current opened file
}

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

        state.storageFilename = filename // reinit

        return readFile(filename, JSON.parse)
    },

    async saveCollection(storage) {
        let filename = state.storageFilename

        if(!filename) { // no file set, prompt
           filename = await showSaveDialog({
                properties: ['openFile'],
                filters: [
                    {name: 'Movie Collection', extensions: ['json']}
                ]
            })

            state.storageFilename = filename
        }

        return writeFile(filename, storage, JSON.stringify)
    },

    async getPoster(filename) {
        let folder = dirname(state.storageFilename) + '/posters/'

        return readFile(folder + filename)
    },

    async savePoster(filename, content) {
        let folder = dirname(state.storageFilename) + '/posters/'

        return writeFile(folder + filename, content)
    }
}

window['ElectronInterface'] = bridge






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

async function readFile(filename, parser) {
    return new Promise((resolve, reject) => {
        fsreadFile(filename, 'utf8', (error, content) => {
            error && reject(error.message)
            try {
                resolve(parser ? parser(content) : content)
            } catch(ex) {
                reject(ex)
            }
        })
    })
}

async function writeFile(filename, content, parser) {
    return new Promise((resolve, reject) => {
        try {
            content = parser ? parser(content) : content
        } catch(ex) {
            reject(ex)
        }
        fswriteFile(filename, content, 'utf8', (error) => {
            return error ? reject(error.message) : resolve(true)
        })
    })
}

async function showOpenDialog(options) {
    return new Promise((resolve, reject) => {
        dialog.showOpenDialog(options, (filenames) => {
            return filenames ? resolve(filenames[0]) : reject(false)
        })
    })
}

async function showSaveDialog(options) {
    return new Promise((resolve, reject) => {
        dialog.showSaveDialog(options, (filename) => {
            return filename ? resolve(filename) : reject(false)
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