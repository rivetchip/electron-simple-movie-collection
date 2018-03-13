'use strict';

const electron = require('electron');  
const {remote, ipcRenderer} = electron;

const win = remote.getCurrentWindow();


function eventServerSend( channel, args ) {
    ipcRenderer.send(channel, args)
}

function eventServerReceive( channel, listener ) {
    ipcRenderer.on(channel, listener)
}





addEventListener('load', () => {

    (function _titlebar(){
        let state = 'idle'

        let titlebar = document.querySelector('app-titlebar')

        let minimizeButton = titlebar.querySelector('.minimize')
        minimizeButton.addEventListener('click', (event) => {
            win.minimize()
            state = 'minimize'
        })

        let maximizeButton = titlebar.querySelector('.maximize')
        maximizeButton.addEventListener('click', (event) => {
            if( state == 'maximize' ) {
                win.unmaximize()
                state = 'idle'
            }
            else if( state == 'idle' ) {
                win.maximize()
                state = 'maximize'
            }
        })

        let closeButton = titlebar.querySelector('.close')
        closeButton.addEventListener('click', (event) => {
            win.close()
            state = 'close'
        })

    })();

    (function _toolbar(){

        let toolbar = document.querySelector('app-toolbar')

        let openButton = toolbar.querySelector('.open')
        openButton.addEventListener('click', (event) => {
            eventServerSend('open-file-dialog')
        })

        eventServerReceive('open-file-dialog', (event, filenames) => {
            const [filename] = filenames
        
            
        })
    })();












    const updateOnlineStatus = () => {
        ipcRenderer.send('online-status-changed', navigator.onLine ? 'online' : 'offline')
      }
    
      window.addEventListener('online',  updateOnlineStatus)
      window.addEventListener('offline',  updateOnlineStatus)



      ipcRenderer.on('asynchronous-reply', (event, arg) => {
        console.log(arg) // affiche "pong"
      })
      ipcRenderer.send('asynchronous-message', 'ping')



});



