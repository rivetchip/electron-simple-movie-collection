var package = require('./package.json');

const electron = require('electron');  
const {app, BrowserWindow, ipcMain} = electron; 

const path = require('path');
const url = require('url');

const debug = false


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
            //win.maximize()
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





ipcMain.on('asynchronous-message', (event, arg) => {
    console.log(arg)  // affiche "ping"
    event.sender.send('asynchronous-reply', 'pong')
})
  


