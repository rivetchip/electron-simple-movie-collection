var package = require('./package.json');

const electron = require('electron');  
const {app, BrowserWindow, ipcMain, dialog} = electron;


const path = require('path');
const url = require('url');

const debug = process.argv.indexOf('--debug')

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

    app.on('ready', createWindow)

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




function eventClientSend( channel, args ) {
    ipcMain.send(channel, args)
}

function eventClientReceive( channel, listener ) {
    ipcMain.on(channel, listener)
}


eventClientReceive('open-file-dialog', (event) => {
    dialog.showOpenDialog({
        properties: ['openFile']
    },
    (files) => {
        if (files) {
            event.sender.send('open-file-dialog', files)
        } 
    })
  })



