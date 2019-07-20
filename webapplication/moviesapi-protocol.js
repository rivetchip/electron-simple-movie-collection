'use strict';

import {lookup, map, replace, urlstringify} from './helpers'


//// Application custom Protocol API

const apiProviders = {
    tmdb : {
        apiKey: 'e7c537c8a509f6d4b6975b3ff7bd5dbf',
        endpoint: 'api.themoviedb.org',
        version: 3,
        translations: ['fr', 'en'], // available languages
        requestUrl: '/{version}/{actionUrl}', // https://endpoint/action?params

        movieWebPage: 'https://www.themoviedb.org/movie/{sourceId}',

        defaultParameters: { // default parameters added to each request
            api_key: '{apiKey}',
            language: '{lang}'
        },
        actions: {
            search: {
                actionUrl: 'search/movie',
                parameters: { // more parameters
                    query: '{keyword}',
                },
                transitions: { // fields transitions from api to app format
                    // format field : api field, callback
                    sourceId: ['id'], // mandatory 'sourceId'
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
                    serie: ['belongs_to_collection', convertNamedValue],
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
            let {success, status_code: errcode, status_message: errmessage} = response

            return {success: success !== false, errcode, errmessage}
        },

        getResponseResult(action, response) { // get api response results
            return response.results || response
        },
    }

}



/**
 * Wrapper for the request() to make exclusive movies requests
 * 
 * @param {String} source provider
 * @param {String} action 
 * @param {String} keyword 
 * @param {String} lang
 */
export async function fetchmovie({source, action, keyword, lang = 'fr'}) {

    const providerConfig = apiProviders[source]

    if(!providerConfig) {
        throw 'Provider not found'
    }

    // get provider configuration

    let {apiKey, endpoint, version, requestUrl, actions, translations, defaultParameters} = providerConfig

    if(!translations.includes(lang)) {
        throw 'Language not found'
    }

    if(!actions[action]) {
        throw 'Action not found'
    }

    // get current action configuration

    let {actionUrl, parameters = {}, transitions} = actions[action]

    actionUrl = replace(actionUrl, {
        '{keyword}': keyword
    })

    requestUrl = replace(requestUrl, {
        '{version}': version,
        '{actionUrl}': actionUrl
    })

    // merge default params with params + replace placeholder values

    if(defaultParameters) {
        parameters = {...defaultParameters, ...parameters}
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

    let {success, errcode, errmessage} = isResponseSuccess(action, response)

    if(!success){
        throw 'Response success error: ' + errcode + ', ' + errmessage
    }

    let results = getResponseResult(action, response) // single or multiple results
    // TODO tmdb pagination


    return transformFetchResponse({source, lang, providerConfig, results, transitions})
}


/**
 * Convert datas received from the api to the application format
 * 
 * @param {String} source original provider
 * @param {String} lang
 * @param {Object} providerConfig
 * @param {Object|Array} results
 * @param {Object} transitions
 */
function transformFetchResponse({source, lang, providerConfig, results, transitions}) {

    const {movieWebPage} = providerConfig

    // convert a single set of result ; if is multiple results -> return array results

    const transpiler = (result) => transformResultSet({source, lang, transitions, movieWebPage, result})

    return isIterable(results) ? map(results, transpiler) : transpiler(results)
}


/**
 * Transpile a single set of result
 * 
 * @param {String} source
 * @param {String} lang
 * @param {Object} result
 * @param {Object} transitions
 * @param {String} movieWebPage
 */
function transformResultSet({source, lang, result, transitions, movieWebPage}) {

    let transpiler = (result) => (transition) => { // transform single value
        const [field, callback] = transition

        let value = lookup(result, field)

        return callback ? callback(value) : value
    }

    let response = map(transitions, transpiler(result))

    // add source infos

    let {sourceId} = response

    response.source = source
    response.lang = lang

    if(sourceId && movieWebPage) { // TODO
        response.webPage = movieWebPage.replace('{sourceId}', sourceId);
    }

    return response
}



/**
 * Check if we can loop thru the object
 * 
 * @param {*} object 
 */
function isIterable(object) {
    return object != null && typeof object[Symbol.iterator] === 'function'
}


function convertDate(dateString) {

    let [year, month, day] = dateString.split('-', 3); // 1998-08-21

    return [year, month, day].join('-')
}

function convertText(value) {
    return value || ''
}

function convertNamedArray(values) {
    return values.map((value) => value.name)
}

function convertNamedValue(value) {
    return value.name || ''
}

function convertDirector(crews) {

    let director = crews.find((crew) => crew.job == 'Director')

    return director.name
}

function convertActorsRoles(casts) {
    return casts.map((cast) => [cast.name, cast.character])
}

function convertRating(value) {
    return Math.round(value) / 2 // rounded & convert tmdb /10 to /5
}

