
import { h, app as hyperapp } from '../hyperapp'



export const SearchToolbar = ({ onSearch }) => (

    <search-toolbar>
        <input
            onkeyup={event => onSearch({event, keyword: event.target.value, keyCode: event.code})}
            className="search-input"
            type="search"
            placeholder="Recherche"
        />
    </search-toolbar>

)

export const ProductItems = ({products, onProductClick, onProductFavorite}) => (

    <product-items>
    {
        products.map((product, index) => (
            <ProductItem
                index={index}
                title={product.title}
                favorite={product.favorite}
                hidden={product.hidden}
                onClick={onProductClick}
                onFavorite={onProductFavorite}
            />
        ))
    }
    </product-items>

)

export const ProductItem = ({ index, title, favorite, hidden = false, onClick, onFavorite }) => (

    <product-item
        key={index}
        className={hidden && "is-hidden"}
        onclick={event => onClick({event, index})}
    >
        <div class="title">{title}</div>

        { favorite && (
            <div
                class="favorite"
                onclick={event => onFavorite({event, index})}
            >
            </div>
        )}

    </product-item>
)

