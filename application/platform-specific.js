'use strict';

/**
 * Params of the Electron native API bridge
 * @param {Object} electron 
 */
export function ElectronBridge(electron) {
    // don't need to transmute functions, already done in preload.js
    return {
        appPlatform: 'dekstop', ...electron
    }
}

/**
 * Params of the Android native API bridge
 * @param {Object} android 
 */
export function AndroidBridge(android) {
    // transmute function in Java.@JavascriptInterface to promises
    return {
        appPlatform: 'mobile',

        applicationClose(status) {

        },

        async openCollection() {

        },
    
        async saveCollection(storage) {

        },
    
        async getPoster(filename) {
    
        },
    
        async savePoster(filename, content) {
            
        }
    }
}

