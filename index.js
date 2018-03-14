var package = require('./package.json');

const electron = require('electron');  
const {app, BrowserWindow, ipcMain, dialog} = electron;

const path = require('path');
const url = require('url');

const platform = process.platform

const debug = process.argv.indexOf('--debug') >= 0

// app.disableHardwareAcceleration()

let win = null



if( process.mas ) {
    app.setName(package.productName);
}

(function initialize() {

    const shouldQuit = makeSingleInstance()

    if( shouldQuit ) {
        return app.quit()
    }

    function createWindow() {

        win = new BrowserWindow({
            width: 800,
            height: 600,
            minWidth: 800,
            minHeight: 600,
            show: false, // wait ready ; prevents white flickering
            // backgroundColor: '#fff',

            // borderless frame
            frame: false,
            transparent: true,
            titleBarStyle: 'hiddenInset', // macos
        })

        win.loadURL(url.format({
            pathname: path.join(__dirname, 'index.html'),
            protocol: 'file:',
            slashes: true
        }))

        // Launch fullscreen with DevTools open, usage: npm run debug
        if( debug ) {
            win.webContents.openDevTools()
        }

        // Show window when page is ready
        win.on('ready-to-show', () => {
            win.show();
        })

        win.on('closed', () => {
            win = null;
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

})();

function makeSingleInstance() {
    if( process.mas ) {
        return false; // Mac app store
    }

    return app.makeSingleInstance(() => {
        if( win ) {
            if( win.isMinimized() ) {
                win.restore();
            }
            win.focus()
        }
    })
}










let filename // current opened file
let collection // current full collection
let options // user options of the collection



function eventClientReceive( channel, listener ) {
    ipcMain.on(channel, listener)
}


// client api

eventClientReceive('open-collection-dialog', (event) => {
    const sender = event.sender
    
    dialog.showOpenDialog({
        properties: ['openFile'],
        filters: [
            {name: 'Movie Collection', extensions: ['json']}
        ]
    },
    (filenames) => {
        if (filenames) {
            [filename] = filenames

            let products = []

            try {
                const content = require(filename)

                options = content.options || {}
                collection = content.collection || []

                // return a simple collection of products
                collection.forEach((product) => {
                    products.push({
                        id: product.id,
                        title: product.title,
                        favorite: product.favorite,
                        // poster: product.poster
                    })
                })

                sender.send('get-collection', products)
            }
            catch( e ) {
                //reinit
                filename = null
                collection = null
                options = null

                return dialog.showErrorBox('Cannot open file', e.message)
            }
        } 
    })
})


eventClientReceive('get-product', (event, productIndex) => {
    const sender = event.sender

    // return a single product from the collection
    let product = collection[productIndex]

    if( product ) {
        return sender.send('get-product', productIndex, product)
    }

    return dialog.showErrorBox('Cannot get product', '')
})











console.log('Running...')
