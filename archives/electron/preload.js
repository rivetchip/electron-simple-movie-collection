'use strict';

// Electron flow
const {remote} = require('electron')
const {dialog} = remote

const remoteWindow = remote.getCurrentWindow()

const {dirname} = require('path')
const {
    readFile: fsreadFile, createReadStream: fsCreateReadStream,
    writeFile: fswriteFile, createWriteStream: fsCreateWriteStream
} = require('fs')


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

    async openCollection(parser) {
        let filename = await showOpenDialog({
            properties: ['openFile'],
            filters: [
                {name: 'Movie Collection', extensions: ['json']}
            ]
        })

        state.storageFilename = filename // reinit

        return parser(await readFileStream(filename))
    },

    async saveCollection(storage, stringify) {
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

        return writeFileStream(filename, stringify(storage))
    },

    async getPoster(filename) {

// todo: string replace .. . / ~

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

async function readFile(filename) {
    return new Promise((resolve, reject) => {
        fsreadFile(filename, 'utf8', (error, content) => {
            return error ? reject(error.message) : resolve(content)
        })
    })
}

async function writeFile(filename, content) {
    return new Promise((resolve, reject) => {
        fswriteFile(filename, content, 'utf8', (error) => {
            return error ? reject(error.message) : resolve(true)
        })
    })
}

async function readFileStream(filename) {
    return new Promise((resolve, reject) => {
        let content = ''
        let stream = fsCreateReadStream(filename, {
            flags: 'r',
            encoding: 'utf8',
            autoClose: true,
            highWaterMark: 64 * 1024 //64kb
        })
        .on('data', (chunk) => {
            content += chunk
        })
        .on('close', () => {
            resolve(content)
        })
        .on('error', (error) => {
            reject(error)
        })
    })
}

async function writeFileStream(filename, content) {
    return new Promise((resolve, reject) => {
        let stream = fsCreateWriteStream(filename, {
            flags: 'w',
            encoding: 'utf8',
            autoClose: true
        })
        .on('close', () => {
            resolve(true)
        })
        .on('error', (error) => {
            reject(error)
        })
        .write(content) // TODO chunk string ?
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

// function chunks(string, chunkSize, callback) {
//     while(string) {
//         if(string.length > chunkSize) {
//             callback(string.slice(0, chunkSize))
//             string = string.substr(chunkSize)
//         } else {
//             callback(string)
//             break
//         }
//     }
// }




console.log('preload.js')
