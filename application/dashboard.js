/** @jsx h */
'use strict';

//TODO cordova
// window.send = ( channel, args ) => {}
// window.receive = ( channel, listener ) => {}
// window.ipc = (channel, args) => {}



import { h, app as hyperapp } from './hyperapp'

// components
import {ComponentAppTitlebar} from './components/app-titlebar'
import {ComponentAppToolbar} from './components/app-toolbar'

import {SearchToolbar, ProductItems} from './components/app-sidebar'

import {ProductPanelEmpty, ProductPanelPreview, ProductPanelPublication} from './components/product-panels'

import {AppStatusbar} from './components/app-statusbar'



import {fetchmovie} from './moviesapi-protocol'


/*
var x = fetchmovie({source:'tmdb', lang:'fr', action:'search', keyword:'blade runner'})
.then(r => {
    console.log(r)
})

var x = fetchmovie({source:'tmdb', lang:'en', action:'movie', keyword:78})
.then(r => {
    console.log(r)
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


function identity(v) {
    return v
}




const state = { // initial state
    isLoading: false,
    isFullscreen: false,
    isHamburgerOpen: false,
    isMobile: appPlatform == 'mobile',

    appTitle: 'Movie Collection',

    location: 'welcome', // preview publication ; current publication or preview mode

    providerIndex: 1, // french
    providers: [
        { name: 'TMDb', identifier: 'tmdb', lang: 'en' },
        { name: 'TMDb', identifier: 'tmdb', lang: 'fr' },
    ],

    movieIndex: null, // current select product
    movie: null, // current product values
    collection: null,

    draftIndex: null, // draft product index / null if new
    draft: null, // curent edit product
}

var actions = {

    // Application titlebar

    onAppClose: () => {
        ipc('application-close')
    },
    onAppMinimize: () => {
        ipc('application-minimize')
    },
    onAppMaximize: () => {
        ipc('application-maximize')
    },

    // when user click fullscreen on the main app
    onAppFullscreen: ({status}) => {
        return {isFullscreen: status}
    },


    // Application toolbar

    onToolbarHamburger: () => ({isHamburgerOpen}) => { // ham click
        return {isHamburgerOpen: !isHamburgerOpen}
    },

    onToolbarOpen: () => async (state, {onReceiveCollection}) => {
        onReceiveCollection(await ipc('open-collection-dialog')) // {collection[index, {product}]}
    },

    onToolbarSave: () => {
        ipc('save-collection-dialog')
    },

    onToolbarNew: () => {
        return {draftIndex: null, draft: null, location: 'publication'}
    },

    // radio provider change
    onProviderChange: ({index}) => {
        return {providerIndex: index}
    },

    // empty the previous collection ; when the collection has been opened
    onReceiveCollection: ({collection}) => {
        // we receive a new Maped array [id, {product}] collection of simple products
        return {
            location: null,
            movieIndex: null,
            movie: null,
            collection
        }
    },


    showProductPreview: ({index, movie}) => {
        console.log('showProductPreview', index)

        return {movieIndex: index, movie, isHamburgerOpen: false, location: 'preview'}
    },

    // set the selected ; then open the preview
    onProductClick: ({index}) => async ({movieIndex}, {showProductPreview}) => {
        console.log('onProductClick', index)




//TODO FIXME showProductPreview
        if(movieIndex != index ) {
            showProductPreview(await ipc('movie', {index})) // {index, product}
        }
    },

    // search event when using the search box on the sidebar

    onSearch: ({keyword, keyCode}) => ({collection}, actions) => {
        console.log('onSearch', keyword)

        // if escape : show all products
        let showEverything = keyCode == 'Escape'

        // hide all products based on keyword ; or if escape : show the all

        let lowerCase = (text) => text.toLowerCase()

        let matchText = (text, keyword) => text.includes(keyword)

        let mapHiddenOnTitle = ({format = lowerCase, match = matchText}) => (product) => {
            
            product.hidden = !match(format(product.title), keyword)
            
            return product
        }

        let mapShowOnTitle = () => (product) => {
            product.hidden = false;
            return product
        }

        let productMapFn = showEverything ? mapShowOnTitle() : mapHiddenOnTitle({format: lowerCase, match: matchText})


        return {products: products.map(productMapFn)}
    },
    
    onProductFavorite: ({index}) => ({products}, actions) => {
        console.log('onProductFavorite', index)



        // products.push({ // TODO
        //     title: 'qsd'
        // })

        return {products}
    },
};





const view = (state, actions) => {

    return (<app className={[
        'viewport',
        state.isMobile && 'is-mobile',
        state.isFullscreen || state.isMobile && 'is-fullscreen',
        state.isHamburgerOpen && 'is-hamburger-open'
    ].filter(c => !!c).join(' ')}>

        <ComponentAppTitlebar
            title={state.appTitle}
            onClose={actions.onAppClose}
            onMinimize={actions.onAppMinimize}
            onMaximize={actions.onAppMaximize}
        />

        <ComponentAppToolbar
            buttons={[
                {
                    name: 'Menu',
                    class: 'hamburger',
                    onclick: actions.onToolbarHamburger
                },
                {
                    class: 'open',
                    name: 'Ouvrir',
                    title: 'Ouvrir un fichier',
                    onclick: actions.onToolbarOpen
                    
                },
                {
                    class: 'save',
                    name: 'Enregistrer',
                    title: 'Enregistrer la liste courante',
                    onclick: actions.onToolbarSave
                },
                {
                    class: 'new',
                    name: 'Ajouter un film',
                    title: 'Ajouter un film',
                    onclick: actions.onToolbarNew,
                },
            ]}
            providers={state.providers}
            providerIndex={state.providerIndex}
            onProviderChange={actions.onProviderChange}
        />

        <app-layout>

            <app-sidebar>

                <SearchToolbar
                    onSearch={actions.onSearch}
                />
                <ProductItems
                    productIndex={state.productIndex}
                    products={state.products}
                    onProductClick={actions.onProductClick}
                    onProductFavorite={actions.onProductFavorite}
                />

            </app-sidebar>

            <product-panel>
                { state.location == 'welcome' && <ProductPanelEmpty /> }
                { state.location == 'preview' && <ProductPanelPreview {...state.product} /> }
                { state.location == 'publication' && <ProductPanelPublication {...state.product} /> }
            </product-panel>

        </app-layout>

        <AppStatusbar
            productCount={(state.products && state.products.length) || 0}
        />

    </app>)
}



const app = hyperapp(state, actions, view, document.body)



// events

receive('fullscreen-status-changed', (event, status) => {
    return app.onAppFullscreen({status})
})

// receive a notification from the main app
receive('notification', (event, message) => {
    //createSnackbar(viewport, message)
})




const updateOnlineStatus = (event) => {
    let status =  navigator.onLine ? 'online' : 'offline'

    let onlineStatusWindow = status ; //TODO
}

addEventListener('online', updateOnlineStatus)
addEventListener('offline', updateOnlineStatus)










//deviceready
document.addEventListener('DOMContentLoaded', () => {

})