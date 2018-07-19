/** @jsx h */
'use strict';

import { h, app as hyperapp } from './hyperapp'

const {appPlatform, appDevice} = window.process

// app components
import {ComponentAppTitlebar} from './components/app-titlebar'
import {ComponentAppToolbar} from './components/app-toolbar'
import {ComponentSidebarSearch, ComponentSidebarMovies} from './components/app-sidebar'
import {ComponentPanelWelcome, ComponentPanelPreview, ComponentPanelPublication} from './components/product-panels'
import {ComponentAppStatusbar} from './components/app-statusbar'

// platform specifics javascript bridges
import ElectronBridge from './platform-specific/electron-preload'
import AndroidBridge from './platform-specific/android-preload'

if(appPlatform == 'desktop' && appDevice == 'electron') {
    // const {onOpenCatalog, saveCatalog} = ElectronBridge
}
if(appPlatform == 'mobile' && appDevice == 'android') {
    // const {onOpenCatalog, saveCatalog} = AndroidBridge
}
console.log(window.process)
// helpers
import {lookup, map, filter, urlstringify} from './helpers'

// disable eval
window.eval = global.eval = () => {throw 'no eval'}


// console.log(window.process, process)




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

    movieIndex: null, // current select movie
    movie: null, // current movie values
    collection: {},
    sidebarCollection: {}, // active movies on the left

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

    onToolbarOpen: () => async (state, actions) => {
        return actions.onReceiveCollection(await ipc('open-collection-dialog'))
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

    onReceiveCollection: ({collection}) => {
        
        console.log(collection)

        const {version, metadata, options, collection: movies} = collection



        return {
            version, metadata, options,
            collection: movies,
            location: 'welcome',
            movieIndex: null,
            movie: null,
            sidebarCollection: movies
        }
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

        return {sidebarCollection: filter(state.collection, (movie, index) => {
            return contains(movie.title, keyword)
        })}
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

                <ComponentSidebarSearch
                    onSearch={actions.onSearch}
                />
                <ComponentSidebarMovies
                    movieIndex={state.movieIndex}
                    collection={state.sidebarCollection}
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