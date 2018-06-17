
# Simple Movie Collection cross-app

Available on Linux, Windows, Mac, Android.


TODO : I really need to write this README :)





bugs list :
starrating : use </> fragments
- preload: window.ipc => max events listeners exeed
- title bar alignement (sur toute largeur, pas juste le width)


todo debugging (desktop) :
----------------

npm install

npm run serve (keep terminal open)

rpm run debug ( with vscode inspect )


ENOSPC
echo fs.inotify.max_user_watches=524288 | sudo tee -a /etc/sysctl.conf && sudo sysctl -p



todo building desktop :
-----------------------

npm install

npm run pack-linux64

>> ./release-builds/

missing libXss.so.1 >> dnf install libXScrnSaver



todo building webview android :
-------------------------------

npm install

npm run bundle-android && ./android.sh

>> ./android/bin/simplemoviecollection.apk



todo credits :
--------------

https://github.com/ungoldman/electron-ipc-log

http://www.latofonts.com/lato-free-fonts/

https://github.com/GNOME/adwaita-icon-theme

star rating
https://codepen.io/mburnette/pen/eNNZbm

android build tool
https://medium.com/@authmane512/7260e1e22676

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