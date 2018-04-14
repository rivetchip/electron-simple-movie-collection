
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

export const ProductItems = ({productIndex, products, onProductClick, onProductFavorite}) => (

    <product-items>

    {products.map(({title, favorite, hidden}, index) => (
        <ProductItem
            index={index}
            title={title}
            selected={productIndex == index}
            favorite={favorite}
            hidden={hidden}
            onClick={onProductClick}
            onFavorite={onProductFavorite}
        />
    ))}

    </product-items>

)

export const ProductItem = ({ index, title, selected, favorite, hidden, onClick, onFavorite }) => (

    <product-item
        key={index}
        className={[selected && 'is-selected', hidden && 'is-hidden'].filter(c => !!c).join(' ')}
        onclick={event => onClick({event, index})}
    >
        <div class="title">{title}</div>

        {favorite && (
            <div class="favorite" onclick={event => onFavorite({event, index})}>
            </div>
        )}

    </product-item>
)

