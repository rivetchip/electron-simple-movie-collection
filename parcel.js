const Bundler = require('parcel-bundler')
const fs = require('fs')

const argv = process.argv.slice(2) // remove node script

let tasks = {}, [taskName] = argv // get first

if(!taskName) {
    taskName = 'build:electron'
}



tasks['build:electron'] = () => {

    const source = './browser-application/index.html'

    const electronConfig = {
        production: true,
        outFile: 'index',
        outDir: './browser-bundle',
        publicUrl: './',
        cache: true,
        cacheDir: '.cache',
        minify: true,
        target: 'browser', // browser/node/electron
        https: false,
    
        logLevel: 2, // 3 = log everything, 2 = log warnings & errors, 1 = log errors
        sourceMaps: false, //(not supported in minified builds yet)
        detailedReport: false
    }

    return build(source, electronConfig);
}


tasks['build:cordova'] = () => {

    const source = './browser-application/index.html'

    const cordovaConfig = {
        production: true,
        outFile: 'index',
        outDir: './mobile-bundle',
        publicUrl: './',
        cache: true,
        minify: true,
        target: 'browser',
        https: false,
    
        logLevel: 2,
        sourceMaps: false,
        detailedReport: false
    }

    return build(source, cordovaConfig);
}





if(tasks[taskName]) {
    console.log('Running "'+taskName+'"...')

    let callback = tasks[taskName]
    let task = callback()

    task.then((result) => {
        console.log('finish')
    })

    task.catch((error) => {
        console.log('error', error)
    })
}



async function build(source, options) {

    const parcel = new Bundler(source, options)

    // clear output folder
    options.outDir && clearPath(options.outDir);

    // create the bundle
    let bundled = await parcel.bundle()

    return bundled
}


function clearPath(dirPath) {
    let files = fs.readdirSync(dirPath)

    for (let i = 0; i < files.length; i++) {
        let filePath = dirPath + '/' + files[i]

        fs.statSync(filePath).isFile() ? fs.unlinkSync(filePath) : clearPath(filePath)
    }

    fs.rmdirSync(dirPath)
}
