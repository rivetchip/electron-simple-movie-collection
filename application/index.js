const package = require('../package.json')

// Electron flow

const {app, BrowserWindow, ipcMain, dialog, globalShortcut} = require('electron')

const platform = process.platform

const {join: pathjoin} = require('path')
const fs = require('fs')
const {format: urlformat} = require('url')

// Application fow

const {logger} = require('./logger')
const {registerMoviesapiProtocol} = require('./moviesapi-protocol')

// User flow

const userDataPath = app.getPath('userData')
const userSettingsFilename = pathjoin(userDataPath, 'settings.json');

const debug = process.argv.includes('--debug')

if( debug ) {
    const electronIpcLog = require('./electron-ipc-log')

    electronIpcLog((event) => {
        let {channel, data, sent, sync} = event
        let args = [sent ? '⬆️' : '⬇️', channel, ...data]
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

if( shouldStartInstance ) {
    app.quit()
}

function createWindow() {

    let width = 1100
    let height = 800

    win = new BrowserWindow({
        icon: pathjoin(__dirname, '..', 'app-icon.png'),
        width,
        height,
        minWidth: width,
        minHeight: height,
        show: false, // wait when ready : prevents white flickering
        // backgroundColor: '#fff',

        webPreferences: {
            nodeIntegration: false,
            // contextIsolation: true,
            preload: pathjoin(__dirname, 'browser-preload.js'),
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

    win.loadURL(urlformat({
        pathname: pathjoin(__dirname, '..', 'browser-bundle/index.html'),
        protocol: 'file:',
        slashes: true
    }))

    // Launch fullscreen with DevTools open, usage: npm run debug
    if( debug ) {
        win.webContents.openDevTools()
    }

    // Show window when page is ready
    win.on('ready-to-show', () => {
        win.show()
        win.focus()
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


    // TODO drag onto
    app.on('open-file', (event, filePath) => {
        console.log(filePath)
    })
}

app.on('ready', () => {
    // register movies api custom protocol
    registerMoviesapiProtocol()

    // debug mode, set a global shortcut to reload the app
    // debug && globalShortcut.register('Ctrl+W', () => {
    //     win.webContents.reloadIgnoringCache()
    //     win.focus()
    // })

    // create brower win + workaround for linux transparency
    setTimeout(createWindow, 100)
})

app.on('window-all-closed', () => {
    if( platform !== 'darwin' ) { // macos stay in dock
        app.quit()
    }
})

app.on('activate', () => {
    if( !win ) {
        createWindow()
    }
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

// reinit the catalogue storage filename
const revertCatalogStorageFilename = () => {
    catalogStorageFilename = null
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
const onSaveFileCatalogStorage = (filename, successHandler, errorhandler) => {
    let content = getCatalogStorageForSaving() // construct catalogue
    content = JSON.stringify(content) // TODO check if var not overitted ; passby référence

    return writeFile(filename, content)
    .then((response) => successHandler())
    .catch((error) => errorhandler(error))
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

    let [openError, filePaths] = await to(showOpenDialog({
        properties: ['openFile'],
        filters: [
            {name: 'Movie Collection', extensions: ['json']}
        ]
    }))
    // .catch((error) => {

    if(openError){

        //reinit only the filename! in case a collection is already opened
        revertCatalogStorageFilename()

        return logger(openError)
    }

    // then we try to read the current file

    let filename = filePaths[0] // get first file

    let [readError, storage] = await to(onReadFileCatalogStorage(filename))

    if(readError){
        showErrorBox('Cannot open file', readError.message || readError)

        return logger(readError)
    }

    // set the catalog storage

    setCatalogStorageFrom(storage)

    // get the current collection, as an array [id, {product}]

    const collectionMap = getCatalogStorageCollection()

    const collection = getProductsSimpleFrom(collectionMap)


    // set the collection from the read file

    setCatalogStorageFilename(filename)

    // then send it back to the client

    return reply({collection})
})

receive('save-collection-dialog', (reply) => { // FIXME

    console.log('TODO save-collection-dialog')

    return;


    const onSaveError = (error) => {
        //reinit files
        revertCatalogStorageFilename()

        showErrorBox('Cannot save file', error)
    }

    const onSaveCollection = (filename) => {
        // when validate save file

        // onSaveFileCatalogStorage(filename, {
        //     success(){},
        //     reject(error){}
        // })

        let promise = onSaveFileCatalogStorage(filename, () => {
            // send notification
            send('notification', 'Fichier sauvegardé!')
        },
        (error) => {
            logger(error)
            onOpenError(error.message || error)
        })
    }

    let filename = getCatalogStorageFilename()

    if( filename ) {
        onSaveCollection(filename)
    }
    else {
        // the file doesn't exist yet, we show the prompt

        showSaveDialog({
            properties: ['openFile'],
            filters: [
                {name: 'Movie Collection', extensions: ['json']}
            ]
        })
        .then((filename) => {
            setCatalogStorageFilename(filename)

            return onSaveCollection(filename)
        })
        .catch((error) => logger('showSaveDialog: no file set'))
    }
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
