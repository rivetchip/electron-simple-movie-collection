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


/**
 * Params of the Browser native API bridge
 * @param {Object} android 
 */
export function BrowserBridge(android) {
    // transmute function in Java.@JavascriptInterface to promises
    return {
        appPlatform: 'browser',

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

