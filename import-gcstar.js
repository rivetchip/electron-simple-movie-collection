#!/usr/bin/env nodejs

const {readFile: fsreadFile, writeFile: fswriteFile} = require('fs')
var {basename} = require('path')

const {JSDOM: jsdom} = require('jsdom')


// get arguments and launch app

let [nodeExec, script, sourceFile, destinationFile = 'simplemovie.json'] = process.argv

console.log('=====')
console.log('Import from GCStar/GCfilms (may not work on all collections)')

if(!sourceFile) {
    console.log('=====')
    console.log('Missing sourceFile argument')
    console.log('Usage:')
    console.log('$ ./import-gcstar.js source.gcs  [destination.json]')
    process.exit(1)
}


app({sourceFile, destinationFile})
.then((converted) => {
    console.log('=====')
    console.log('Conversion finished to file "'+destinationFile+'"')
    // console.log(converted)
})
.catch((error) => {
    console.log('Error:', error)
})


async function app({sourceFile, destinationFile}) {

    let converted = {
        version: 1,
        metadata: {
            imported: new Date(),
            created: new Date(),
            source: 'GCfilms'
        },
        options: {},
        collection : {}
    }


    let [errorRead, xml] = await to(readFile(sourceFile))

    if(errorRead) {
        console.log('Cannot read file "'+sourceFile+'"')
        console.log(errorRead)
        process.exit(1)
    }

    const {window} = new jsdom(xml, {contentType: 'text/xml'})

    let domcollection = window.document.querySelector('collection')

    // get infos on collection

    let type = domcollection.getAttribute('type')
    let version = domcollection.getAttribute('version')
    let count = Number(domcollection.getAttribute('items'))

    console.log('Start converting collection...', {type, version, count})

    // create progress

    let items = domcollection.querySelectorAll('item')

    // start converting items

    items.forEach((item, index) => {
        let [movieId, movie] = convertFromDomItem(item, index);
    
        converted.collection[movieId] = movie
    })

    // save the item

    let [errorWrite] = await to(writeFile(destinationFile, JSON.stringify(converted)))

    if(errorWrite) {
        console.log('Cannot write file "'+destinationFile+'"')
        console.log(errorWrite)
        process.exit(1)
    }


    return converted
}

function convertDateReleased(date) {
    date = String(date)
    
    date = date.replace(/\//gi, '-')

    if(date.length == 4) {//year
        date += '-01-01' // january
    }

    let format = new Date(date.replace( /(\d{2})-(\d{2})-(\d{4})/, "$2/$1/$3"))

    if(format == 'Invalid Date') {
        // console.log('convertDateReleased skip', {date})
        return null
    }

    return [format.getYear(), format.getMonth(), format.getDay()].join('-')
}

function convertSimpleDate(date) {
    date = String(date)

    let format = new Date(date.replace( /(\d{2})\/(\d{2})\/(\d{4})/, "$2/$1/$3"))

    if(format == 'Invalid Date') {
        // console.log('convertSimpleDate skip', {date})
        return null
    }

    return format
}

function convertPoster(image) {
    image = String(image)
    
    return basename(image)
}

function convertRating(rating) {
    rating = Number(rating)
    
    return Math.floor(rating / 2) // GC on /10, here on /5
}

function convertTags(tags) {
    tags = String(tags)
    
    return tags.split(',')
}

//webPage="http://www.themoviedb.org/movie/128##TMDb (FR)"
function convertSource(webPage) {
    webPage = String(webPage)
    
    if(webPage.includes('themoviedb')) {
        return 'tmdb'
    }

    return null
}

function convertWebPage(webPage) {
    webPage = String(webPage)

    let split = webPage.split('##')
    return split[0]
}

function concertSourceId(webPage) {
    webPage = String(webPage)

    if(webPage.includes('themoviedb')) {
        let split = webPage.split('##')
        let pageUrl = split[0]

        return basename(pageUrl)
    }

    return null
}


function convertFromDomItem(domitem, index) {
    const movieId = domitem.getAttribute('id') || Date.now()

    const result = {
        title: domitem.getAttribute('title'),
        original: domitem.getAttribute('original'),
        tagline: null,//slogan
        serie: domitem.getAttribute('serie'),
        duration: Number(domitem.getAttribute('time')),
        dateReleased: convertDateReleased(domitem.getAttribute('date')),
        director: domitem.getAttribute('director'),
        poster: convertPoster(domitem.getAttribute('image')),
        description: domitem.querySelector('synopsis').textContent,
        comment: domitem.querySelector('comment').textContent,
        countries: mapObject(domitem.querySelectorAll('country line col'), (col) => col.textContent),
        genres: mapObject(domitem.querySelectorAll('genre line col'), (col) => col.textContent),
        actors: mapObject(domitem.querySelectorAll('actors line'), (line) => {
            let cols = line.querySelectorAll('col')
            return [cols[0].textContent, cols[1].textContent] //[actor, role]
        }),
        rating: convertRating(domitem.getAttribute('rating')),
        ratingPress: convertRating(domitem.getAttribute('ratingpress')),
        dateCreated: convertSimpleDate(domitem.getAttribute('added')),
        dateModified: null,
        favorite: domitem.getAttribute('favourite') == 1,
        companies: [],
        keywords: convertTags(domitem.getAttribute('tags')),
        source: convertSource(domitem.getAttribute('webPage')),
        webPage: convertWebPage(domitem.getAttribute('webPage')),
        sourceId: concertSourceId(domitem.getAttribute('webPage')),
    }

    return [movieId, result]
}


function to(promise) {
    return promise.then((response) => {
       return [null, response];
    })
    .catch(error => [error]);
 }
function readFile(filename) {
    return new Promise((resolve, reject) => {
        fsreadFile(filename, 'utf8', (error, data) => {
            return error ? reject(error) : resolve(data)
        })
    })
}
function writeFile(filename, content) {
    return new Promise((resolve, reject) => {
        fswriteFile(filename, content, 'utf8', (error) => {
            return error ? reject(error) : resolve()
        })
    })
}
function mapObject(object, iteratee) {
    object = Object(object)
    // as array of objects
    return Object.keys(object)
    .map((key) => iteratee(object[key], key))
}
