'use strict';

const electron = require('electron')
const {remote, ipcRenderer} = electron

const win = remote.getCurrentWindow()



// IPC sending events

exports.send = function send( channel, args ) {
    ipcRenderer.send(channel, args)
}

exports.receive = function receive( channel, listener ) {
    ipcRenderer.on(channel, listener)
}

// create element

exports.h = function h( tagName, props, children = [] ) {
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
exports.delegate = function delegate( parent, selector, eventType, callback ) {

    parent.addEventListener(eventType, (event) => {
        let target = event.target;

        if( target.matches(selector) ) {
            callback(event)
        }
    })
}

exports.toggleState = function toggleState( element, one, two ) {
    element.setAttribute('data-state', element.getAttribute('data-state') === one ? two : one)
}

exports.getState = function getState( element ) {
    return element.getAttribute('data-state')
}




// create a temp snackbar

exports.createSnackbar = (() => {
    let duration = 3000
    let previous // previous snack

    const onAnimationendEvent = (event, elapsed) => {
        let target = event.target

        if( event.animationName == 'snackbar-fadeout' ) {
            // when the animation end, we remove self
            target.remove()

            if( previous === target ) {
                previous = null
            }
        }
    }

    return (viewport, message) => {
        if( previous ) {
            // dismiss
            previous.remove()
        }

        let snackbar = document.createElement('snackbar')
        snackbar.innerText = message
        snackbar.classList.add('is-visible')
        
        snackbar.addEventListener('animationend', onAnimationendEvent)
        snackbar.addEventListener('webkitAnimationEnd', onAnimationendEvent)

        let timeoutId = setTimeout(() => {
            // dismiss
            snackbar.classList.remove('is-visible');
        }, duration)

        previous = snackbar
        viewport.appendChild(snackbar)
    }
})()



console.log('dashboard.js')