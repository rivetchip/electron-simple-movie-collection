# Simple Movie Collection app
(currently under heavy development)

Simple application for managing your personnal movie collection and can fetch infos from TMDb.

Available on 🐧 Linux 🤖 Android

-> To import your current GCStar/GCFilms collection, look at the import section.

<img src="media/screenshot-desktop.png" height="350"> <img src="media/screenshot-mobile.png" height="350">


## Getting in Touch

If you have any questions regarding the use or development,
you can [file an issue](https://github.com/rivetchip/simple-movie-collection/issues)


## Development builds

Required dependencies : meson gtk3-devel glib-devel json-glib libcurl
You can build and install using [Meson](http://mesonbuild.com/) :

### Building

~~~sh
cd gtkapplication
meson build
ninja -C build
ninja -C build install
~~~

### Debug

~~~sh
cd gtkapplication
meson -Ddeveloper_mode=true --buildtype=debugoptimized devbuild
ninja -C devbuild
./devbuild/moviecollection --inspect
~~~


## Android

It is a simple Webview with basic viewing capabilities.
Required dependencies : nodejs npm inkscape java-1.8.0-openjdk-devel (or any latest java-jdk)
Install [Android SDK](https://developer.android.com/studio/#command-tools) (currently v28)

### Building

~~~sh
npm run bundle && ./android.sh
~~~

It will create mimaps, compile, sign, launch the APK using ADB.
APK will be stored in `/android/apk/simplemoviecollection.apk`

### Debug

~~~sh
npm run serve   (keep terminal open)

./android.sh test    (to install and show console logs)
./android.sh install
~~~

Watch-out for Chrome versions when building for different API -> add targets to `.babelrc`
(eg: default Android 23 with Chrome 51 don't support async/await)



## Import from GCStar/GCFilms

Required dependencies : nodejs npm npm-jsdom

~~~sh
node ./import-gcstar.js mycollection.gcs  [destination.ndjson]
~~~

[x] Copy all your posters inside a "/posters/" folder :

- your-collection.ndjson
- posters/
    - superposter.jpg
    - superposter2.jpg
    - etc...


[x] How to use on Android :

- copy the file to your sdcard : `/Android/data/fr.spidery.moviecollection/moviecollection.ndjson`
- copy all the posters in : `/Android/data/fr.spidery.moviecollection/posters/`

/!\ The document's name __must be__ `moviecollection.ndjson`

- /Android/data/fr.spidery.moviecollection/
    - moviecollection.ndjson
    - posters/
        - superposter.jpg
        - superposter2.jpg



## Credits :

[Lato Fonts](http://www.latofonts.com/lato-free-fonts/) - SIL Open Font License

[GNOME Adwaita Icons](https://github.com/GNOME/adwaita-icon-theme) - GPL v2

[Android Build tool](https://medium.com/@authmane512/7260e1e22676)


### Icons :

Godzilla made by [Freepik](https://www.flaticon.com/authors/freepik) from [www.flaticon.com](https://www.flaticon.com) is licensed by [CC 3.0 BY](http://creativecommons.org/licenses/by/3.0/)

Clapperboard made by [Roundicons](https://www.flaticon.com/authors/roundicons) from [www.flaticon.com](https://www.flaticon.com) is licensed by [CC 3.0 BY](http://creativecommons.org/licenses/by/3.0/)


## License

[MIT License](LICENSE)

