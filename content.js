'use strict';

addEventListener('load', () => {

    const electron = require('electron');  
    const {remote, ipcRenderer} = electron;

    const win = remote.getCurrentWindow();


    (function _titlebar(){

        let titlebar = document.querySelector('app-titlebar')

        let minimizeButton =  titlebar.querySelector('.minimize')
        minimizeButton.onclick = ( event ) => {
            win.minimize()
        }

        let maximizeButton =  titlebar.querySelector('.maximize')
        maximizeButton.onclick = ( event ) => {
            win.isMaximized() ? win.unmaximize() : win.maximize()
        }

        let closeButton =  titlebar.querySelector('.close')
        closeButton.onclick = ( event ) => {
            win.close()
        }

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



