
import { h } from '../hyperapp'



export const ComponentSidebarSearch = ({onSearch}) => (

    <search-toolbar>
        <input
            type="search"
            className="search-input"
            placeholder="Recherche"
            oninput={event => onSearch({keyword: event.target.value})}
        />
    </search-toolbar>

)

 const ComponentSidebarMovie = ({ index, title, selected, favorite, onClick, onFavorite }) => (

    <product-item
        key={index}
        className={[
            selected && 'is-selected'
        ].filter(c => !!c).join(' ')}

        onclick={event => onClick({index})}
        ondblclick={event => onFavorite({index})}
    >
        {title}

        {favorite && <div class="favorite"></div>}

        {/* TODO */}

    </product-item>
)

export const ComponentSidebarMovies = ({movieIndex, activeCollection, collection, onClick, onFavorite}) => (

    <product-items>

    {activeCollection.map((index) => (
        <ComponentSidebarMovie
            index={index}
            selected={movieIndex == index}
            title={collection[index].title}
            favorite={collection[index].favorite}
            onClick={onClick}
            onFavorite={onFavorite}
        />
    ))}

    </product-items>
)

