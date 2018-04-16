
import { h, app as hyperapp } from '../hyperapp'




export const ProductPreview = ({ onSearch }) => (
    <div class="previex" />
)



export const ProductPublication = ({  }) => (

    <div class="publicationx" />


)

export const StarsRating = ({ name, rating, count = 5 }) => (

    <div class="stars-rating">
        <div>
            <input
                id="rating-none"
                class="stars-rating-check"
                checked={!rating}
                value="0"
                name={name}
                type="radio"
            />
            <label title="Non-noté" aria-label="No rating" class="stars-rating-none" for="rating-none"></label>
        </div>
        
        {[...Array(count)].map((i, x) =>
            <frag key={x}>
                <input
                    id={name+'-'+x}
                    class="stars-rating-check"
                    checked={rating == x}
                    value={x}
                    name={name}
                    type="radio"
                />
                <label for={name+'-'+x} aria-label={x+' star'} class="stars-rating-star"></label>
            </frag>
        )}

    </div>

)

