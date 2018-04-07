const package = require('../package.json')

const {protocol} = require('electron')

const https = require('https')
const {parse: urlparse} = require('url')

const {stringify: urlstringify} = require('querystring')


//// Application custom Protocol API

const apiProviders = {
    tmdb : {
        key: 'e7c537c8a509f6d4b6975b3ff7bd5dbf',
        endpoint: 'api.themoviedb.org',
        version: 3
    }
}





/**
 * HTTPS external request
 * @param {array} options 
 */
const request = (options) => {

    return new Promise((resolve, reject) => {

        https.request(options, (response) => {
            response.setEncoding('utf8')

            let statusCode = response.statusCode
            let headers = response.headers

            let body = ''
    
            response.on('data', (chunk) => {
                body += chunk
            })

            response.on('end', () => {
                resolve({statusCode, headers, body})
            })
        })

        .on('error', (error) => {
            reject(error.message || error)
        })

        .end()
    })
}


// covnert dot-ntoation to the real object
const lookup = (context, token) => {

    for( let key of token.split('.') ) {

        context = context[key]

        if(context == null) {
            return null
        }
    }

    return context

    //return token.split('.').reduce((accumulator, value) => accumulator[value], context);
}

const convertDate = (dateString) => {

    let [year, month, day] = dateString.split('-', 3); // 1998-08-21

    return [year, month, day].join('-')
}

const convertText = (value) => {
    return value || ''
}

const convertNamedArray = (values) => {
    let response = []

    if( !Array.isArray(values) ) {
        return []
    }

    for( var value of values ) {
        response.push(value.name)
    }

    return response
}

const convertNamedVaue = (value) => {
    return value.name || ''
}

const convertDirector = (crews) => {

    if( !Array.isArray(crews) ) {
        return []
    }

    for( let crew of crews ) {

        if(crew.job == 'Director') {
            // get director name; usually the first
            return crew.name
        }
    }

    return ''
}

const convertActorsRoles = (casts) => {

    if( !Array.isArray(casts) ) {
        return []
    }

    let response = []

    for( let cast of casts ) {
        //[actor, role]

        response.push([
            cast.name, cast.character
        ])
    }

    return response
}

const convertRating = (value) => {
    return Math.round(value) / 2 // rounded & convert tmdb /10 to /5
}

// trim first & last characters of a string
const trimchar = (string, character) => {

    const first = [...string].findIndex(char => char !== character)

    const last = [...string].reverse().findIndex(char => char !== character)

    return string.substring(first, string.length - last)
}

// fields transitions from api to app format
const transitions = {
    // format field : api field, callback
    title: ['title'],
    original: ['original_title'],
    tagline: ['tagline', convertText],
    duration: ['runtime'],
    dateReleased: ['release_date', convertDate],
    director: ['casts.crew', convertDirector],
    description: ['overview'],
    countries: ['production_countries', convertNamedArray],
    genres: ['genres', convertNamedArray],
    actors: ['casts.cast', convertActorsRoles],
    ratingPress: ['vote_average', convertRating],
    serie: ['belongs_to_collection', convertNamedVaue],
    companies: ['production_companies', convertNamedArray],
    keywords: ['keywords.keywords', convertNamedArray],
}


/**
 * Convert datas received from the api to the application format
 * 
 * @param {String} provider 
 * @param {String} action 
 * @param {*} keyword 
 * @param {*} results 
 */
const moviesapiRequestTransition = (provider, action, keyword, results) => {

    if(action == 'search' ) {
        // multiple ; simple

        let response = []

        for( let movie of results ) {
            
            response.push({
                provider, // tmdb
                providerId: movie.id, // store for later
                title: movie.title,
                original: movie.original_title,
                overview: movie.overview,
                dateReleased: convertDate(movie.release_date)
            })
        }

        return response
    }

    if(action == 'movie' ) {
        // single ; complete

        let response = {}

        for( let key in transitions ) {

            let [field, callback] = transitions[key]

            let value = lookup(results, field)

            if(callback) {
                value = callback(value)
            }

            response[key] = value
        }

        // add source informations :

        let sourceId = results.id
        let imdbId = results.imdb_id

        response.source = provider // tmdb
        response.sourceId = sourceId

        if(provider == 'tmdb') {
            response.webPage = 'http://www.themoviedb.org/'+sourceId
        }
        

        if(imdbId) {
            response.imdbId = imdbId
        }

        return response
    }
}

/**
 * Wrapper for the request() to make exclusive movies requests
 * 
 * @param {String} provider
 * @param {String} language
 * @param {String} action 
 * @param {*} keyword 
 */
const moviesapiRequest = (provider, language, action, keyword) => {

    let requestUrl
    let parameters = {} // query string
    let queryString // full querystring

    if(!apiProviders[provider]) {
        throw new Error('moviesapiRequest: provider not found')
    }

    const {key: apiKey, endpoint, version} = apiProviders[provider]


    switch(action) {

        case 'search': // search movies based on a keyword
            requestUrl = 'search/movie'
            parameters.query = keyword
        break

        case 'movie': // get full informations about a movie
            requestUrl = 'movie/'+keyword
            parameters.append_to_response = 'casts,keywords'
        break

        case 'discover': // discover new movies
            //requestUrl = 'discover/movie'
        break

        case 'popular': // popular movies
            //requestUrl = 'movie/popular'
        break

        case 'now-playing': // movies in theatres
            //requestUrl = 'movie/now_playing'
        break

        default: // todo
            throw new Error('moviesapiRequest: action not found')
        break
    }

    if(requestUrl) {
        // append language and api key
        parameters.api_key = apiKey
        parameters.language = language

        // then encode url with parameters

        queryString = requestUrl+'?'+urlstringify(parameters)
    }

    return request({
        host: endpoint,
        path: '/'+version+'/'+queryString,
        headers: {
            'User-Agent': 'Mozilla/5.0',
            'Accept': 'application/json',
            'Accept-Language': language //'fr,fr-FR,en-US,en'
        }
    })
    .then(({statusCode, headers, body}) => JSON.parse(body))
    .then((response) => {

        if( response.success === false ) {
            let code = response.status_code //tmdb
            let message = response.status_message

            throw new Error('moviesapiRequest: '+code+'-'+message)
        }
    
        return response.results || response // if multiple
    })
    .then((results) => moviesapiRequestTransition(provider, action, keyword, results))
}


/**
 * Register Eletron custom protocol
 */
function registerMoviesapiProtocol() {

    const protocolHandler = (request, callback) => {

        // moviesapi://tmdb-fr/search/blade runner

        let {hostname, pathname: queryString, query} = urlparse(request.url)

        let {provider, language} = hostname.split('-', 2) // tmdb-fr

        queryString = trimchar(decodeURIComponent(queryString), '/') // /search/blade%20runner

        // get actions from url

        let [action, keyword] = queryString.split('/', 2) // search/blade runner
        // let slashIndex = queryString.indexOf('/')
        // let action = queryString.substr(0, slashIndex);
        // let keyword = queryString.substr(slashIndex+1)

        // send the request back to the client

        let mimeType = 'application/json'
        let charset = 'utf8'

        moviesapiRequest(provider, language, action, keyword)

        .then((response) => callback({
            data: JSON.stringify(response), mimeType, charset
        }))

        .catch((error) => callback({
            data: JSON.stringify({error: error.message || error}), mimeType, charset
        }))
    }

    const completionHandler = (error) => {
        error && logger('MoviesapiProtocol', error) // TODO import logger
    }


    protocol.registerStringProtocol('moviesapi', protocolHandler, completionHandler)
}



exports.registerMoviesapiProtocol = registerMoviesapiProtocol