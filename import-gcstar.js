#!/usr/bin/env nodejs

const {readFile: fsreadFile, writeFile: fswriteFile} = require('fs')
var {basename} = require('path')

const {JSDOM: jsdom} = require('jsdom')


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




async function app(sourceFile) {

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


    let xml = await readFile(sourceFile)

    const {window} = new jsdom(xml, {contentType: 'text/xml'})

    let domcollection = window.document.querySelector('collection')

    let type = domcollection.getAttribute('type')
    let version = domcollection.getAttribute('version')
    let count = domcollection.getAttribute('items')

    console.log('Collection', {type, version, count})

    let items = domcollection.querySelectorAll('item')

    items.forEach((item, index) => {
        let [movieId, movie] = convertFromDomItem(item, index);
    
        converted.collection[movieId] = movie
    })


    return converted
}

function convertFromDomItem(domitem, index) {
    const movieId = domitem.getAttribute('id') || Date.now()

    function convertDateReleased(date) {
        date = date.replace(/\//gi, '-')

        if(date.length == 4) {//year
            date += '-01-01' // january
        }

        let format = new Date(date.replace( /(\d{2})-(\d{2})-(\d{4})/, "$2/$1/$3"))

        if(format == 'Invalid Date') {
            return null
        }

        return [format.getYear(), format.getMonth(), format.getDay()].join('-')
    }

    function convertPoster(image) {
        return basename(image)
    }

    function convertRating(rating) {
        return Math.floor(rating / 2) // GC on /10, here on /5
    }

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
            return [cols[0].textContent, cols[1].textContent] //actor, role
        }),
        rating: convertRating(domitem.getAttribute('rating')),
        ratingPress: convertRating(domitem.getAttribute('ratingpress')),
    }





    console.log(result.actors)









    return [movieId, result]
}








app('./Movie list.gcs')
.then((converted) => {
    console.log('Conversion finished to file "simplemovie.json"')
    // console.log(converted)
})
.catch((error) => {
    console.log('Error:', error)
})
