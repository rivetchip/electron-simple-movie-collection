
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

 const ComponentSidebarMovie = ({ movieHash, title, selected, favorite, onClick, onFavorite }) => (

    <product-item
        key={movieHash}
        className={[
            selected && 'is-selected'
        ].filter(c => !!c).join(' ')}

        onclick={event => onClick({movieHash})}
        ondblclick={event => onFavorite({movieHash})}
    >
        {title}

        {favorite && <div class="favorite"></div>}

        {/* TODO */}

    </product-item>
)

export const ComponentSidebarMovies = ({movieHash, activeCollection, collection, onClick, onFavorite}) => (

    <product-items>

    {activeCollection.map((hash) => (
        <ComponentSidebarMovie
            movieHash={hash}
            selected={hash == movieHash}
            title={collection[hash].title}
            favorite={collection[hash].favorite}
            onClick={onClick}
            onFavorite={onFavorite}
        />
    ))}

    </product-items>
)

