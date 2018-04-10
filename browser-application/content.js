/** @jsx h */
'use strict';

import { h, app as hyperapp } from './hyperapp'

// components
import {SearchToolbar, ProductItems} from './components/app-sidebar'

// console.log(<div />);

const state = { // initial state
    count: 0,

    products: [
        {title:'sqd', favorite: false},
        {title:'sqerd', favorite: true},
        {title:'sqd', favorite: false},
        {title:'sqerd', favorite: true},
    ]
}

var actions = {
    up: function(value) {
        return function (state, actions) {
            return { count: state.count + value };
        };
    },

    xxxclick: ({e, index}) => (state, actions) => {
        console.log('xxxclick')
        console.log(e);
        console.log(index)
    },

    onSearch: ({e, keyword}) => (state, actions) => {
        console.log('onSearch')
        console.log(e)
        console.log(keyword)
    },
    
    favorite: ({e, index}) => (state, actions) => {

        console.log('favorite')
        console.log(e)
        console.log(index)
        // console.log(actions)

        state.products.push({
            title: 'qsd'
        })

        return {
            products: state.products
        }
    }
};



const view = (state, actions) => (

        <app-sidebar>
            <SearchToolbar
                onSearch={actions.onSearch}
            />
            <ProductItems
                products={state.products}
                onProductClick={actions.xxxclick}
                onProductFavorite={actions.favorite}
            />
        </app-sidebar>
)



hyperapp(state, actions, view, document.querySelector('app-layout'))
  
