
import { h } from '../hyperapp'


export const ComponentPanelWelcome = () => (
    <div>vide</div>
)


export const ComponentPanelPreview = ({ title, dateReleased, rating, poster, director, duration, countries, genres, actors, description }) => (

    <product-preview>

        <div class="video-panels">

            <div class="video-details">
                <div data-field="title" class="video-placeholder video-title">{title}</div>
                <div data-field="dateReleased" class="video-panel-subtitle video-placeholder">{dateReleased}</div>

                <div class="ratings-wrap">
                    <StarsRating name="rating" rating={rating} disabled={true} />
                </div>

                <div class="video-attributes">
                    <div class="video-attribute video-director">
                        <label>Réalisateur:</label>
                        <div data-field="director" class="video-placeholder">{director}</div>
                    </div>
                    <div class="video-attribute video-duration">
                        <label>Durée:</label>
                        <div data-field="duration" class="video-placeholder">{duration} min.</div>
                    </div>
                    <div class="video-attribute video-countries">
                        <label>Nationalité:</label>
                        <div data-field="countries" class="video-placeholder">{countries}</div>
                    </div>
                    <div class="video-attribute video-genres">
                        <label>Genres:</label>
                        <div data-field="genres" class="video-placeholder">{genres}</div>
                    </div>
                    <div class="video-attribute video-actors">
                        <label>Actors :</label>
                        <div data-field="actors" class="video-placeholder">{actors}</div>
                    </div>
                </div>

            </div>

            <img data-field="poster" class="video-poster" src={poster} />

        </div>

        <div class="video-overview">
            <h3 class="video-panel-title">Synopsis</h3>
            <div class="video-description">
                {description}
            </div>
        </div>

    </product-preview>

)



export const ComponentPanelPublication = ({  }) => (

    <div class="publicationx" />


)


//{[...Array(count)].map((i, x) => TODO with fragments

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

