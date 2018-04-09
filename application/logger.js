const package = require('../package.json')


// event logger for simple messages
const logger = (...messages) => {
    messages.forEach((message) => {
        console.log('\x1b[36m%s\x1b[0m', '[logger]')
    
        if(message.message){
            console.log(message.message)
        }

        if(message.stack){
            console.log(message.stack)
        }

        console.log(message)
    })
}

exports.logger = logger
