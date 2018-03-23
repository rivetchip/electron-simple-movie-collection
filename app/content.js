'use strict';

const electron = require('electron')
const {remote, ipcRenderer} = electron

const win = remote.getCurrentWindow()

const {createSnackbar} = require('./dashboard')


let winState = 'preview' // view or edit product


function send( channel, args ) {
    ipcRenderer.send(channel, args)
}

function receive( channel, listener ) {
    ipcRenderer.on(channel, listener)
}



;addEventListener('load', () => {

    const viewport = document.querySelector('.viewport')


    ;(function _titlebar(){

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

    })()

    ;(function _toolbar(){

        const toolbar = document.querySelector('app-toolbar')

        let openButton = toolbar.querySelector('.open')
        openButton.addEventListener('click', (event) => {
            send('open-collection-dialog')
        })

        let saveButton = toolbar.querySelector('.save')
        saveButton.addEventListener('click', (event) => {
            send('save-collection-dialog')
        })
    })()

    ;(function _frame(){

        const appFrame = document.querySelector('app-layout')
        const listItems = appFrame.querySelector('column-items')

        const searchPanel = document.querySelector('search-toolbar')
        const searchInput = searchPanel.querySelector('.search-input')

        const previewPanel = appFrame.querySelector('product-preview')
        const publicationPanel = appFrame.querySelector('product-publication')


        // delegate events ; on item click
        delegate(listItems, 'product-item', 'click', (event) => {
            eventOpenProductDisplay(previewPanel, event)
        })

        // event ; on search key up
        searchInput.addEventListener('keyup', (event) => {
            event.preventDefault()

            const target = event.target
            const keyword = target.value.toLowerCase()
            const keyCode = event.code

            // if( searchTimeout ) {
            //     clearTimeout(searchTimeout)
            // }

            // Loop through all rows, and hide those who don't match the search query
            let productItems = listItems.querySelectorAll('product-item')

            let hiddenClass = 'is-hidden'

            productItems.forEach((product, index) => {
                if( keyCode == 'Escape' ) {
                    // show all items
                    return product.classList.remove(hiddenClass)
                }

                // normal key press
                let title = product.querySelector('.title')
                title = title.innerText.toLowerCase()

                if( title.indexOf(keyword) >= 0 ) { // match
                    product.classList.remove(hiddenClass)
                } else {
                    product.classList.add(hiddenClass)
                }
            })
        })


        // get the full collection from server
        receive('get-collection', (event, products) => {
            // empty the current collection
            emptyProductListItems(listItems)

            // parse the content & and show the list to the column
            appendProductListItems(listItems, products)
        })

        // get a single, full product
        receive('get-product', (event, productIndex, product) => {
            // parse and show the view panel
            if( product ) {
                winState = 'preview'

                openProductDisplay(previewPanel, productIndex, product)
            }
        })

  
    })()


    // general

    receive('notification', (event, message) => {
        createSnackbar(viewport, message)
    })

    receive('fullscreen-status-changed', (event, status) => {
        if( status ) {
            return viewport.classList.add('is-fullscreen')
        }

        return viewport.classList.remove('is-fullscreen')
    })

    


    function emptyProductListItems(listItems){
        listItems.innerHTML = '' // fatest way ; find a better one ?
    }

    function appendProductListItems( listItems, products ) {
        products.forEach((product, index) => {
            let item = createProductItem(index, product)
            listItems.appendChild(item)
        })
    }


    let previousSelectedProduct

    function eventOpenProductDisplay( previewPanel, event ) {
        event.preventDefault()

        const target = event.target
        const productIndex = target.getAttribute('data-index')

        // first, we hide the panel content ( to open it later, if the product if found )
        previewPanel.classList.remove('is-visible')

        // remove old selected element, if any
        if( previousSelectedProduct ) {
            previousSelectedProduct.classList.remove('is-selected')
        }

        // set the selected now element
        target.classList.add('is-selected')

        // set the now selected
        previousSelectedProduct = target

        // get infos from server
        send('get-product', productIndex)
    }

    function openProductDisplay( previewPanel, productIndex, product ) {
        const findField = (field) => previewPanel.querySelector('[data-field="'+field+'"]')

        // set the panel open :
        previewPanel.classList.add('is-visible')

        // set the product index to the view :
        previewPanel.setAttribute('data-index', productIndex)

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
    function delegate( parent, selector, eventType, callback ) {

        parent.addEventListener(eventType, (event) => {
            let target = event.target;

            if( target.matches(selector) ) {
                callback(event)
            }
        })
    }

    function toggleState( element, one, two ) {
        element.setAttribute('data-state', element.getAttribute('data-state') === one ? two : one)
    }
    
    function getState( element ) {
        return element.getAttribute('data-state')
    }







})


const updateOnlineStatus = (event) => {
    send('online-status-changed', navigator.onLine ? 'online' : 'offline')
}

addEventListener('online',  updateOnlineStatus)
addEventListener('offline', updateOnlineStatus)
