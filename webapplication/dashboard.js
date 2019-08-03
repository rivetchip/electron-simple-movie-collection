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
import {map, filter} from './helpers'

// app other utilities
import {fetchmovie} from './moviesapi-protocol'

// platform specifics javascript bridges & interfaces
import {DeviceBridge} from './platform-specific'

let $bridge = DeviceBridge(window.WebkitgtkInterface || window.AndroidInterface || window.ElectronInterface);

// disable eval
window.eval = global.eval = () => {throw 'no eval'}



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

console.log($bridge)

function identity(v) {
    return v
}




const state = { // initial state

    isLoading: false,
    isHamburgerOpen: false,
    // if fullscreen, remove margin around fake window
    isFullscreen: ['mobile', 'desktop'].includes($bridge.platform),
    // if mobile : smaller interface
    isMobile: ['mobile'].includes($bridge.platform),
    // remove header bar controls if we already have a real window
    appHeaderTitle: 'Movie Collection',
    appHeaderBar: ['desktop-window'].includes($bridge.platform), //todo: remove

    // current interface : welcome preview publication
    location: 'welcome',

    providerHash: 1, // french
    providers: [
        {name: 'TMDb', identifier: 'tmdb', lang: 'en'},
        {name: 'TMDb', identifier: 'tmdb', lang: 'fr'},
    ],

    version: 1,
    metadata: {}, // version, dates, etc

    collection: {},
    activeCollection: [], // active movies on sidebar

    movieHash: null, // current select movie
    movie: null, // current movie values

    draftHash: null, // draft movie id / null if new
    draft: null, // curent edit movie
}



import {version, metadata, options, collection as movies} from './___.json'

state.collection = movies;
state.activeCollection = Object.keys(movies);
state.isMobile = true;
state.isFullscreen = true;





const actions = {

    // Application toolbar

    onToolbarHamburger: () => (state) => { // ham click
        return {isHamburgerOpen: !state.isHamburgerOpen}
    },

    onToolbarOpen: () => async (state, actions) => {
        try {
            let storage = JSON.parse(await $bridge.openCollection())
            actions.onReceiveCollection({storage})
        }
        catch(error) {
            console.log('openCollection<< '+error)
        }
    },

    onReceiveCollection: ({storage}) => {
        const {version, metadata, collection: movies} = storage

        // reinit view
        return {
            version, metadata,
            collection: movies,
            activeCollection: Object.keys(movies),
            location: 'welcome',
            movieHash: null,
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

        try {
            await $bridge.saveCollection(JSON.stringify(storage))
        }
        catch(error) {
            console.log('saveCollection >> '+error)
        }
    },

    onToolbarNew: () => {
        return {movieHash: null, draftHash: null, draft: null, location: 'publication'}
    },

    // radio provider change
    onProviderChange: ({providerHash}) => {
        return {providerHash}
    },




    openPanelPreview: ({movieHash, movie}) => (state, actions) => {
        console.log('openPanelPreview', movieHash)

        return {location: 'preview', movieHash, movie, isHamburgerOpen: false}
    },

    // set the selected ; then open the preview
    onSidebarClick: ({movieHash}) => async (state, actions) => {
        console.log('onSidebarClick', movieHash)

        let movie = Object.assign({}, state.collection[movieHash])

        try {
            movie.poster_url = await $bridge.getPoster(movie.poster)
        }
        catch(error) {
            console.log('onSidebarClick >> '+error)
        }

        return actions.openPanelPreview({movieHash, movie})
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
    
    onSidebarFavorite: ({movieHash}) => (state, actions) => {
        console.log('onProductFavorite', movieHash)

        // Replace item at index using native splice

        return {collection: map(state.collection, (movie, hash) => {
            return hash == movieHash ? {...movie, favorite: !movie.favorite} : movie
        })}
    },


    onFetchInformations: () => async (state, actions) => {
        let provider = state.providers[state.providerHash]
        let keyword = state.draft.originalTitle // on the panel header

        let infos = await fetchmovie({
            action: 'search',
            keyword: keyword,
            source: provider.identifier,
            lang: provider.lang
        })

        console.log(infos)
    },






};





const view = (state, actions) => (

    <app className={[
        'viewport',
        state.isMobile && 'is-mobile',
        state.isFullscreen && 'is-fullscreen',
        state.isHamburgerOpen && 'is-hamburger-open'
    ].filter(c => !!c).join(' ')}>

        {state.appHeaderBar && 
            <ComponentAppTitlebar
                title={state.appHeaderTitle}
                onClose={actions.onAppClose}
                onMinimize={actions.onAppMinimize}
                onMaximize={actions.onAppMaximize}
            />
        }

        <ComponentAppToolbar
            buttons={[
                {
                    className: 'hamburger',
                    name: 'Menu',
                    onclick: actions.onToolbarHamburger
                },
                {
                    className: 'open',
                    name: 'Ouvrir',
                    title: 'Ouvrir un fichier',
                    onclick: actions.onToolbarOpen
                },
                {
                    className: 'save',
                    name: 'Enregistrer',
                    title: 'Enregistrer la liste courante',
                    onclick: actions.onToolbarSave
                },
                {
                    className: 'new',
                    name: 'Ajouter un film',
                    title: 'Ajouter un film',
                    onclick: actions.onToolbarNew
                }
            ]}
            providers={state.providers}
            providerHash={state.providerHash}
            onProviderChange={actions.onProviderChange}
        />

        <app-layout>

            <app-sidebar>

                <ComponentSidebarSearch
                    onSearch={actions.onSearch}
                />
                <ComponentSidebarMovies
                    movieHash={state.movieHash}
                    collection={state.collection}
                    activeCollection={state.activeCollection}
                    onClick={actions.onSidebarClick}
                    onFavorite={actions.onSidebarFavorite}
                />

            </app-sidebar>

            <product-panel>
                {state.location == 'welcome' &&
                    <ComponentPanelWelcome />
                }

                {state.location == 'preview' &&
                    <ComponentPanelPreview
                        movieHash={state.movieHash}
                        movie={state.movie}
                    />
                }

                {state.location == 'publication' &&
                    <ComponentPanelPublication
                        draftHash={state.draftHash}
                        draft={state.draft}
                        onFetchInformations={actions.onFetchInformations}
                    />
                }
            </product-panel>

        </app-layout>

        <ComponentAppStatusbar
            status={Object.keys(state.collection).length + ' films'}
            filters={'empty'}
        />

    </app>
)


const app = hyperapp(state, actions, view, document.body)







//android, open collection


// bridge.openCollection(JSON.parse).then(r => {
//     // console.log(r+"xx")
//     app.onReceiveCollection(r)
// })
// .catch(error=> {
//     console.log('ERRR'+error)
// })










const updateOnlineStatus = (event) => {
    let status =  navigator.onLine ? 'online' : 'offline'

    let onlineStatusWindow = status ; //TODO
}

addEventListener('online', updateOnlineStatus)
addEventListener('offline', updateOnlineStatus)
