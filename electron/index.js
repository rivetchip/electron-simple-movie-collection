const package = require('./package.json')

// Electron flow
const {platform, argv} = process

const {app, BrowserWindow, ipcMain, dialog} = require('electron')

const {join: pathjoin} = require('path')
const {
    access: fsaccess,
    readFile: fsreadFile,
    writeFile: fswriteFile,
    constants: fsconstants
} = require('fs')

// Application fow
const {logger} = require('./logger')

// User flow
const userDataPath = app.getPath('userData')
const userSettingsFilename = pathjoin(userDataPath, 'settings.json');

const debug = argv.includes('--debug')

if(platform == 'linux') {
    //--enable-transparent-visuals --disable-gpu
    app.disableHardwareAcceleration() // transparency and fire win.ready-show
}

if(debug) {
    const electronIpcLog = require('./electron-ipc-log')

    electronIpcLog((event) => {
        let {channel, data, sent, sync} = event
        let args = [sent ? '-->' : '<--', channel, ...data]
        let mode = sync ? 'ipc:sync' : 'ipc'

        console.log(mode, ...args)
    })
}

// app.disableHardwareAcceleration()

let win

function createWindow() {

    let width = 1100
    let height = 800

    win = new BrowserWindow({
        icon: pathjoin(__dirname, 'app-icon.png'),
        width,
        height,
        minWidth: width,
        minHeight: height,
        show: false, // wait when ready : prevents white flickering
        //backgroundColor: '#fff',

        webPreferences: {
            nodeIntegration: false,
            // contextIsolation: true,
            preload: pathjoin(__dirname, 'preload.js'),
        },

        // borderless frame
        resizable: true,
        frame: debug ? true : false,
        transparent: true,
        titleBarStyle: 'hidden', // macos
    })

    if( !debug ){
        win.setMenu(null) // no menu
    }

    if( win.setSheetOffset ) {
        win.setSheetOffset(50) // +titlebar height on mac
    }

    win.loadFile(pathjoin(__dirname, 'www/index.html'))

    // Launch fullscreen with DevTools open
    if(debug) {
        win.webContents.openDevTools()
    }

    // Show window when page is ready
    win.once('ready-to-show', () => {
        console.log('ready-to-show')
        
        win.show()
        win.focus()
    })

    /*win.webContents.on('dom-ready', function() {
        console.log('dom-ready')
    })*/

    win.on('closed', () => {
        win = null
    })

    win.on('maximize', () => {
        send('fullscreen-status-changed', true)
    })

    win.on('unmaximize', () => {
        send('fullscreen-status-changed', false)
    })
}

// create brower win + workaround for linux transparency
app.on('ready', () => setTimeout(createWindow, 100)) // TODO disable on future version


app.on('window-all-closed', () => {
    if( platform !== 'darwin' ) { // macos stay in dock
        app.quit()
    }
})

app.on('activate', () => { // mac
    if(!win) {
        createWindow()
    }
})

// TODO drag onto
app.on('open-file', (event, filePath) => {
    console.log(filePath)
})

process.on('uncaughtException', (error) => logger('uncaughtException', error))

process.on('unhandledRejection', (error) => logger('unhandledRejection', error))









/**
 * Receive an event from the renderer
 * listener: (reply, reject, args)
 * reply(args) : send back a OK response
 * reject(reason) : send back a NOTOK response
 * args* : args got from the renderer
 * 
 * @param {String} channel 
 * @param {Function} listener 
 */
const receive = (channel, listener) => {

    return ipcMain.on(channel, (event, message) => {

        const sender = event.sender
        const {responseChannel, args} = message // from the renderer

        const reply = (args) => sender.send(responseChannel, {args}) // send back
        const reject = (error) => sender.send(responseChannel, {error}) // send error back

        return listener(reply, reject, args)
    })
}

// send a message to the renderer TODO remove
const send = (channel, args) => {
    return win.webContents.send(channel, args)
}





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

function fileExist(filename) {
    return new Promise((resolve, reject) => {
        fsaccess(filename, fsconstants.F_OK, (error) => {
            return error ? reject(error) : resolve(data)
        })
    })
}

function readFile(filename) {
    return new Promise((resolve, reject) => {
        fsreadFile(filename, 'utf8', (error, data) => {
            return error ? reject(error) : resolve(data)
        })
    })
}

function writeFile(filename, content) {
    return new Promise((resolve, reject) => {
        fswriteFile(filename, content, 'utf8', (error) => {
            return error ? reject(error) : resolve()
        })
    })
}

function showOpenDialog(options) {
    return new Promise((resolve, reject) => {
        dialog.showOpenDialog(options, (filenames) => {
            return filenames ? resolve(filenames) : reject('showOpenDialog: no file set')
        })
    })
}

function showSaveDialog(options) {
    return new Promise((resolve, reject) => {
        dialog.showSaveDialog(options, (filename) => {
            return filename ? resolve(filename) : reject('showSaveDialog: no file set')
        })
    })
}

function showErrorBox (title, content) {
    dialog.showErrorBox(title, content)
}




































//// Application events flow

let storageFilename // current opened file








// const the collection from the content of a file or anything ; and get the colection
const setCatalogStorageFrom = (content) => {
    // parse content of file, & assigns defaults values
    const defaults = {
        version: 1,
        options: {},
        collection: [] // Map
    }

    const storage = Object.assign({}, defaults, content) // shallow merge

    //.map((product, index) => [index, product])

    // set the new storage to the catalog

    catalogStorage = storage
}









// read a file collection ; and return a simple collection of products
const onReadStorageFilename = (filename) => {
    return readFile(filename).then((content) => JSON.parse(content))
}

// save the collection
const onSaveFileCatalogStorage = (filename) => {
    let content = getCatalogStorageForSaving() // construct catalogue
    content = JSON.stringify(content) // TODO check if var not overitted ; passby référence

    return writeFile(filename, content)
}






// client api

receive('application-close', () => {
    win.close()
})

receive('application-minimize', () => {
    win.minimize()
})

receive('application-maximize', () => {
    if( win.isMaximized() ) {
        win.unmaximize()
    } else {
        win.maximize()
    }
})


receive('open-collection-dialog', async (reply) => {

    let [errorOpen, filePaths] = await to(showOpenDialog({
        properties: ['openFile'],
        filters: [
            {name: 'Movie Collection', extensions: ['json']}
        ]
    }))

    if(errorOpen){
        return logger(errorOpen)
    }

    // then we try to read the current file

    let filename = filePaths[0] // get first file

    let [errorRead, collection] = await to(onReadStorageFilename(filename))

    if(errorRead){
        showErrorBox('Cannot open file', errorRead.message || errorRead)

        return logger(errorRead)
    }

    // then send it back to the client

    return reply({collection})
})

receive('save-collection-dialog', async (reply) => {

    const onSaveCollection = async (filename) => {
        // when validate save file

        let [errorSave, successSave] = await to(onSaveFileCatalogStorage(filename))

        if( errorSave ) {
            logger(errorSave)

            return showErrorBox('Cannot save file', errorSave.message || errorSave)
        }
        
        // set the opened file

        setCatalogStorageFilename(filename)

        // send notification

        send('notification', 'Fichier sauvegardé!')
    }

    // get current opened file (if already set)

    let filename = getCatalogStorageFilename()

    if( filename ) {
        return onSaveCollection(filename)
    }

    // the file doesn't exist yet, we show the prompt

    let [errorSave, newfilename] = await to(showSaveDialog({
        properties: ['openFile'],
        filters: [
            {name: 'Movie Collection', extensions: ['json']}
        ]
    }))

    if( errorSave ) {
        return logger('showSaveDialog: no file set')
    }

    // save file if all is correct

    return onSaveCollection(newfilename)
})







console.log('Running...')

console.log(
    'electron', process.versions.electron,
    'node', process.versions.node,
    'chrome', process.versions.chrome
)
