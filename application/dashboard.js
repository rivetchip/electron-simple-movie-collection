/** @jsx h */
'use strict';

//TODO cordova
// window.send = ( channel, args ) => {}
// window.receive = ( channel, listener ) => {}
// window.ipc = (channel, args) => {}



import { h, app as hyperapp } from './hyperapp'

// components
import {AppTitlebar} from './components/app-titlebar'
import {AppToolbar} from './components/app-toolbar'

import {SearchToolbar, ProductItems} from './components/app-sidebar'

import {ProductPanelEmpty, ProductPanelPreview, ProductPanelPublication} from './components/product-panels'

import {AppStatusbar} from './components/app-statusbar'



import {fetchmovie} from './moviesapi-protocol'



var x = fetchmovie({source:'tmdb', lang:'fr', action:'search', keyword:'blade runner'})

.then(r => {

    console.log(r)

})


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


function identity(v) {
    return v
}




const state = { // initial state
    isLoading: false,
    isFullscreen: false,
    isHamburgerOpen: false,
    isMobile: appPlatform == 'mobile',

    appTitle: 'Movie Collection',

    location: null, // current publication or preview mode

    providerIndex: 1, // french
    providers: [
        { name: 'TMDb', identifier: 'tmdb', lang: 'en' },
        { name: 'TMDb', identifier: 'tmdb', lang: 'fr' },
    ],

    productIndex: null, // current select product
    product: null, // current product values
    products: null,

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

    onToolbarNewProduct: () => {
        return {draftIndex: null, draft: {}, location: 'publication'}
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
            productIndex: null,
            product: null,
            products: collection
        }
    },


    showProductPreview: ({index, product}) => {
        console.log('showProductPreview', index)

        return {productIndex: index, product, isHamburgerOpen: false, location: 'preview'}
    },

    // set the selected ; then open the preview
    onProductClick: ({index}) => async ({productIndex}, {showProductPreview}) => {
        console.log('onProductClick', index)

        if(productIndex != index ) {
            showProductPreview(await ipc('product', {index})) // {index, product}
        }
    },

    // search event when using the search box on the sidebar

    onSearch: ({keyword, keyCode}) => ({products}, actions) => {
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





const view = ({appTitle, isMobile, isFullscreen, isHamburgerOpen, location, providerIndex, providers, productIndex, product, products}, actions) => {

    let productPanel

    if(location == 'preview') {
        productPanel = <ProductPanelPreview {...product} />
    }
    else if(location == 'publication') {
        productPanel = <ProductPanelPublication {...product} />
    }
    else {
        productPanel = <ProductPanelEmpty />
    }

    return (<app className={[
        'viewport',
        isMobile && 'is-mobile',
        isFullscreen || isMobile && 'is-fullscreen',
        isHamburgerOpen && 'is-hamburger-open'
    ].filter(c => !!c).join(' ')}>

        <AppTitlebar
            title={appTitle}
            onClose={actions.onAppClose}
            onMinimize={actions.onAppMinimize}
            onMaximize={actions.onAppMaximize}
        />

        <AppToolbar
            onHamburger={actions.onToolbarHamburger}
            onOpen={actions.onToolbarOpen}
            onSave={actions.onToolbarSave}
            onNewProduct={actions.onToolbarNewProduct}

            providerIndex={providerIndex}
            providers={providers}
            onProviderChange={actions.onProviderChange}
        />

        <app-layout>

            <app-sidebar>

                {/* TODO check if products to do onSearch() event */}

                <SearchToolbar
                    onSearch={actions.onSearch}
                />
                <ProductItems
                    productIndex={productIndex}
                    products={products}
                    onProductClick={actions.onProductClick}
                    onProductFavorite={actions.onProductFavorite}
                />

            </app-sidebar>

            <product-panel>
                {productPanel}
            </product-panel>

        </app-layout>

        <AppStatusbar
            productCount={(products && products.length) || 0}
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
    return ipc('online-status-changed', {status})
}

addEventListener('online', updateOnlineStatus)
addEventListener('offline', updateOnlineStatus)










//deviceready
document.addEventListener('DOMContentLoaded', () => {

})