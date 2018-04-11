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

    onProductClick: ({e, index}) => (state, actions) => {
        console.log('onProductClick')
        console.log(index)

        //return fetch('moviesapi://tmdb-fr/movie/78')
        return fetch('moviesapi://tmdb-fr/search/blade runner')
        .then(response => response.json())
        .then(actions.setQuotes);
    },

    setQuotes: (response) => (state, actions) => {
        console.log('response')
        console.log(response)

        // state.products.push({
        //     title: response.title
        // })

        return {
            products: response // todo test
        }
    },

    onSearch: ({e, keyword, keyCode}) => (state, actions) => {
        console.log('onSearch')

        let products = state.products


        if( keyCode == 'Escape' ) {
            
            // show all products







            return;
        }

        // hide all products based on keyword

        products.forEach((product, index) => {
            const title = product.title

            products[index].hidden = title.indexOf(keyword) < 0
        })

        return {products}
    },
    
    onProductFavorite: ({e, index}) => (state, actions) => {

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
                onProductClick={actions.onProductClick}
                onProductFavorite={actions.onProductFavorite}
            />
        </app-sidebar>
)



hyperapp(state, actions, view, document.body)
  
