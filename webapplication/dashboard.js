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

    count: 0, stuff: []

};

const store = createStore(state);


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


const App1 = connect('count', actions)(({ count, increment, incrementBy }) => (
      <div>
        <p>Count: {count}</p>
        <button onClick={increment}>Increment</button>
        <button onClick={() => incrementBy(10)}>Increment by 10</button>
      </div>
    )
  )


// const node = document.getElementById('app');
// render(View(state, actions), node, node.firstElementChild);


export const getApp1 = () => (
    <Provider store={store}>
      <App1 />
    </Provider>
  )

const node = document.getElementById('app');
render(getApp1(), node, node.firstElementChild);





function updateOnlineStatus(event) {
    const status = navigator.onLine ? 'online' : 'offline';
}

window.addEventListener('online', updateOnlineStatus);
window.addEventListener('offline', updateOnlineStatus);



module.hot && module.hot.accept(function(error) {
    console.clear();
    error && console.error('>> Cannot apply HMR update', error);
});
