
import { h, app as hyperapp } from '../hyperapp'



export const ProductPanelEmpty = () => (
    <div>vide</div>
)


export const ProductPanelPreview = ({ title, dateReleased, rating, poster, director, duration, countries, actors, description }) => (

    <product-preview>

        <div class="video-panels">

            <img data-field="poster" class="video-poster" src={poster} />

            <div class="video-details">
                <div data-field="title" class="video-placeholder video-title">{title}</div>
                <div data-field="dateReleased" class="video-panel-subtitle video-placeholder">{dateReleased}</div>

                <div class="ratings-wrap">
                    <span class="rating">5</span>
                    <div class="expanded-rating">
                        <div class="star-rating">
                            <span class="star on"></span>
                            <span class="star on"></span>
                            <span class="star on"></span>
                            <span class="star on"></span>
                            <span class="star on"></span>
                            <span class="star "></span>
                            <span class="star "></span>
                            <span class="star "></span>
                            <span class="star "></span>
                            <span class="star "></span>
                        </div>
                    </div>
                </div>

                <div class="video-attributes">
                    <div class="video-attribute video-director">
                        <label>Réalisateur:</label>
                        <div data-field="director" class="video-placeholder">{director}</div>
                    </div>
                    <div class="video-attribute video-duration">
                        <label>Durée:</label>
                        <div data-field="duration" class="video-placeholder">{duration}</div>
                    </div>
                    <div class="video-attribute video-countries">
                        <label>Nationalité:</label>
                        <div data-field="countries" class="video-placeholder">{countries}</div>
                    </div>
                    <div class="video-attribute video-genres">
                        <label>Genres:</label>
                        <div data-field="genres" class="video-placeholder">{duration}</div>
                    </div>
                    <div class="video-attribute video-actors">
                        <label>Actors :</label>
                        <div data-field="actors" class="video-placeholder">{actors}</div>
                    </div>
                </div>

            </div>

        </div>

        <div class="video-overview">
            <h3 class="video-panel-title">Synopsis</h3>
            <div class="video-description">
                {description}
            </div>
        </div>

    </product-preview>

)



export const ProductPanelPublication = ({  }) => (

    <div class="publicationx" />


)


//{[...Array(count)].map((i, x) => TODO with fragments

export const StarsRating = ({ name, rating, count = 5 }) => (

    <div class="stars-rating">
 
        <input
            id="rating-none"
            class="stars-rating-check"
            checked={!rating}
            value="0"
            name={name}
            type="radio"
        />
        <label title="Non-noté" aria-label="No rating" class="stars-rating-none" for="rating-none"></label>

        <input
            id={name+'-1'}
            class="stars-rating-check"
            checked={rating == 1}
            value={1}
            name={name}
            type="radio"
        />
        <label for={name+'-1'} aria-label={'1 star'} class="stars-rating-star"></label>

        <input
            id={name+'-2'}
            class="stars-rating-check"
            checked={rating == 2}
            value={2}
            name={name}
            type="radio"
        />
        <label for={name+'-2'} aria-label={'2 star'} class="stars-rating-star"></label>

        <input
            id={name+'-3'}
            class="stars-rating-check"
            checked={rating == 3}
            value={3}
            name={name}
            type="radio"
        />
        <label for={name+'-3'} aria-label={'3 star'} class="stars-rating-star"></label>

        <input
            id={name+'-4'}
            class="stars-rating-check"
            checked={rating == 4}
            value={4}
            name={name}
            type="radio"
        />
        <label for={name+'-4'} aria-label={'4 star'} class="stars-rating-star"></label>

        <input
            id={name+'-5'}
            class="stars-rating-check"
            checked={rating == 5}
            value={5}
            name={name}
            type="radio"
        />
        <label for={name+'-5'} aria-label={'5 star'} class="stars-rating-star"></label>

    </div>

)

