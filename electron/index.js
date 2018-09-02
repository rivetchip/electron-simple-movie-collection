// Electron flow
const {platform, env} = process

const {app, BrowserWindow} = require('electron')

const {join: pathjoin} = require('path')

// User flow
const userDataPath = app.getPath('userData')
const userSettingsFilename = pathjoin(userDataPath, 'settings.json');

const debug = env.NODE_ENV == 'development'


if(platform == 'linux') {
    //--enable-transparent-visuals --disable-gpu
    app.disableHardwareAcceleration() // transparency and fire win.ready-show
}

// Application flow
let webView

function createWindow() {

    let width = 1100
    let height = 800

    webView = new BrowserWindow({
        icon: pathjoin(__dirname, 'app-icon.png'),
        width,
        height,
        minWidth: width,
        minHeight: height,
        show: false, // wait when ready : prevents white flickering
        //backgroundColor: '#fff',

        webPreferences: {
            nodeIntegration: false,
            webgl: false,
            webaudio: false,
            plugins: false,
            navigateOnDragDrop: false, // whether drag/drop linnk/file onto causes a navigation
            // contextIsolation: true,
            preload: pathjoin(__dirname, 'preload.js')
        },

        // borderless frame
        resizable: true,
        frame: debug ? true : false,
        transparent: true,
        titleBarStyle: 'hidden', // macos
    })

    if(!debug){
        webView.setMenu(null) // no menu
    }

    if(webView.setSheetOffset) {
        webView.setSheetOffset(50) // +titlebar height on mac
    }

    webView.loadFile(pathjoin(__dirname, 'www/index.html'))

    // Launch fullscreen with DevTools open
    if(debug) {
        webView.webContents.openDevTools({mode: 'bottom'})
    }

    // Show window when page is ready
    webView.once('ready-to-show', () => {
        webView.show()
        webView.focus()
    })

    webView.on('closed', () => {
        webView = null
    })

    webView.on('maximize', () => {
        // send('fullscreen-status-changed', true)
    })

    webView.on('unmaximize', () => {
        // send('fullscreen-status-changed', false)
    })
}

// create brower win + workaround for linux transparency TODO disable on future version
app.on('ready', () => setTimeout(createWindow, 100))

app.on('window-all-closed', () => {
    if(platform !== 'darwin') { // macos stay in dock
        app.quit()
    }
})

app.on('activate', () => { // mac
    if(!webView) {
        createWindow()
    }
})

process.on('uncaughtException', (error) => {
    console.log('uncaughtException', error)
})

process.on('unhandledRejection', (error) => {
    console.log('unhandledRejection', error)
})





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
// const receive = (channel, listener) => {

//     return ipcMain.on(channel, (event, message) => {

//         const sender = event.sender
//         const {responseChannel, args} = message // from the renderer

//         const reply = (args) => sender.send(responseChannel, {args}) // send back
//         const reject = (error) => sender.send(responseChannel, {error}) // send error back

//         return listener(reply, reject, args)
//     })
// }

// send a message to the renderer TODO remove
// const send = (channel, args) => {
//     return win.webContents.send(channel, args)
// }




console.log('Running...')

console.log(
    'electron', process.versions.electron,
    'node', process.versions.node,
    'chrome', process.versions.chrome
)
