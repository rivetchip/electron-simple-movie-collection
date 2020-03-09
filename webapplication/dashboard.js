/** @jsx h */
'use strict';

import {h, render} from 'preact';

// import createStore from 'unistore'
// import {Provider, connect} from 'unistore/preact'

import {createStore, Provider, connect} from './store'


// app components
import * as Components from './components';


// import Dummyfile from '../simplemovie.ndjson'



const state = { // initial state
    isLoading: false,
    isMenuOpen: false,

    count: 0, stuff: [],
    xx:1

};

const actions = store => ({
    actionOpenMenu: function(event1,event2) {
        console.log(event1,event2)
        // return {...state, isMenuOpen: !state.isMenuOpen};
    },

    // voir si on peut supprimer ...state afi nde ne pas copier à chaque-fois :/

    increment(state) {
        // The returned object will be merged into the current state
        return { count: state.count+1 }
      },
    
      // The above example as an Arrow Function:
      increment2: ({ count }) => ({ count: count+1 }),
    
      // Actions receive current state as first parameter and any other params next
      // See the "Increment by 10"-button below
      incrementBy: ({ count }, incrementAmount) => {
        return { count: count+incrementAmount }
      },

});

const store = createStore(state);


function View(state, actions) {
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





const App = connect(actions, function(state, actions, props) {
    return <div xx="xx">
    <p>Count: {JSON.stringify(state)}</p>
    <button onClick={actions.increment}>Increment</button>
    <button onClick={() => actions.incrementBy(10)}>Increment by 10</button>
    </div>;
});

function getApp() {
    return <Provider store={store}>
        <App />
    </Provider>;
}

const node = document.getElementById('app');
render(getApp(), node, node.firstElementChild);





function updateOnlineStatus(event) {
    const status = navigator.onLine ? 'online' : 'offline';
}

window.addEventListener('online', updateOnlineStatus);
window.addEventListener('offline', updateOnlineStatus);



module.hot && module.hot.accept(function(error) {
    console.clear();
    error && console.error('>> Cannot apply HMR update', error);
});
