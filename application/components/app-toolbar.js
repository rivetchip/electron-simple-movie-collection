
import { h } from '../hyperapp'

export const ComponentAppToolbar = ({buttons, providers, providerIndex, onProviderChange}) => (

    <app-toolbar>

        {buttons.map((button, key) => (
            <button class={button.class} onclick={event => button.onclick()} title={button.title}>
                {button.name}
            </button>
        ))}

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

