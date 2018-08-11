
import { h } from '../hyperapp'


export const ComponentPanelWelcome = () => (
    <div>Welcome!</div>
)


export const ComponentPanelPreview = ({ movieHash, movie }) => (

    <product-preview>

        <div class="video-panels">

            <div class="video-details">
                <div class="video-title">{movie.title}</div>
                <div class="video-panel-subtitle">{movie.dateReleased}</div>

                <div class="ratings-wrap">
                    <StarsRating name="rating" rating={movie.rating} disabled={true} />
                </div>

                <div class="video-attributes">
                    
                    {movie.director && 
                        <div class="video-attribute">
                            <label>Réalisateur:</label>
                            <div class="video-placeholder">{movie.director}</div>
                        </div>
                    }

                    {movie.duration > 0 && 
                        <div class="video-attribute">
                            <label>Durée:</label>
                            <div class="video-placeholder">{movie.duration} min.</div>
                        </div>
                    }

                    {(movie.countries && movie.countries.length > 0) && 
                        <div class="video-attribute">
                            <label>Nationalité:</label>
                            <div class="video-placeholder">{movie.countries.join(' / ')}</div>
                        </div>
                    }

                    {(movie.genres && movie.genres.length > 0) && 
                        <div class="video-attribute">
                            <label>Genres:</label>
                            <div class="video-placeholder">{movie.genres.join(' / ')}</div>
                        </div>
                    }
                    
                    {(movie.actors && movie.actors.length > 0) && 
                        <div class="video-attribute">
                            <label>Actors :</label>
                            <div class="video-placeholder">
                                {movie.actors.map(([actor, role]) => (
                                    <div>
                                        {actor} {role && '('+role+')'}
                                    </div>
                                ))}
                            </div>
                        </div>
                    }

                </div>

            </div>

            <img class="video-poster" src={movie.poster} />

            {/* {poster && 
                <div class="video-poster" style={{backgroundImage: 'url(' + poster + ')'}}></div>
            } */}

        </div>

        {movie.description && 
            <div class="video-overview">
                <h3 class="video-panel-title">Synopsis</h3>
                <div class="video-description">
                    {movie.description}
                </div>
            </div>
        }

    </product-preview>

)



export const ComponentPanelPublication = ({ draftHash, draft, onFetchInformation }) => (

    <product-publication>

        <div class="publication-header">
            <input type="text" class="publication-input" placeholder="Titre principal" value={draft.title} />
            
            <button
                class="publication-download"
                onclick={event => onFetchInformation()}
            >
                Fetch informations
            </button>
        </div>

        <div class="video-panels">

            <div class="video-details">
                <div class="video-title">{draft.title}</div>
                <div class="video-panel-subtitle">{draft.dateReleased}</div>

                <div class="ratings-wrap">
                    <StarsRating name="rating" rating={draft.rating} disabled={true} />
                </div>

                <div class="video-attributes">
                    
                    {draft.director && 
                        <div class="video-attribute">
                            <label>Réalisateur:</label>
                            <div class="video-placeholder">{director}</div>
                        </div>
                    }

                    {draft.duration > 0 && 
                        <div class="video-attribute">
                            <label>Durée:</label>
                            <div class="video-placeholder">{duration} min.</div>
                        </div>
                    }

                    {(draft.countries && countries.length > 0) && 
                        <div class="video-attribute">
                            <label>Nationalité:</label>
                            <div class="video-placeholder">{countries.join(' / ')}</div>
                        </div>
                    }

                    {(draft.genres && genres.length > 0) && 
                        <div class="video-attribute">
                            <label>Genres:</label>
                            <div class="video-placeholder">{genres.join(' / ')}</div>
                        </div>
                    }
                    
                    {(draft.actors && actors.length > 0) && 
                        <div class="video-attribute">
                            <label>Actors :</label>
                            <div class="video-placeholder">
                                {actors.map(([actor, role]) => (
                                    <div>
                                        {actor} {role && '('+role+')'}
                                    </div>
                                ))}
                            </div>
                        </div>
                    }

                </div>

            </div>

            <img class="video-poster" src={draft.poster} />

            {/* {poster && 
                <div class="video-poster" style={{backgroundImage: 'url(' + poster + ')'}}></div>
            } */}

        </div>

        <div class="video-overview">
            <h3 class="video-panel-title">Synopsis</h3>
            <div class="video-description">
                {draft.description}
            </div>
        </div>



    

    </product-publication>
)

export const StarsRating = ({ name, rating, count = 5, disabled = false }) => (

    <div class="stars-rating">

        <input
            id="rating-none"
            class="stars-rating-check"
            checked={!rating}
            value="0"
            name={name}
            type="radio"
            disabled={disabled}
        />
        <label title="Non-noté" aria-label="No rating" class="stars-rating-none" for="rating-none"></label>

        {Array.from({length: count}, (v, key) => [
            <input
                key={key}
                id={name+'-'+(key+1)}
                class="stars-rating-check"
                checked={rating == (key+1)}
                value={key}
                name={name}
                type="radio"
                disabled={disabled}
            />,
            <label for={name+'-'+(key+1)} aria-label={(key+1)+' star'} class="stars-rating-star"></label>
        ])}

    </div>

)

