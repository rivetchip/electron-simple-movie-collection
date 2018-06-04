
# Simple movie collection electron app


bugs list :
starrating : use </> fragments
- preload: window.ipc => max events listeners exeed



todo debugging :
----------------

npm install

npm run serve

rpm run debug ( with vscode inspect )


todo building :
---------------

npm install

npm run pack-linux64

folder ./release-builds/



todo credits :
--------------

https://github.com/ungoldman/electron-ipc-log

http://www.latofonts.com/lato-free-fonts/

https://github.com/GNOME/adwaita-icon-theme

star rating
https://codepen.io/mburnette/pen/eNNZbm


todo attribution: ( about page ) 🐛

https://www.themoviedb.org/about/logos-attribution
"This product uses the TMDb API but is not endorsed or certified by TMDb."










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