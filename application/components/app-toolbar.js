
import { h } from '../hyperapp'



const ComponentProvider = ({ providerHash, provider, isSelected, onchange }) => (

    <div key={providerHash}>
        <input
            id={'provider-'+provider.identifier+'-'+provider.lang}
            type="radio"
            name="provider"
            class="provider-switch"
            checked={isSelected}
            onchange={event => onchange({providerHash})}
        />
        <label for={'provider-'+provider.identifier+'-'+provider.lang} class="provider">
            {provider.name}
            <div class="provider-lang">
                {provider.lang.toUpperCase()}
            </div>
        </label>
    </div>
)

export const ComponentAppToolbar = ({buttons, providers, providerHash, onProviderChange}) => (

    <app-toolbar>

        {buttons.map((button, key) => (
            <button class={button.class} onclick={event => button.onclick()} title={button.title}>
                {button.name}
            </button>
        ))}

        {providers && (
            <div id="providers">
                {providers.map((provider, hash) => (
                    <ComponentProvider
                        providerHash={hash}
                        provider={provider}
                        isSelected={providerHash == hash}
                        onchange={onProviderChange}
                    />
                ))}
            </div>
        )}

    </app-toolbar>
)

