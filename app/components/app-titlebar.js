'use strict';

// require main app functions
const ownerDocument = document.currentScript.ownerDocument



class ComponentAppTitlebar extends HTMLElement {

    constructor() {
        super()

        this.state = 'idle'

        //this.attachShadow({mode: 'open'})
        // this.shadowRoot.appendChild(template.content.cloneNode(true))
    }

    connectedCallback() {

        if( !this.hasAttribute('title') ) {
            this.setAttribute('title', 'Movie Collection')
        }

        let minimizeButton = this.querySelector('.minimize')
        minimizeButton.addEventListener('click', (event) => this.onMinimizeEvent(event))


        let maximizeButton = this.querySelector('.maximize')
        maximizeButton.addEventListener('click', (event) => this.onMaximizeEvent(event))

        let closeButton = this.querySelector('.close')
        closeButton.addEventListener('click', (event) => this.onCloseEvent(event))
    }

    onCloseEvent( event ) {

        send('application-close')

        this.state = 'close'
    }

    onMinimizeEvent( event ) {

        send('application-minimize')

        this.state = 'minimize'
    }

    onMaximizeEvent( event ) {
    
        if( this.state == 'idle' ) {

            send('application-maximize')

            this.state = 'maximize'
        }
        else if( this.state == 'maximize' ) {

            send('application-unmaximize')

            this.state = 'idle'
        }
    }


    static get observedAttributes() {
        return ['title']
    }

    attributeChangedCallback( name, oldValue, newValue ) {

        if( name == 'title' ) {
            return this.onAttributeChangeTitle(newValue)
        }
    }

    onAttributeChangeTitle( value ) {

    }


}

exports.ComponentAppTitlebar = ComponentAppTitlebar