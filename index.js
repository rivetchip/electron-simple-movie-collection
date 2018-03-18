const package = require('./package.json')

const electron = require('electron')
const {app, BrowserWindow, ipcMain, dialog} = electron

const path = require('path')
const fs = require('fs')
const url = require('url')

const platform = process.platform
const appData = app.getPath('appData')

const debug = process.argv.indexOf('--debug') >= 0

// app.disableHardwareAcceleration()

let win
let onlineStatusWindow


// event logger for simple messages
const logger = (message) => {
    console.log('\x1b[36m%s\x1b[0m', '[logger]')
    console.log(message)
}

if( process.mas ) {
    app.setName(package.productName)
}

;(function initialize() {

    const shouldQuit = makeSingleInstance()

    if( shouldQuit ) {
        return app.quit()
    }

    function createWindow() {

        let width = 1100
        let height = 800

        win = new BrowserWindow({
            icon: path.join(__dirname, 'app-icon.png'),
            width,
            height,
            minWidth: width,
            minHeight: height,
            show: false, // wait ready ; prevents white flickering
            // backgroundColor: '#fff',

            // borderless frame
            frame: false,
            transparent: true,
            titleBarStyle: 'hiddenInset', // macos
        })

        win.loadURL(url.format({
            pathname: path.join(__dirname, 'app/index.html'),
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
    }

    app.on('ready', () => {
        setTimeout(createWindow, 100) // Workaround for linux transparency
    })

    app.on('window-all-closed', () => {
        if( process.platform !== 'darwin' ) { // macos stay in dock
            app.quit()
        }
    })

    app.on('activate', () => {
        if( !win ) {
            createWindow()
        }
    })

})()

function makeSingleInstance() {
    if( process.mas ) {
        return false // Mac app store
    }

    return app.makeSingleInstance(() => {
        if( win ) {
            if( win.isMinimized() ) {
                win.restore()
            }
            win.focus()
        }
    })
}

process.on('uncaughtException', (error) => logger(error))

process.on('unhandledRejection', (error) => logger(error))










let filename // current opened file
let catalogStorage = {} // content of the colelction & others stuffs


// get an event fron the renderer
function eventClientReceive( channel, listener ) {
    ipcMain.on(channel, listener)
}

// send a message to the renderer
function eventClientSend( channel, args ) {
    win.webContents.send(channel, args)
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



// empty the catalog storage
const emptyCatalogStorage = () => {
    catalogStorage = {}
}

// cosntruct the collection from the content of a file or anything
const setCatalogStorageFrom = (content) => {
    // parse content of file, & assigns defaults values
    let defaults = {
        version: 1,
        options: {},
        collection: []
    }

    catalogStorage = Object.assign({}, defaults, content) // shallow merge

    return catalogStorage.collection
}

// return the collection of all products
const getCatalogStorageCollection = () => {
    return catalogStorage.collection
}

// get a single product
const getCatalogStorageProduct = (productIndex) => {
    let product = catalogStorage.collection[productIndex]
    
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
        products.push({
            id: product.id,
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
            return filenames ? resolve(filenames) : reject() // no file
        })
    })
}

const showSaveDialog = (options) => {
    return new Promise((resolve, reject) => {
        dialog.showSaveDialog(options, (filename) => {
            return filename ? resolve(filename) : reject() // no file
        })
    })
}



// read a file collection ; and return a simple collection of products
const onReadFileCatalogStorage = (filename, successHandler, errorhandler) => {
    return readFile(filename)
    .then((content) => JSON.parse(content))
    .then((content) => setCatalogStorageFrom(content))
    .then((response) => getCatalogStorageCollection())
    .then((collection) => getProductsSimpleFrom(collection))
    .then((products) => successHandler(products))
    .catch((error) => errorhandler(error))
}

// save the collection
const onSaveFileCatalogStorage = (filename, successHandler, errorhandler) => {
    let content = getCatalogStorageForSaving()
    content = JSON.stringify(content)

    return writeFile(filename, content)
    .then((response) => successHandler())
    .catch((error) => errorhandler(error))
}






// client api

eventClientReceive('online-status-changed', (event, status) => {
    logger('event:online-status-changed: '+status)

    onlineStatusWindow = status
})


eventClientReceive('open-collection-dialog', (event) => {
    const sender = event.sender
    
    showOpenDialog({
        properties: ['openFile'],
        filters: [
            {name: 'Movie Collection', extensions: ['json']}
        ]
    })
    .then((filePaths) => {
        filename = filePaths[0] // get the single first

        const onOpenError = (message) => {

            //reinit only the filename! in case a collection is already opened
            filename = null

            return dialog.showErrorBox('Cannot open file', message)
        }

        let promise = onReadFileCatalogStorage(filename, (products) => {
            return sender.send('get-collection', products)
        },
        (error) => {
            logger(error)
            onOpenError(error.message ? error.message : error)
        })
    })
    .catch((error) => logger('showOpenDialog: no file set'))
})

eventClientReceive('save-collection-dialog', (event) => {
    const sender = event.sender

    const onSaveError = (error) => {
        //reinit files
        filename = null

        return dialog.showErrorBox('Cannot save file', error)
    }

    const onSaveCollection = (filename) => {
        // when validate save file

        let promise = onSaveFileCatalogStorage(filename, () => {
            // send notification
            eventClientSend('notification', 'Fichier sauvegardé!')
        },
        (error) => {
            logger(error)
            onOpenError(error.message ? error.message : error)
        })
    }

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
        .then((filePath) => {
            filename = filePath
            return onSaveCollection(filePath)
        })
        .catch((error) => logger('showSaveDialog: no file set'))
    }
})


eventClientReceive('get-product', (event, productIndex) => {
    const sender = event.sender

    // return a single product from the collection
    let product = getCatalogStorageProduct(productIndex)

    if( product ) {
        return sender.send('get-product', productIndex, product)
    }

    dialog.showErrorBox('Cannot get product', '')
})












console.log('Running...')
