
import { h } from '../hyperapp'



export const ComponentAppTitlebar = ({title, onClose, onMinimize, onMaximize}) => (

    <app-titlebar>

        <app-controls>
            <button class="close" onclick={event => onClose()}></button>

            <button class="minimize" onclick={event => onMinimize()}></button>

            <button class="maximize" onclick={event => onMaximize()}></button>
        </app-controls>

        <div class="title">
            {title}
        </div>

    </app-titlebar>
)





