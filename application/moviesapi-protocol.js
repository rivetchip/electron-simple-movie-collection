
import {forEach, map, replace, urlstringify} from './utils'


//// Application custom Protocol API

const apiProviders = {
    tmdb : {
        apiKey: 'e7c537c8a509f6d4b6975b3ff7bd5dbf',
        endpoint: 'api.themoviedb.org',
        version: 3,
        translations: ['fr', 'en'], // available languages
        requestUrl: '/{version}/{actionUrl}', // https:// endpoint / action

        movieWebPage: 'https://www.themoviedb.org/movie/{sourceId}',

        defaultParameters: { // default parameters added to each request
            api_key: '{apiKey}',
            language: '{lang}'
        },
        actions: {
            search: {
                actionUrl: 'search/movie',
                parameters: {
                    query: '{keyword}'
                },
                transitions: { // fields transitions from api to app format
                    // format field : api field, callback
                    sourceId: ['id'],
                    title: ['title'],
                    original: ['original_title'],
                    overview:['overview'],
                    dateReleased: ['release_date', convertDate],
                },
            },
            movie: {
                actionUrl: 'movie/{keyword}',
                parameters: {
                    append_to_response: 'casts,keywords'
                },
                transitions: {
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
    }

}

/**
 * Convert datas received from the api to the application format
 * 
 * @param {String} provider tmdb
 * @param {String} action
 * @param {*} keyword
 * @param {String} language
 * @param {String} movieWebPage
 * @param {Object} transitions
 * @param {Object|Array} results
 */
const moviesApiRequestTransition = ({providerConfig, action, keyword, lang, transitions, results}) => {

FIXME();



    const {movieWebPage} = providerConfig

    if(!transitions){
        throw new Error('moviesApiRequestTransition: transitions not found')
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

        if( !transitions ){
            response.sourceId = null
        }

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
 * @param {String} source provider
 * @param {String} action 
 * @param {String} keyword 
 * @param {String} language
 */
export const fetchmovie = async ({source, action, keyword, lang = 'fr'}) => {

    const providerConfig = apiProviders[source]

    if(!providerConfig) {
        throw new Error('Provider not found')
    }

    // get provider configuration

    let {apiKey, endpoint, version, requestUrl, actions, translations, defaultParameters} = providerConfig

    if(!translations.includes(lang)) {
        throw new Error('Language not found')
    }

    if(!actions[action]) {
        throw new Error('Action not found')
    }

    // get current action configuration

    let {actionUrl, parameters} = actions[action]

    actionUrl = replace(actionUrl, {
        '{keyword}': keyword
    })

    requestUrl = replace(requestUrl, {
        '{version}': version,
        '{actionUrl}': actionUrl
    })

    // merge default params with params + replace placeholder values

    if(defaultParameters) {
        parameters = Object.assign({}, defaultParameters, parameters || {})
    }

    parameters = map(parameters, (parameter) => replace(parameter, {
        '{keyword}': keyword,
        '{apiKey}': apiKey,
        '{lang}': lang
    }))

    requestUrl += '?'+urlstringify(parameters)

    // do the request & get the response

    const response = await fetch('https://'+endpoint+requestUrl, {
        headers: {
            'User-Agent': 'Mozilla/5.0',
            'Accept': 'application/json',
            'Accept-Language': lang
        },
        mode: 'cors',
        cache: 'default'
    })
    .then((response) => response.json())

    // check is the response is success

    const {isResponseSuccess, getResponseResult} = providerConfig

    let isSuccess = isResponseSuccess(action, response)

    if(isSuccess !== true){
        let {code, message} = isSuccess

        throw new Error('Response error: '+code+', '+message)
    }

    let results = getResponseResult(action, response) // single or multiple results


    return moviesApiRequestTransition({
        providerConfig,
        results,
        action,
        keyword,
        lang,
    })
}



/**
 * Wrapper for the request() to make exclusive movies requests
 * 
 * @param {String} provider
 * @param {String} action 
 * @param {*} keyword 
 * @param {String} language
 */
export const fetchmoxxxxvie = async (request) => {

    let [fullProvider, action, keyword] = request.split('/', )


console.log(request.split('/', 3))


    //let response = await moviesApiRequest({provider, action, keyword, language})







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

    if( !Array.isArray(values) ) {
        return []
    }

    return values.map((value) => value.name)
}

function convertNamedVaue(value) {
    return value.name || ''
}

function convertDirector(crews) {

    if( !Array.isArray(crews) ) {
        return []
    }

    return crews.find((crew) => crew.job == 'Director')
}

function convertActorsRoles(casts) {

    if( !Array.isArray(casts) ) {
        return []
    }

    return casts.map((cast) => [cast.name, cast.character])
}

function convertRating(value) {
    return Math.round(value) / 2 // rounded & convert tmdb /10 to /5
}

