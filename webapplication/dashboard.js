/** @jsx h */
'use strict';


import {h, render} from 'preact';

// app components
import * as Components from './components';


// import Dummyfile from '../simplemovie.ndjson'



const state = { // initial state
    isLoading: false,
    isMenuOpen: false,

};

const actions = {
    actionOpenMenu: function(event) {
        // console.log(state, event)
        // return {...state, isMenuOpen: !state.isMenuOpen};
    },

    // voir si on peut supprimer ...state afi nde ne pas copier à chaque-fois :/



};

function View(state) {
    return <app className={[
        'viewport',
        // state.isMobile && 'is-mobile',
        // state.isFullscreen && 'is-fullscreen',
        // state.isHamburgerOpen && 'is-hamburger-open'
    ].filter(c => !!c).join(' ')}>

        <Components.Header
            actionOpenMenu={actions.actionOpenMenu}
        ></Components.Header>

        <span>
            Menu: {String(state.isMenuOpen)}
        </span>

    </app>
}





const node = document.getElementById('app');
render(View(state), node, node.firstElementChild);







function updateOnlineStatus(event) {
    const status = navigator.onLine ? 'online' : 'offline';
}

window.addEventListener('online', updateOnlineStatus);
window.addEventListener('offline', updateOnlineStatus);



module.hot && module.hot.accept(function(error) {
    console.clear();
    error && console.error('>> Cannot apply HMR update', error);
});
