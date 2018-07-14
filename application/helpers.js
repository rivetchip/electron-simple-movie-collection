
/**
 * Convert dot-notation to the real object
 * @param {Object} context 
 * @param {String} token 
 */
export function lookup(context, token) {
    return token.split('.').reduce((accumulator, value) => accumulator[value], context);
}

/**
 * Loop over an onject
 * @param {Object} object 
 * @param {Function} iteratee 
 */
function eachObject(object, iteratee) { // lodash
    object = Object(object)

    return Object.keys(object).forEach((key) => iteratee(object[key], key))
}

/**
 * Loop over and array
 * @param {Array} array 
 * @param {Function} iteratee 
 */
function eachArray(array, iteratee) { // lodash
    return array.forEach((value, key) => iteratee(value, key))
}

/**
 * Loop over an object or array
 * @param {Array|Object} collection 
 * @param {Function} iteratee 
 */
export function forEach(collection, iteratee) {
    const func = Array.isArray(collection) ? eachArray : eachObject

    return func(collection, iteratee)
}

/**
 * Map over an object
 * @param {Object} object 
 * @param {Function} iteratee 
 */
function mapObject(object, iteratee) { // lodash
    object = Object(object)

    const result = {}

    Object.keys(object).forEach((key) => {
        result[key] = iteratee(object[key], key)
    })
    return result

    // as array of objects
    // return Object.keys(object).map((key) => {
    //     return iteratee(key, object[key]);
    // });
}

/**
 * Map over an array
 * @param {Array} array 
 * @param {Function} iteratee 
 */
function mapArray(array, iteratee) { // lodash
    return array.map(iteratee)
}

/**
 * Map over an object or array
 * @param {Object|Array} collection 
 * @param {Function} iteratee 
 */
export function map(collection, iteratee) {
    const func = Array.isArray(collection) ? mapArray : mapObject

    return func(collection, iteratee)
}

/**
 * String replace with {key: value}
 * @param {String} string 
 * @param {Object} replaces 
 */
export function replace(string, replaces) {

    forEach(replaces, (value, key) => {
        string = string.replace(key, value)
    })

    return string
}


/**
 * Check if object is empty
 * @param {Object} object 
 */
function isEmptyObject(object) {
    return !Object.keys(object).length
}

/**
 * Check if parameter can be empty
 * @param {any} value 
 */
function isEmptyGeneric(value) {
    return !value.length
}

/**
 * Check if an object or array is empty (has values)
 * @param {Object|Array} value 
 */
export function isEmpty(value) { // lodash
    const func = Array.isArray(value) ? isEmptyGeneric : isEmptyObject

    return func(value)
}

/**
 * Filter object properties based on predicate
 * @param {Object} object 
 * @param {Function} predicate 
 */
function filterObject(object, predicate) {
    object = Object(object)

    const result = {}

    Object.keys(object).forEach((key) => {
        const value = object[key]
        if(predicate(value, key, object)) {
            result[key] = value
        }
    })
    return result
}

/**
 * Filter an array based on its values
 * @param {Array} array 
 * @param {Function} predicate 
 */
function filterArray(array, predicate) {
    return array.filter(predicate)
}

/**
 * Filter a collection based on ts properties
 * @param {Object|Array} collection 
 * @param {Function} predicate 
 */
export function filter(collection, predicate) {
    const func = Array.isArray(collection) ? filterArray : filterObject

    return func(collection, predicate)
}



/**
 * Return a query string based on an object {key: value}
 * FIXME does not work with arrays, only object
 * @param {Object} object 
 */
export function urlstringify(object) {
    let escape = encodeURIComponent
    let params = []

        //TODO use map()
    forEach(object, (value, key) => {
        params.push(key + '=' + escape(value))
    })

    return params.join('&')
}


/**
 * Trim first & last characters of a string
 * @param {String} string 
 * @param {String} character 
 */
export function trimchar(string, character) {

    const first = [...string].findIndex(char => char !== character)

    const last = [...string].reverse().findIndex(char => char !== character)

    return string.substring(first, string.length - last)
}

