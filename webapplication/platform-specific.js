'use strict';

/**
 * Params of the WebkitGtkBridge native API bridge
 * @param {Object} bridge 
 */
export function DeviceBridge(bridge) {
    // transmute function in JSC interface to promises
    return {
        developerMode: bridge.developer_mode,

        platform: bridge.platform,

        applicationClose(status) {

        },

        async openCollection() {
            return bridge.openCollection()
        },
    
        async saveCollection(contents) {
            return bridge.saveCollection(contents)
        },
    
        async getPoster(posterName) {
            return bridge.getPoster(posterName)
        },
    
        async savePoster(posterName, contents) {
            return bridge.getPoster(posterName, contents)
        }
    }
}

