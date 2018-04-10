/** @jsx h */
'use strict';

import { h, app as hyperapp } from './hyperapp'

// components
import {SearchToolbar, ProductItems} from './components/app-sidebar'

// console.log(<div />);

function xxxclick (e, index)  {
    // console.log('xxxclick')
    // console.log(e);
    // console.log(this);

    console.log(e, index)
}

function onSearch(e, keyword){
    console.log(e)
    console.log(keyword)
}



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
    up: function up(value) {
        return function (state) {
            return { count: state.count + value };
        };
    },

    favorite: (e, index) => (state) => {

        console.log(e)
        console.log(index)

        state.products.push({
            title: 'qsd'
        })

        return {
            products: state.products
        }
    }
};



const view = (state, actions) => (

    <app-layout>

        <app-sidebar>
            <SearchToolbar
                onSearch={onSearch}
            />
            <ProductItems
                products={state.products}
                onProductClick={xxxclick}
                onProductFavorite={actions.favorite}
            />
        </app-sidebar>
    




    </app-layout>
)



hyperapp(state, actions, view, document.body)
  
