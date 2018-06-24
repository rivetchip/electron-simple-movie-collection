# Simple Movie Collection cross-app
(currently in beta and under heavy development)

Simple application for managing your personnal movie collections.
It can also import informations from TMDb.

Available on Linux, Windows & Android.


<img src="media/screenshot-desktop.png" height="350"> <img src="media/screenshot-mobile.png" height="350">




Debugging Desktop :
-------------------

```sh
npm install

npm run serve   (keep terminal open)

npm run debug   (or using vscode debug)
```

Linux: Error "ENOSPC" : (max watch files limit exeed)
```sh
echo fs.inotify.max_user_watches=524288 | sudo tee -a /etc/sysctl.conf && sudo sysctl -p
```


Debugging Android :
-------------------

First install [latest Android SDK](https://developer.android.com/studio/#command-tools) (currently v28), then :

```sh
npm run bundle-android && ./android.sh test
```

It will compile, sign, launch the APK using ADB and finally show console Logs.


Build Desktop :
---------------

```sh
npm install

npm run build-linux32  (choose your arch)
npm run build-linux64
npm run build-win32
npm run build-win64
npm run build-mac
```

Release executable will be in `/release-builds/` folder

Linux: missing `libXss.so.1` library > Install `libXScrnSaver` package


Build Android :
---------------

(Same steps as debugging)

```sh
npm install

npm run bundle-android && ./android.sh
```

Release APK will be `/android/bin/simplemoviecollection.apk`



Credits :
---------

[Lato Fonts](http://www.latofonts.com/lato-free-fonts/)
[GNOME Adwaita Icons](https://github.com/GNOME/adwaita-icon-theme)
[Star rating script](https://codepen.io/mburnette/pen/eNNZbm)
[Electron IPC Logger](https://github.com/ungoldman/electron-ipc-log)
[Android Build tool](https://medium.com/@authmane512/7260e1e22676)


## License

[MIT License](LICENSE)
