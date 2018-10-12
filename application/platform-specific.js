'use strict';

/**
 * Params of the Electron native API bridge
 * @param {Object} bridge 
 */
export function ElectronBridge(bridge) {
    // don't need to transmute functions, already done in preload.js
    return {
        platform: 'desktop-window', ...bridge
    }
}

/**
 * Params of the Android native API bridge
 * @param {Object} bridge 
 */
export function AndroidBridge(bridge) {
    // transmute function in Java.@JavascriptInterface to promises
    return {
        platform: 'mobile',

        applicationClose(status) {

        },

        async openCollection(parser) { // TODO if file not exist -> undefined
            let storage = bridge.openCollection() // or null


console.log('storage'+storage)

            return parser(storage)
        },
    
        async saveCollection(storage, stringify) {
            let content = stringify(storage)

            return bridge.saveCollection(content) // or null
        },
    
        async getPoster(filename) {
    
        },
    
        async savePoster(filename, content) {
            
        }
    }
}


/**
 * Params of the WebkitGtkBridge native API bridge
 * @param {Object} bridge 
 */
export function WebkitgtkBridge(bridge) {
    // transmute function in Java.@JavascriptInterface to promises
    return {
        platform: 'desktop-webview',

        applicationClose(status) {

        },

        async openCollection(parser) {

            console.log(bridge.openCollection())
        },
    
        async saveCollection(storage, stringify) {

        },
    
        async getPoster(filename) {
    
        },
    
        async savePoster(filename, content) {
            
        }
    }
}

