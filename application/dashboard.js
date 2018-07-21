/** @jsx h */
'use strict';

import { h, app as hyperapp } from './hyperapp'

// app components
import {ComponentAppTitlebar} from './components/app-titlebar'
import {ComponentAppToolbar} from './components/app-toolbar'
import {ComponentSidebarSearch, ComponentSidebarMovies} from './components/app-sidebar'
import {ComponentPanelWelcome, ComponentPanelPreview, ComponentPanelPublication} from './components/product-panels'
import {ComponentAppStatusbar} from './components/app-statusbar'

// helpers
import {lookup, map, filter, urlstringify} from './helpers'

// platform specifics javascript bridges & interfaces
let bridge

import {ElectronBridge, AndroidBridge} from './platform-specific'

if('ElectronInterface' in window) {
    bridge = ElectronBridge(window.ElectronInterface)
}
if('AndroidInterface' in window) {
    bridge = AndroidBridge(window.AndroidInterface)
}

const {appPlatform} = bridge


// disable eval
window.eval = global.eval = () => {throw 'no eval'}






// import {fetchmovie} from './moviesapi-protocol'


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
    isHamburgerOpen: false,
    isFullscreen: appPlatform == 'mobile',
    isMobile: appPlatform == 'mobile',

    appTitle: 'Movie Collection',

    location: 'welcome', // preview publication ; current publication or preview mode

    providerIndex: 1, // french
    providers: [
        { name: 'TMDb', identifier: 'tmdb', lang: 'en' },
        { name: 'TMDb', identifier: 'tmdb', lang: 'fr' },
    ],

    metadata: {}, // version, dates, etc

    movieIndex: null, // current select movie
    movie: null, // current movie values
    collection: {},
    activeCollection: [], // active movies on sidebar

    draftIndex: null, // draft movie index / null if new
    draft: null, // curent edit movie
}



// import {version, metadata, options, collection as movies} from './___.json'

// state.collection = movies;
// state.sidebarCollection = movies;
// state.isMobile = true;





var actions = {

    // Application titlebar

    onAppClose: () => {
        return bridge.applicationClose()
    },
    onAppMinimize: () => {
        return bridge.applicationMinimize()
    },
    onAppMaximize: () => {
        return bridge.applicationMaximize()
    },

    // when user click fullscreen on the main app
    onAppFullscreen: ({status}) => {
        return {isFullscreen: status}
    },


    // Application toolbar

    onToolbarHamburger: () => (state) => { // ham click
        return {isHamburgerOpen: !state.isHamburgerOpen}
    },

    onToolbarOpen: () => async (state, actions) => {
        
        bridge.openCollection().then((storage) => {
            actions.onReceiveCollection(storage)
        })
        .catch((error) => {
            console.log('openCollection',error)
        })
    },

    onReceiveCollection: (storage) => {
        const {version, metadata, collection: movies} = storage

        return {
            version, metadata,
            collection: movies,
            activeCollection: Object.keys(movies),
            location: 'welcome',
            movieIndex: null,
            movie: null,
        }
    },

    onToolbarSave: () => async (state, actions) => {

        const defaults = {
            version: 1,
            metadata: {},
            collection: {}
        }

        const storage = Object.assign({}, defaults, {  // shallow merge
            version: state.version,
            metadata: state.metadata,
            collection: state.collection
        })

        bridge.saveCollection(storage).catch((error) => {
            console.log('saveCollection',error)
        })
    },

    onToolbarNew: () => {
        return {draftIndex: null, draft: null, location: 'publication'}
    },

    // radio provider change
    onProviderChange: ({index}) => {
        return {providerIndex: index}
    },




    openPanelPreview: ({index}) => (state, actions) => {
        console.log('openPanelPreview', index)

        let movie = state.collection[index]

        if(movie) {// TODO better
            return {movieIndex: index, movie, isHamburgerOpen: false, location: 'preview'}
        }
    },

    // set the selected ; then open the preview
    onSidebarClick: ({index}) => async (state, actions) => {
        console.log('onSidebarClick', index)

        return actions.openPanelPreview({index})
    },

    // filter collection based on keyword search
    // TODO is escape : contains("") aslways true
    onSearch: ({keyword}) => (state, actions) => {
        console.log('onSearch', keyword)

        let lowerCase = (text) => text.toLowerCase()

        let matchText = (text, keyword) => text.includes(keyword)

        let containsCurry = ({match, format}) => (text, keyword) => {
            return match(format(text), format(keyword))
        }

        let contains = containsCurry({match: matchText, format: lowerCase})

        return {activeCollection: Object.keys(
            filter(state.collection, (movie) => contains(movie.title, keyword))
        )}
    },
    
    onSidebarFavorite: ({index}) => ({products}, actions) => {
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
        state.isFullscreen && 'is-fullscreen',
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

                <ComponentSidebarSearch
                    onSearch={actions.onSearch}
                />
                <ComponentSidebarMovies
                    movieIndex={state.movieIndex}
                    collection={state.collection}
                    activeCollection={state.activeCollection}
                    onClick={actions.onSidebarClick}
                    onFavorite={actions.onSidebarFavorite}
                />

            </app-sidebar>

            <product-panel>
                { state.location == 'welcome' && <ComponentPanelWelcome
                    />
                }
                
                { state.location == 'preview' && <ComponentPanelPreview
                    movieIndex={state.movieIndex}
                    {...state.movie} />
                }

                { state.location == 'publication' && <ComponentPanelPublication
                    movieIndex={state.movieIndex}
                    {...state.movie} />
                }
            </product-panel>

        </app-layout>

        <ComponentAppStatusbar
            status={Object.keys(state.collection).length + ' films'}
            filters={'empty'}
        />

    </app>)
}


const app = hyperapp(state, actions, view, document.body)



// events

// receive('fullscreen-status-changed', (event, status) => {
//     return app.onAppFullscreen({status})
// })

// receive a notification from the main app
// receive('notification', (event, message) => {
//     //createSnackbar(viewport, message)
// })




const updateOnlineStatus = (event) => {
    let status =  navigator.onLine ? 'online' : 'offline'

    let onlineStatusWindow = status ; //TODO
}

addEventListener('online', updateOnlineStatus)
addEventListener('offline', updateOnlineStatus)










//deviceready
document.addEventListener('DOMContentLoaded', () => {

})