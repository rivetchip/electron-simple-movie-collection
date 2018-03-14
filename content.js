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




let collection



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


        // delegate events
        delegate(itemsList, 'product-item', 'click', eventOpenProductDisplay)



        // get the full collection from server
        eventServerReceive('get-collection', (event, products) => {
            // parse the content & and show the list to the column
            appendProductItemsList(itemsList, products)
        })

        // get a single, full product
        eventServerReceive('get-product', (event, productIndex, product) => {
            // parse and show the view panel
            openProductDisplay(panelContent, productIndex, product)
        })

  
    })();







    function appendProductItemsList( itemsList, products ) {
        products.forEach((product, index) => {
            let item = createProductItem(index, product)
            itemsList.appendChild(item)
        })
    }

    function eventOpenProductDisplay( event ) {
        event.preventDefault()

        const target = event.target
        const productIndex = target.getAttribute('data-index')

        eventServerSend('get-product', productIndex)
    }

    function openProductDisplay( panelContent, productIndex, product ) {
        const findField = (field) => panelContent.querySelector('[data-field="'+field+'"]')

        // set the product index :
        panelContent.setAttribute('data-index', productIndex)

        // set the descriptions to the fields :




        // TODO

    }

    function createProductItem( index, product ) {
        let item = h('product-item', {
            'data-index': index,
            'data-favorite': product.favorite
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






    // create element

    function h( tagName, props, children = [] ) {
        // document.createDocumentFragment()

        const isEvent = (attr) => attr.startsWith('on:')
        const getEventName = (attr) => attr.substring(3) // remove "on:"

        let element = document.createElement(tagName)

        if( typeof props === 'string' ) {
            element.appendChild(document.createTextNode(props))
        }
        else {
            Object.entries(props).forEach(([name, value]) => {

                switch( name ) {
                    case 'className':
                        return element.className = value

                    case 'html':
                        return element.innerHTML = value

                    case 'text':
                        return element.appendChild(document.createTextNode(value))
                }

                if( !isEvent(name) ) {
                    return element.setAttribute(name, value)
                }

                const eventName = getEventName(name)
                element.addEventListener(eventName, value)
            })
        }

        children.forEach((child) => {
            if( typeof child === 'string' ) {
                child = document.createTextNode(child)
            }

            element.appendChild(child)
        })

        children.forEach((child) => element.appendChild(child))
  
        return element
    }

    // delegate events

    function delegate( parent, target, eventType, callback ) {

        parent.addEventListener(eventType, function( event ) {
            var element = event.target;
            var matchesCallback = element.matches || element.matchesSelector;

            if( (matchesCallback).call(element, target) ) {
                callback.call(element, event);
            }
        });
    }

    function toggleState( element, one, two ) {
        element.setAttribute('data-state', element.getAttribute('data-state') === one ? two : one);
    }
    
    function getState( element ) {
        return element.getAttribute('data-state');
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



