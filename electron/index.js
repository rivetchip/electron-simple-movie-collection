const package = require('./package.json')

// Electron flow
const {platform, argv} = process

const {app, BrowserWindow, ipcMain, dialog} = require('electron')

const {join: pathjoin} = require('path')
const fs = require('fs')

// Application fow
const {logger} = require('./logger')

// User flow
const userDataPath = app.getPath('userData')
const userSettingsFilename = pathjoin(userDataPath, 'settings.json');

const debug = argv.includes('--debug')

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
let onlineStatusWindow




// Someone tried to run a second instance, we should focus our window
const shouldStartInstance = app.makeSingleInstance((commandLine, workingDirectory) => {
    if( win ){
        if( !win.isVisible() ) {
            win.show()
        }
        if( win.isMinimized() ) {
            win.restore()
        }
        win.focus()
    }
    return true
})

if(shouldStartInstance) {
    app.quit()
}

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
            preload: 'electron-preload.js',
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

    win.webContents.on('dom-ready', function() {
        console.log('dom-ready')
    })

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






//// Application events flow


let catalogStorage = {} // content of the collection, options & others stuffs

let catalogStorageFilename // current opened file


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
const to = (promise) => {
    return promise.then((response) => {
       return [null, response];
    })
    .catch(error => [error]);
 }

const fileExist = (filename) => {
    return new Promise((resolve, reject) => {
        fs.access(filename, fs.F_OK, (error) => {
            return error ? reject(error) : resolve(data)
        })
    })
}

const readFile = (filename) => {
    return new Promise((resolve, reject) => {
        fs.readFile(filename, 'utf8', (error, data) => {
            return error ? reject(error) : resolve(data)
        })
    })
}

const writeFile = (filename, content) => {
    return new Promise((resolve, reject) => {
        fs.writeFile(filename, content, 'utf8', (error) => {
            return error ? reject(error) : resolve()
        })
    })
}


//set the catalog storage filename
const setCatalogStorageFilename = (filename) => {
    catalogStorageFilename = filename
}

const getCatalogStorageFilename = () => {
    return catalogStorageFilename
}

// empty the catalog storage
const emptyCatalogStorage = () => {
    catalogStorage = {}
}

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

// return the collection of all products
const getCatalogStorageCollection = () => {
    return catalogStorage.collection
}

// get a single product
const getCatalogStorageProduct = (index) => {

    let product = catalogStorage.collection[index]

    return product 
}

// construct file for when we want to save it to a file or other
// TODO : if save when there is no movies : options, version are not set
const getCatalogStorageForSaving = () => {
    let content = catalogStorage

    return content
}

// return a simple collection of products
const getProductsSimpleFrom = (collection) => {

    let products = []

    collection.forEach((product) => {
        // [index, {product}] ; foreach.index = real index

        products.push({
            title: product.title,
            favorite: product.favorite,
            // poster: product.poster
        })
    })

    return products
}


const showOpenDialog = (options) => {
    return new Promise((resolve, reject) => {
        dialog.showOpenDialog(options, (filenames) => {
            return filenames ? resolve(filenames) : reject('showOpenDialog: no file set')
        })
    })
}

const showSaveDialog = (options) => {
    return new Promise((resolve, reject) => {
        dialog.showSaveDialog(options, (filename) => {
            return filename ? resolve(filename) : reject('showSaveDialog: no file set')
        })
    })

    dialog.showSaveDialog()
}

const showErrorBox = (title, content) => {
    dialog.showErrorBox(title, content)
}


// read a file collection ; and return a simple collection of products
const onReadFileCatalogStorage = (filename) => {
    return readFile(filename).then((content) => JSON.parse(content))
}

// save the collection
const onSaveFileCatalogStorage = (filename) => {
    let content = getCatalogStorageForSaving() // construct catalogue
    content = JSON.stringify(content) // TODO check if var not overitted ; passby référence

    return writeFile(filename, content)
}






// client api

receive('online-status-changed', (reply, reject, {status}) => {
    logger('event:online-status-changed: '+status)

    onlineStatusWindow = status
})

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

    let [errorRead, content] = await to(onReadFileCatalogStorage(filename))

    if(errorRead){
        showErrorBox('Cannot open file', errorRead.message || errorRead)

        return logger(errorRead)
    }

    // set the catalog content

    setCatalogStorageFrom(content)

    // get the current collection, as an array [id, {product}]

    const collectionMap = getCatalogStorageCollection()

    const collection = getProductsSimpleFrom(collectionMap)


    // set the collection from the read file

    setCatalogStorageFilename(filename)

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



receive('product', (reply, reject, {index}) => {

    // return a single product from the collection
    let product = getCatalogStorageProduct(index)

    if( product ) {
        return reply({index, product})
    }

    // TODO : if multiple not found product : multiple events on IPC renderer

    return showErrorBox('Cannot get product', '')
})











console.log('Running...')

console.log(
    'electron', process.versions.electron,
    'node', process.versions.node,
    'chrome', process.versions.chrome
)
