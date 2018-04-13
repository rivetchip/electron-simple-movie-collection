/** @jsx h */
'use strict';

import { h, app as hyperapp } from './hyperapp'

// components
import {AppTitlebar} from './components/app-titlebar'
import {AppToolbar} from './components/app-toolbar'

import {SearchToolbar, ProductItems} from './components/app-sidebar'

// console.log(<div />);





const state = { // initial state
    count: 123,
    fullscreen: false,

    titlebar: {
        title: 'Movie Collection'
    },

    provider: 0, // first
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



    up: function(value) {
        return function (state, actions) {
            return { count: state.count + value };
        };
    },

    // when user click fullscreen on the main app
    onFullscreen: ({status}) => (state, actions) => {
        return {fullscreen: status}
    },

    // when the collection has been opened
    onReceiveCollection: ({products}) => (state, actions) => {
        return {products}
    },













    onProductClick: ({e, index}) => (state, actions) => {
        console.log('onProductClick')
        console.log(index)

        //return fetch('moviesapi://tmdb-fr/movie/78')
        return fetch('moviesapi://tmdb-fr/search/blade runner')
        .then(response => response.json())
        .then(actions.setQuotes);
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

    onSearch: ({e, keyword, keyCode}) => (state, actions) => {
        console.log('onSearch')

        let products = state.products


        if( keyCode == 'Escape' ) {
            
            // show all products







            return;
        }

        // hide all products based on keyword

        products.forEach((product, index) => {
            const title = product.title

            products[index].hidden = title.indexOf(keyword) < 0
        })

        return {products}
    },
    
    onProductFavorite: ({e, index}) => (state, actions) => {

        console.log('favorite')
        console.log(e)
        console.log(index)
        // console.log(actions)

        state.products.push({
            title: 'qsd'
        })

        return {
            products: state.products
        }
    },
};






const view = (state, actions) => (

        <div>



        <AppTitlebar
            {...state.titlebar}
            events={actions.titlebar}
        />

        <AppToolbar
            providers={state.providers}
        />

        <app-sidebar
            className={state.fullscreen && "is-fullscreen"}
        >
            <SearchToolbar
                onSearch={actions.onSearch}
            />
            <ProductItems
                products={state.products}
                onProductClick={actions.onProductClick}
                onProductFavorite={actions.onProductFavorite}
            />
        </app-sidebar>


        </div>
)


const viewport = document.querySelector('app.viewport')

const app = hyperapp(state, actions, view, viewport)
  


// events

receive('fullscreen-status-changed', (event, status) => {
    return app.onFullscreen({status})
})

// receive a notification from the main app
receive('notification', (event, message) => {

})

// get the full collection from server
receive('get-collection', (event, products) => {

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