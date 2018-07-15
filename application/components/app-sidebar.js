
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
    >
        <div class="title">{title}</div>

        {favorite && (
            <div class="favorite" onclick={event => onFavorite({index})}>
            {/* TODO */}
            </div>
        )}

    </product-item>
)

export const ComponentSidebarMovies = ({movieIndex, collection, onClick, onFavorite}) => (

    <product-items>

    {Object.entries(collection).map(([index, movie]) => (
        <ComponentSidebarMovie
            index={index}
            title={movie.title}
            selected={movieIndex == index}
            favorite={movie.favorite}
            onClick={onClick}
            onFavorite={onFavorite}
        />
    ))}

    </product-items>
)

