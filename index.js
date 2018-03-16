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



if( process.mas ) {
    app.setName(package.productName)
}

;(function initialize() {

    const shouldQuit = makeSingleInstance()

    if( shouldQuit ) {
        return app.quit()
    }

    function createWindow() {

        let width = 1000
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










let filename // current opened file
let collection // current full collection
let options // user options of the collection


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

// cosntruct the collection from the content of a file
const constructCollectionFrom = (content) => {
    // parse content of file
    options = content.options
    collection = content.collection

    return collection
}

// construct file from the collection datas
const constructCollectionFileFrom = (collection, options) => {
    let content = {
        options,
        collection
    }

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



// read a file collection
const onReadFileCollection = (filename, successHandler, errorhandler) => {
    return readFile(filename)
    .then((content) => JSON.parse(content))
    .then((content) => constructCollectionFrom(content))
    .then((collection) => getProductsSimpleFrom(collection))
    .then((products) => successHandler(products))
    .catch((error) => errorhandler(error))
}

// save the collection

const onSaveFileCollection = (filename, successHandler, errorhandler) => {
    let content = constructCollectionFileFrom(collection, options)
    content = JSON.stringify(content)

    return writeFile(filename, content)
    .then(() => successHandler())
    .catch((error) => errorhandler(error))
}






// client api

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

        const onOpenError = (error) => {
            //reinit collections
            filename = null
            collection = null
            options = null

            return dialog.showErrorBox('Cannot open file', error)
        }

        let promise = onReadFileCollection(filename, (products) => {
            return sender.send('get-collection', products)
        },
        (error) => {
            onOpenError('error') // TODO
        })
    })
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

        let promise = onSaveFileCollection(filename, () => {
            // send notification
            eventClientSend('notification', 'Save ok')
        },
        (error) => {
            onSaveError('error') // TODO
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
    }
})


eventClientReceive('get-product', (event, productIndex) => {
    const sender = event.sender

    // return a single product from the collection
    let product = collection[productIndex]

    if( product ) {
        return sender.send('get-product', productIndex, product)
    }

    dialog.showErrorBox('Cannot get product', '')
})




eventClientReceive('online-status-changed', (event, status) => {
    console.log('online-status-changed: '+status)
})







console.log('Running...')
