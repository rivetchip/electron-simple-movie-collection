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

        const titlebar = document.querySelector('app-titlebar')

        let state = 'idle'

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

        const toolbar = document.querySelector('app-toolbar')

        let openButton = toolbar.querySelector('.open')
        openButton.addEventListener('click', (event) => {
            eventServerSend('open-collection-dialog')
        })
    })();

    (function _panel(){

        const panel = document.querySelector('app-panel')
        const itemsList = panel.querySelector('column-items')
        const panelContent = panel.querySelector('display-product')

        // get the full collection from server
        eventServerReceive('get-collection', (event, products) => {
            // parse the content & and show the list to the column
            appendProductItemsList(itemsList, products)
        })

  
        
    })();







    function appendProductItemsList( itemsList, products ) {
        products.forEach((product, index) => {
            let item = createProductItem(product)
            itemsList.appendChild(item)
        })
    }

    function createProductItem( product ) {
        let item = h('product-item', {
            'data-id': product.id,
            'data-favorite': product.favorite,
            'on:click': (event) => {
                console.log(event)
            }
        })

        let title = h('div', {
            className: 'title',
            text: product.title
        })

        item.appendChild(title)

        let favorite = h('div', {
            className: 'favorite'
        })

        item.appendChild(favorite)

        return item
    }







    function h( tagName, props, children = [] ) {
        // document.createDocumentFragment()

        const isEvent = (attr) => attr.startsWith('on:')
        const getEventName = (attr) => attr.substring(3) // remove "on:"

        let el = document.createElement(tagName)

        if( typeof props === 'string' ) {
            el.appendChild(document.createTextNode(props))
        }
        else {
            Object.entries(props).forEach(([name, value]) => {

                switch( name ) {
                    case 'className':
                        return el.className = value

                    case 'html':
                        return el.innerHTML = value

                    case 'text':
                        return el.appendChild(document.createTextNode(value))
                }

                if( !isEvent(name) ) {
                    return el.setAttribute(name, value)
                }

                const eventName = getEventName(name)
                el.addEventListener(eventName, value)
            })
        }

        children.forEach((child) => {
            if( typeof child === 'string' ) {
                child = document.createTextNode(child)
            }

            el.appendChild(child)
        })

        children.forEach((child) => el.appendChild(child))
  
        return el
    }




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



