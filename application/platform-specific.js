'use strict';

/**
 * Params of the Electron native API bridge
 * @param {Object} electron 
 */
export function ElectronBridge(bridge) {
    // don't need to transmute functions, already done in preload.js
    return {
        platform: 'dekstop', ...bridge
    }
}

/**
 * Params of the Android native API bridge
 * @param {Object} android 
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
 * Params of the Browser native API bridge
 * @param {Object} android 
 */
export function BrowserBridge(android) {
    // transmute function in Java.@JavascriptInterface to promises
    return {
        platform: 'browser',

        applicationClose(status) {

        },

        async openCollection(parser) {

        },
    
        async saveCollection(storage, stringify) {

        },
    
        async getPoster(filename) {
    
        },
    
        async savePoster(filename, content) {
            
        }
    }
}

