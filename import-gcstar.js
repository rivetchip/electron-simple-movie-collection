#!/usr/bin/env node

const {readFile: fsreadFile, writeFile: fswriteFile} = require('fs')
var {basename} = require('path')
const {JSDOM: jsdom} = require('jsdom')
const crypto = require('crypto')

// get arguments and launch app
let [nodeExec, script, sourceFile, destinationFile = 'simplemovie.ndjson'] = process.argv

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

    let converted = [
        // first line: metadata
        {version: 1, created: new Date(), imported: new Date(), source: 'GCfilms'}
    ]

    try {
        xml = await readFile(sourceFile)
    }
    catch(err) {
        console.log('Cannot read file "'+sourceFile+'"')
        console.log(err.message)
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

    items.forEach((item, index) => {
        converted.push(convertFromDomItem(item, index))
    })

    converted = converted.map(JSON.stringify)

    // save the item
    try {
        await writeFile(destinationFile, converted.join("\n"))
    }
    catch(err) {
        console.log('Cannot write file "'+destinationFile+'"')
        console.log(err.message)
        process.exit(1)
    }

    return converted
}

function str_pad(n) {
    return String('00' + n).slice(-2)
}

function convertDate(date) {
    return [
        date.getFullYear(),
        str_pad(date.getMonth()),
        str_pad(date.getDay())
    ].join('-')
}

function convertDateReleased(format) {
    format = String(format).replace(/\//gi, '-')

    if(format.length == 4) {
        return format // year
    }

    let date = new Date(format) // try withotu regex first

    if(date == 'Invalid Date') {

        let regexcomma = /(\d{4})-(\d{2})-(\d{2})/gi //2014-03-26
        let regexslash = /(\d{2})-(\d{2})-(\d{4})/gi //29/10/2013

        if(regexcomma.test(format)) {
            format = format.replace(regexcomma, "$2/$3/$1") //month/day/year
            date = new Date(format)
        }
        if(regexslash.test(format)) {
            format = format.replace(regexslash, "$2/$1/$3")
            date = new Date(format)
        }
    }

    if(date == 'Invalid Date') {
        console.log('convertDateReleased skip', {format})
        return null
    }

    return convertDate(date)
}

function convertSimpleDate(format) {
    format = String(format).replace(/\//gi, '-')

    let [day, month, year] = format.split('-')

    return [year, str_pad(month), str_pad(day)].join('-')
}

function convertPoster(image) {
    return basename(String(image))
}

function convertRating(rating) {
    return (Number(rating) * 10) // GC on /10, here on /100
}

function convertTags(tags) {
    return String(tags).split(',').filter(t => t.length && t.length > 0);
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

        return basename(pageUrl) // .../movie/68718
    }

    return null
}


function convertFromDomItem(domitem, index) {
    return {
        movieId: uuidv4(), //domitem.getAttribute('id') || Date.now(),
        title: domitem.getAttribute('title'),
        originalTitle: domitem.getAttribute('original'),
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
        rating: convertRating(
            domitem.getAttribute('rating') > 0 ? domitem.getAttribute('rating') : domitem.getAttribute('ratingpress')
        ),
        // ratingPress: convertRating(domitem.getAttribute('ratingpress')),
        dateCreated: convertSimpleDate(domitem.getAttribute('added')),
        dateModified: null,
        favorite: domitem.getAttribute('favourite') == 1,
        companies: [],
        keywords: convertTags(domitem.getAttribute('tags')),
        source: convertSource(domitem.getAttribute('webPage')),
        webPage: convertWebPage(domitem.getAttribute('webPage')),
        sourceId: concertSourceId(domitem.getAttribute('webPage')),
    }
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
            return error ? reject(error) : resolve(true)
        })
    })
}
function mapObject(object, iteratee) {
    object = Object(object)
    // as array of objects
    return Object.keys(object)
    .map((key) => iteratee(object[key], key))
}

// Generate RFC-4122 compliant random UUIDs using crypto API
function uuidv4() {
    // WHATWG crypto RNG
    // let b = window.crypto.getRandomValues(new Uint8Array(16))
    let b = crypto.randomBytes(16)

    // Per 4.4, set bits for version and 'clock_seq_hi_and_reserved'
    b[6] = (b[6] & 0x0f) | 0x40
    b[8] = (b[8] & 0x3f) | 0x80

    // Cache toString(16)
    let hexBytes = []
    for(let i = 0; i < 256; i++) {
        hexBytes[i] = (i + 0x100).toString(16).substr(1)
    }

    // bytesToUuid: convert array of 16 byte values to UUID string format of the form:
    // XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
    return hexBytes[b[0]] + hexBytes[b[1]] + 
        hexBytes[b[2]] + hexBytes[b[3]] + '-' +
        hexBytes[b[4]] + hexBytes[b[5]] + '-' +
        hexBytes[b[6]] + hexBytes[b[7]] + '-' +
        hexBytes[b[8]] + hexBytes[b[9]] + '-' +
        hexBytes[b[10]] + hexBytes[b[11]] + 
        hexBytes[b[12]] + hexBytes[b[13]] +
        hexBytes[b[14]] + hexBytes[b[15]]
}
