
import { h, app as hyperapp } from '../hyperapp'



export const AppToolbar = ({providers}) => (

    <app-toolbar>
        <div>
            <button class="open" title="Ouvrir un fichier">Ouvrir</button>
            <button class="save" title="Enregistrer la liste courante">Enregistrer</button>
            <button class="new-product" title="Ajouter un film">Ajouter un film</button>
        </div>

        <div id="providers">
        {
            providers.map(({identifier, name, lang}, index) => (
                <div key={index}>
                    <input id={'provider-'+identifier+'-'+lang} class="provider-switch" type="radio" name="provider" value={identifier+'-'+lang} checked />
                    <label for={'provider-'+identifier+'-'+lang} class="provider">
                        {name} <div class="provider-lang">{lang.toUpperCase()}</div>
                    </label>
                </div>
            ))
        }
        </div>

    </app-toolbar>

)

