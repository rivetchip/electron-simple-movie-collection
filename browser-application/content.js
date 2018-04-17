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







const state = { // initial state
    isLoading: false,
    isFullscreen: false,
    location: null, // current publicatoion or preview mode

    titlebar: {
        title: 'Movie Collection'
    },

    providerIndex: 1, // french
    providers: [
        { name: 'TMDb', identifier: 'tmdb', lang: 'en' },
        { name: 'TMDb', identifier: 'tmdb', lang: 'fr' },
    ],

    productIndex: null, // current select product
    products: new Map([
        [4, {title:'sqd', favorite: false}],
        [5, {title:'sqerd', favorite: true}],
        [6, {title:'sqd', favorite: false}],
        [7, {title:'sqerd', favorite: true}],
    ])
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
        return {isFullscreen: status}
    },

    // radio provider change
    onProviderChange: ({event, index}) => {
        return {providerIndex: index}
    },

    // empty the previous collection ; when the collection has been opened
    onReceiveCollection: ({products}) => {
        return {products: new Map(
            products.map((product, index) => [index, product]
        ))}
    },












    onProductClick: ({event, index}) => (state, actions) => {
        console.log('onProductClick', index)

        // set the selected ; then open the preview

        return {productIndex: index, location: 'preview'}
    },

    // search event when using the search box on the sidebar

    onSearch: ({event, keyword, keyCode}) => ({products}, actions) => {
        console.log('onSearch', keyword)

        // set to lower case in case of search accents and others
        keyword = keyword.toLowerCase()

        // if escape : show all products
        let showEverything = false

        if( keyCode == 'Escape' ) {
            showEverything = true
        }

        // hide all products based on keyword ; or if escape : show the all

        products.forEach((product, index) => {
            let {title} = product
            title = title.toLowerCase()

            product.hidden = showEverything ? false : title.indexOf(keyword) < 0

            products.set(index, product)
        })

        return {products}
    },
    
    onProductFavorite: ({event, index}) => ({products}, actions) => {
        console.log('onProductFavorite', index)



        // products.push({ // TODO
        //     title: 'qsd'
        // })

        return {products}
    },
};





const view = ({isFullscreen, titlebar, location, providerIndex, providers, productIndex, products}, actions) => {

    let productPanel
    const product = products.get(productIndex) // current product

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
            {...titlebar}
            {...actions.titlebar}
        />

        <AppToolbar
            providerIndex={providerIndex}
            providers={providers}
            onProviderChange={actions.onProviderChange}
            events={actions.toolbar}
            // TODO BETTER
        />

        <app-layout>

            <app-sidebar>

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
            productCount={products.size}
        />

    </app>)
}



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