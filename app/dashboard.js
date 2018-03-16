'use strict';


exports.createSnackbar = (() => {
    let duration = 3000
    let previous // previous snack

    const onAnimationendEvent = (event, elapsed) => {
        let target = event.target

        if( event.animationName == 'snackbar-fadeout' ) {
            // when the animation end, we remove self
            target.remove()

            if( previous === target ) {
                previous = null
            }
        }
    }

    return (viewport, message) => {
        if( previous ) {
            // dismiss
            previous.remove()
        }

        let snackbar = document.createElement('snackbar')
        snackbar.innerText = message
        snackbar.classList.add('is-visible')
        
        snackbar.addEventListener('animationend', onAnimationendEvent)
        snackbar.addEventListener('webkitAnimationEnd', onAnimationendEvent)

        let timeoutId = setTimeout(() => {
            // dismiss
            snackbar.classList.remove('is-visible');
        }, duration)

        previous = snackbar
        viewport.appendChild(snackbar)
    }
})()

