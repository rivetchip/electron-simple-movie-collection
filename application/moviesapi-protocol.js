const package = require('../package.json')

// Electron flow

const {protocol} = require('electron')

const {request: httpsrequest} = require('https')
const {parse: urlparse} = require('url')

const {stringify: urlstringify} = require('querystring')

// Application fow

const {logger} = require('./logger')



//// Application custom Protocol API

const apiProviders = {
    tmdb : {
        key: 'e7c537c8a509f6d4b6975b3ff7bd5dbf',
        endpoint: 'api.themoviedb.org',
        version: 3,
        requestUrl: '/{version}/{actionUrl}', // https:// endpoint / action

        movieWebPage: 'https://www.themoviedb.org/movie/{sourceId}',

        defaultParameters: { // default parameters added to each request
            api_key: '{apiKey}',
            language: '{language}'
        },
        actions: {
            search: {
                requestUrl: 'search/movie',
                parameters: {
                    query: '{keyword}'
                }
            },
            movie: {
                requestUrl: 'movie/{keyword}',
                parameters: {
                    append_to_response: 'casts,keywords'
                }
            },
            discover: {

            },
            popular: {

            },
            nowplaying: {

            }
        },

        isResponseSuccess(action, response) { // check if api request is success
            if( response.success === false ) {
                let code = response.status_code //tmdb
                let message = response.status_message

                return {code, message}
            }

            return true
        },

        getResponseResult(action, response) { // get api response results
            return response.results || response
        },

        transitions: { // fields transitions from api to app format
            search: {
                // format field : api field, callback
                sourceId: ['id'],
                title: ['title'],
                original: ['original_title'],
                overview:['overview'],
                dateReleased: ['release_date', convertDate],
            },
            movie: {
                // format field : api field, callback
                sourceId: ['id'],
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
            
        }
    }
}


/**
 * HTTPS external request
 * @param {array} options 
 */
function request(options) {

    return new Promise((resolve, reject) => {

        httpsrequest(options, (response) => {
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

/**
 * Convert datas received from the api to the application format
 * 
 * @param {String} provider 
 * @param {String} action 
 * @param {*} keyword 
 * @param {*} results 
 */
const moviesapiRequestTransition = (provider, providerConfiguration, language, action, keyword, results) => {

    // get transitions of the current action

    const {transitions: providerTransitions, movieWebPage} = providerConfiguration

    const transitions = providerTransitions[action]

    if(!transitions){
        throw new Error('moviesapiRequestTransition: transitions not found')
    }


    // convert a single set of result

    const transpileResult = (result) => {

        let response = {}

        for( let key in transitions ) {

            const [field, callback] = transitions[key]

            let value = lookup(result, field)

            if(callback) {
                value = callback(value)
            }

            response[key] = value
        }

        // add source infos

        response.source = provider

        if(language){
            response.language = language
        }

        if(response.sourceId && movieWebPage){
            response.webPage = movieWebPage.replace('{sourceId}', response.sourceId);
        }


        return response
    }


    // if is multiple results -> return array results

    if(isIterable(results)){

        let response = []

        for( let result of results ) {
            response.push(transpileResult(result))
        }

        return response
    }


    // if is single result -> return single object

    let response = transpileResult(results)

    return response
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

    let requestUrl // full url ( api request uri / action ?querystring )
    let queryString // query string (action parameters)


    // check current provider name exist

    const providerConfiguration = apiProviders[provider]

    if(!providerConfiguration) {
        throw new Error('moviesapiRequest: provider not found')
    }

    // get provider configuration

    let {key: apiKey, endpoint, version, requestUrl: apiRequestUrl, actions, defaultParameters} = providerConfiguration

    // check current action exist

    if(!actions[action]) {
        throw new Error('moviesapiRequest: action not found')
    }

    // get current action configuration

    let {requestUrl: actionUrl, parameters = {}} = actions[action]


    // replace action url placeholders

    actionUrl = actionUrl.replace('{keyword}', keyword)

    // request uri = api url + action url

    requestUrl = apiRequestUrl
    .replace('{version}', version)
    .replace('{actionUrl}', actionUrl)


    // merge default params with params

    if(defaultParameters) {
        parameters = Object.assign({}, defaultParameters, parameters)
    }

    // if parameters set, replace placeholder values

    let hasParameters = false

    for(const index in parameters) {
        let parameter = parameters[index]

        parameters[index] = parameter
        .replace('{keyword}', keyword)
        .replace('{apiKey}', apiKey)
        .replace('{language}', language)

        hasParameters = true
    }

    // finally encode parameters

    if(hasParameters) {

        // set the full, final request url

        queryString = '?'+urlstringify(parameters)

        requestUrl += queryString
    }


    // send the formated request to the api

    return request({
        host: endpoint,
        path: requestUrl,
        headers: {
            'User-Agent': 'Mozilla/5.0',
            'Accept': 'application/json',
            'Accept-Language': language //'fr,fr-FR,en-US,en'
        }
    })
    .then(({statusCode, headers, body}) => JSON.parse(body))
    .then((response) => {

        const {isResponseSuccess, getResponseResult} = providerConfiguration

        // check is the response is success

        var isSuccess = isResponseSuccess(action, response)

        if(isSuccess !== true){
            let {code, message} = isSuccess

            throw new Error('moviesapiRequest: '+code+'-'+message)
        }
    
        return getResponseResult(action, response) // single or multiple results
    })
    .then((results) => moviesapiRequestTransition(provider, providerConfiguration, language, action, keyword, results))
}


/**
 * Register Eletron custom protocol
 */
function registerMoviesapiProtocol() {

    const protocolHandler = (request, callback) => {

        // moviesapi://tmdb-fr/search/blade runner

        let {hostname, pathname: queryString, query} = urlparse(request.url)

        let [provider, language] = hostname.split('-', 2) // tmdb-fr

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
        error && logger('MoviesapiProtocol', error)
    }


    protocol.registerStringProtocol('moviesapi', protocolHandler, completionHandler)
}

exports.registerMoviesapiProtocol = registerMoviesapiProtocol




/**
 * Convert dot-notation to the real object
 * 
 * @param {Object} context 
 * @param {String} token 
 */
function lookup(context, token) {

    for( let key of token.split('.') ) {

        context = context[key]

        if(context == null) {
            return null
        }
    }

    return context

    // return token.split('.').reduce((accumulator, value) => accumulator[value], context);
}

/**
 * Check if we can loop thru the object
 * 
 * @param {*} object 
 */
function isIterable(object) {
    return object != null && typeof object[Symbol.iterator] === 'function'
}

// trim first & last characters of a string
function trimchar(string, character) {

    const first = [...string].findIndex(char => char !== character)

    const last = [...string].reverse().findIndex(char => char !== character)

    return string.substring(first, string.length - last)
}


function convertDate(dateString) {

    let [year, month, day] = dateString.split('-', 3); // 1998-08-21

    return [year, month, day].join('-')
}

function convertText(value) {
    return value || ''
}

function convertNamedArray(values) {
    let response = []

    if( !Array.isArray(values) ) {
        return []
    }

    for( var value of values ) {
        response.push(value.name)
    }

    return response
}

function convertNamedVaue(value) {
    return value.name || ''
}

function convertDirector(crews) {

    if( !Array.isArray(crews) ) {
        return []
    }

    for( let crew of crews ) {

        if(crew.job == 'Director') { // TODO multiple directors ?
            // get director name; usually the first
            return crew.name
        }
    }

    return ''
}

function convertActorsRoles(casts) {

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

function convertRating(value) {
    return Math.round(value) / 2 // rounded & convert tmdb /10 to /5
}

