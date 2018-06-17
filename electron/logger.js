const package = require('./package.json')


/**
 * Event logger for simple messages
 * 
 * @param {Array} messages 
 */
const logger = (stream) => (...messages) => {
    messages.forEach((message) => {
        stream('\x1b[36m%s\x1b[0m', '[logger]')
    
        if(message.message){
            stream(message.message)
        }

        if(message.stack){
            stream(message.stack)
        }

        stream(message)
    })
}

exports.logger = logger(console.log)
