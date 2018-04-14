/** @jsx h */
'use strict';

import { h, app as hyperapp } from './hyperapp'

// components
import {AppTitlebar} from './components/app-titlebar'
import {AppToolbar} from './components/app-toolbar'

import {SearchToolbar, ProductItems} from './components/app-sidebar'

// console.log(<div />);



/*
fetch('moviesapi://tmdb-fr/movie/78')
// fetch('moviesapi://tmdb-fr/search/blade runner')
.then(response => response.text())

.then(function(response) {
    console.log(response)
})
*/

// previewPanel index
// send('get-product', productIndex)

/*
if( previousSelectedProduct ) {
            previousSelectedProduct.classList.remove('is-selected')
        }
*/

//openProductDisplay







const state = { // initial state
    count: 123,
    fullscreen: false,
    location: 'idle', // current publicatoion or preview mode

    titlebar: {
        title: 'Movie Collection'
    },

    providerIndex: 1, // french
    providers: [
        { name: 'TMDb', identifier: 'tmdb', lang: 'en' },
        { name: 'TMDb', identifier: 'tmdb', lang: 'fr' },
    ],

    products: [
        {title:'sqd', favorite: false},
        {title:'sqerd', favorite: true},
        {title:'sqd', favorite: false},
        {title:'sqerd', favorite: true},
    ]
}

var actions = {

    titlebar: {

        onClose: ({event}) => {
            return send('application-close')
        },
    
        onMinimize: ({event}) => {
            return send('application-minimize')
        },
    
        onMaximize: ({event}) => {
            return send('application-maximize')
        }
    },

    toolbar: {

        // open file button
        onOpen: ({event}) => {
            return send('open-collection-dialog')
        },

        // save file button
        onSave: ({event}) => {
            return send('save-collection-dialog')
        },

        // create new product button
        onNew: ({event}) => {

        },
    },

    // when user click fullscreen on the main app
    onFullscreen: ({status}) => {
        return {fullscreen: status}
    },

    // radio provider change
    onProviderChange: ({event, index}) => {
        return {providerIndex: index}
    },

    // when the collection has been opened
    onReceiveCollection: ({products}) => {
        return {products}
    },













    onProductClick: ({event, index}) => (state, actions) => {
        console.log('onProductClick')
        // console.log(index)

        console.log(state.providerIndex)

        //return fetch('moviesapi://tmdb-fr/movie/78')
        // return fetch('moviesapi://tmdb-fr/search/blade runner')
        // .then(response => response.json())
        // .then(actions.setQuotes);
    },

    setQuotes: (response) => (state, actions) => {
        console.log('response')
        console.log(response)

        // state.products.push({
        //     title: response.title
        // })

        return {
            products: response // todo test
        }
    },

    // search event when using the search box on the sidebar

    onSearch: ({event, keyword, keyCode}) => ({products}, actions) => {

        // set to lower case in case of search accents and others

        keyword = keyword.toLowerCase()

        // if escape : show all products

        let showEverything = false

        if( keyCode == 'Escape' ) {
            showEverything = true
        }

        // hide all products based on keyword ; or if escape : show the all

        products.forEach(({title}, index) => {
            title = title.toLowerCase()

            products[index].hidden = showEverything ? false : title.indexOf(keyword) < 0
        })

        return {products}
    },
    
    onProductFavorite: ({event, index}) => ({products}, actions) => {

        console.log('favorite')
        console.log(event)
        console.log(index)
        // console.log(actions)

        products.push({ // TODO
            title: 'qsd'
        })

        return {products}
    },
};





const view = (state, actions) => (

    <app className={['viewport', state.fullscreen && 'is-fullscreen'].filter(e => !!e).join(' ')}>

        <AppTitlebar
            {...state.titlebar}
            events={actions.titlebar}
        />

        <AppToolbar
            providerIndex={state.providerIndex}
            providers={state.providers}
            onProviderChange={actions.onProviderChange}
            events={actions.toolbar}
        />

        <app-layout>

            <app-sidebar>

                <SearchToolbar
                    onSearch={actions.onSearch}
                />
                <ProductItems
                    products={state.products}
                    onProductClick={actions.onProductClick}
                    onProductFavorite={actions.onProductFavorite}
                />

            </app-sidebar>


            {/* EDITION */}





        </app-layout>


    </app>
)



const app = hyperapp(state, actions, view, document.body)



// events

receive('fullscreen-status-changed', (event, status) => {
    return app.onFullscreen({status})
})

// receive a notification from the main app
receive('notification', (event, message) => {
    //createSnackbar(viewport, message)
})

// get the full collection from server
receive('get-collection', (event, products) => {
    return app.onReceiveCollection({products})
})

// get a single, full product
receive('get-product', (event, index, product) => {
    // parse and show the view panel
    if( product ) {
        // winState = 'preview'

    }
})





const updateOnlineStatus = (event) => {
    return send('online-status-changed', navigator.onLine ? 'online' : 'offline')
}

addEventListener('online',  updateOnlineStatus)
addEventListener('offline', updateOnlineStatus)











document.addEventListener('DOMContentLoaded', () => {

})