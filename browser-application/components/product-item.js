

export const ProductItem = ({ index, title, toggle, favorite }) => (

    <product-item
        //class={done && "done"}
        onclick={e =>
            toggleFavorite({
                value: done,
                id: id
            })
        }
    >
        <div class="title">{title}</div>

        { favorite && (
            <div class="favorite"></div>
        )}

    </product-item>
)
