
import { h, app as hyperapp } from '../hyperapp'



export const AppTitlebar = ({title, onClose, onMinimize, onMaximize}) => (

    <app-titlebar>

        <app-controls>
            <button class="close" onclick={event => onClose()}>
            </button>

            <button class="minimize" onclick={event => onMinimize()}>
            </button>

            <button class="maximize" onclick={event => onMaximize()}>
            </button>
        </app-controls>

        <div class="title">
            {title}
        </div>

        <app-controls>
            <button class="menu toggle-popover"></button>
            <popover class="popover">
                <span class="popover-item import">Importer</span>
                <span class="popover-item about">À propos</span>
            </popover>
        </app-controls>

    </app-titlebar>
)





