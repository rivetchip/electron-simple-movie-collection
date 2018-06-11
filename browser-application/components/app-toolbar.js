
import { h, app as hyperapp } from '../hyperapp'



export const AppToolbar = ({providerIndex, providers, onProviderChange, onHamburger, onOpen, onSave, onNewProduct}) => (

    <app-toolbar>
        <div>
            <button class="hamburger" onclick={event => onHamburger()}>
                Menu
            </button>

            <button class="open" onclick={event => onOpen()} title="Ouvrir un fichier">
                Ouvrir
            </button>
    
            <button class="save" onclick={event => onSave()} title="Enregistrer la liste courante">
                Enregistrer
            </button>

            <button class="new-product" onclick={event => onNewProduct()} title="Ajouter un film">
                Ajouter un film
            </button>
        </div>

        {providers && (
            <div id="providers">
                {providers.map(({identifier, name, lang}, index) => (
                    <div key={index}>
                        <input
                            id={'provider-'+identifier+'-'+lang}
                            class="provider-switch" type="radio"
                            name="provider" value={identifier+'-'+lang}
                            checked={providerIndex == index}
                            onchange={event => onProviderChange({index})}
                        />
                        <label for={'provider-'+identifier+'-'+lang} class="provider">
                            {name} <div class="provider-lang">{lang.toUpperCase()}</div>
                        </label>
                    </div>
                ))}
            </div>
        )}

    </app-toolbar>

)

