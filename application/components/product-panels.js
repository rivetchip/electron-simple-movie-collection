
import { h } from '../hyperapp'


export const ComponentPanelWelcome = () => (
    <div>Welcome!</div>
)


export const ComponentPanelPreview = ({ title, dateReleased, rating, poster, director, duration, countries, genres, actors, description }) => (

    <product-preview>

        <div class="video-panels">

            <div class="video-details">
                <div class="video-title">{title}</div>
                <div class="video-panel-subtitle">{dateReleased}</div>

                <div class="ratings-wrap">
                    <StarsRating name="rating" rating={rating} disabled={true} />
                </div>

                <div class="video-attributes">
                    
                    {director && 
                        <div class="video-attribute video-director">
                            <label>Réalisateur:</label>
                            <div class="video-placeholder">{director}</div>
                        </div>
                    }

                    {duration > 0 && 
                        <div class="video-attribute video-duration">
                            <label>Durée:</label>
                            <div class="video-placeholder">{duration} min.</div>
                        </div>
                    }

                    {(countries && countries.length > 0) && 
                        <div class="video-attribute video-countries">
                            <label>Nationalité:</label>
                            <div class="video-placeholder">{countries.join(' / ')}</div>
                        </div>
                    }

                    {(genres && genres.length > 0) && 
                        <div class="video-attribute video-genres">
                            <label>Genres:</label>
                            <div class="video-placeholder">{genres.join(' / ')}</div>
                        </div>
                    }
                    
                    {(actors && actors.length > 0) && 
                        <div class="video-attribute video-actors">
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

            <img class="video-poster" src={poster} />

        </div>

        {description && 
            <div class="video-overview">
                <h3 class="video-panel-title">Synopsis</h3>
                <div class="video-description">
                    {description}
                </div>
            </div>
        }

    </product-preview>

)



export const ComponentPanelPublication = ({  }) => (

    <div class="publicationx" />


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

