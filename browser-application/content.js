/** @jsx h */
'use strict';

import { h, app as hyperapp } from './hyperapp'

// components
import {AppTitlebar} from './components/app-titlebar'
import {AppToolbar} from './components/app-toolbar'

import {SearchToolbar, ProductItems} from './components/app-sidebar'

import {ProductPanelEmpty, ProductPanelPreview, ProductPanelPublication} from './components/product-panels'

import {AppStatusbar} from './components/app-statusbar'

// console.log(process);



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

        return {productIndex: index, product, location: 'preview'}
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
        let showEverything = false

        if( keyCode == 'Escape' ) {
            showEverything = true
        }

        // hide all products based on keyword ; or if escape : show the all

        let lowerCase = (text) => text.toLowerCase()

        let matchText = (text, keyword) => text.includes(keyword)

        let mapHiddenOnTitle = ({format = lowerCase, match = matchText, showEverything}) => {
            return (product) => {
                let title = format(product.title) // in case of search accents and others

                product.hidden = showEverything ? false : !match(title, keyword)

                return product
            }
        }

        let hiddenOnTitle = mapHiddenOnTitle({format: lowerCase, match: matchText, showEverything})

        return {products: products.map(hiddenOnTitle)}
    },
    
    onProductFavorite: ({index}) => ({products}, actions) => {
        console.log('onProductFavorite', index)



        // products.push({ // TODO
        //     title: 'qsd'
        // })

        return {products}
    },
};





const view = ({appTitle, isFullscreen, location, providerIndex, providers, productIndex, product, products}, actions) => {

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

    return (<app className={['viewport', isFullscreen && 'is-fullscreen'].filter(c => !!c).join(' ')}>

        <AppTitlebar
            title={appTitle}
            onClose={actions.onAppClose}
            onMinimize={actions.onAppMinimize}
            onMaximize={actions.onAppMaximize}
        />

        <AppToolbar
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
            productCount={(products && products.size) || 0}
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
    return send('online-status-changed', navigator.onLine ? 'online' : 'offline')
}

addEventListener('online',  updateOnlineStatus)
addEventListener('offline', updateOnlineStatus)











document.addEventListener('DOMContentLoaded', () => {

})