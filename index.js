const package = require('./package.json')

// Electron flow

const {app, BrowserWindow, ipcMain, dialog} = require('electron')

const platform = process.platform

const path = require('path')
const fs = require('fs')
const {format: urlformat} = require('url')

// Application fow

const {registerMoviesapiProtocol} = require('./moviesapi-protocol')


// User flow

const userDataPath = app.getPath('userData')
const userSettingsFilename = path.join(userDataPath, 'settings.json');

const debug = process.argv.includes('--debug')

// app.disableHardwareAcceleration()

let win
let onlineStatusWindow


// event logger for simple messages
const logger = (...messages) => {
    messages.forEach((message) => {
        console.log('\x1b[36m%s\x1b[0m', '[logger]', message.message || message)
    })
}



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
        icon: path.join(__dirname, 'app-icon.png'),
        width,
        height,
        minWidth: width,
        minHeight: height,
        show: false, // wait when ready : prevents white flickering
        // backgroundColor: '#fff',

        webPreferences: {
            //nodeIntegration: false, // todo wait for module in chrome 61
            //contextIsolation: true,
            preload: path.join(__dirname, 'app/preload.js'),
        },

        // borderless frame
        frame: false,
        transparent: true,
        titleBarStyle: 'hidden', // macos
    })

    win.setMenu(null) // no menu

    if( win.setSheetOffset ) {
        win.setSheetOffset(50) // +titlebar height on mac
    }

    win.loadURL(urlformat({
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
    registerMoviesapiProtocol() // register movies api custom protocol

    setTimeout(createWindow, 100) // create brower win + workaround for linux transparency
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

process.on('uncaughtException', (error) => logger('uncaughtException', error))

process.on('unhandledRejection', (error) => logger('unhandledRejection', error))















//// Application events flow


let catalogStorage = {} // content of the collection, options & others stuffs

let catalogStorageFilename // current opened file


// get an event fron the renderer
function receive( channel, listener ) {
    ipcMain.on(channel, listener)
}

// send a message to the renderer
function send( channel, args ) {
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
    let defaults = {
        version: 1,
        options: {},
        collection: []
    }

    catalogStorage = Object.assign({}, defaults, content) // shallow merge
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

    dialog.showSaveDialog()
}

const showErrorBox = (title, content) => {
    dialog.showErrorBox(title, content)
}


// read a file collection ; and return a simple collection of products
const onReadFileCatalogStorage = (filename, successHandler, errorhandler) => {
    return readFile(filename)
    .then((content) => JSON.parse(content))
    .then((content) => setCatalogStorageFrom(content)) // cosntruct catalogue
    .then((response) => getCatalogStorageCollection()) // get parsed catalogue
    .then((collection) => getProductsSimpleFrom(collection)) // get simple previews
    .then((products) => successHandler(products))
    .catch((error) => errorhandler(error))
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

receive('online-status-changed', (event, status) => {
    logger('event:online-status-changed: '+status)

    onlineStatusWindow = status
})

receive('application-close', (event) => {
    win.close()
})

receive('application-minimize', (event) => {
    win.minimize()
})

receive('application-maximize', (event) => {
    if( win.isMaximized() ) {
        win.unmaximize()
    } else {
        win.maximize()
    }
})


receive('open-collection-dialog', (event) => {
    const sender = event.sender
    
    showOpenDialog({
        properties: ['openFile'],
        filters: [
            {name: 'Movie Collection', extensions: ['json']}
        ]
    })
    .then((filePaths) => {
        let filename = filePaths[0] // get the single first

        setCatalogStorageFilename(filename)


        const onOpenError = (message) => {

            //reinit only the filename! in case a collection is already opened
            revertCatalogStorageFilename()

            showErrorBox('Cannot open file', message)
        }

        let promise = onReadFileCatalogStorage(filename, (products) => {
            return sender.send('get-collection', products)
        },
        (error) => {
            logger(error)
            onOpenError(error.message || error)
        })
    })
    .catch((error) => logger('showOpenDialog: no file set'))
})

receive('save-collection-dialog', (event) => {
    const sender = event.sender

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
            sender.send('notification', 'Fichier sauvegardé!')
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


receive('get-product', (event, productIndex) => {
    const sender = event.sender

    // return a single product from the collection
    let product = getCatalogStorageProduct(productIndex)

    if( product ) {
        sender.send('get-product', productIndex, product)

        return
    }

    showErrorBox('Cannot get product', '')
})












console.log('Running...')
