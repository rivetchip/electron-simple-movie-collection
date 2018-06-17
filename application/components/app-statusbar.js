
import { h, app as hyperapp } from '../hyperapp'



export const AppStatusbar = ({productCount}) => (
    <app-statusbar>

        {productCount} films

        <app-filters>
        </app-filters>

    </app-statusbar>
)


