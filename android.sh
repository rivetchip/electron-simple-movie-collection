#!/bin/bash

set -e

cd android

SDK="/home/$USER/Android/Sdk"

AAPT="$SDK/build-tools/28.0.0/aapt"
DX="$SDK/build-tools/28.0.0/dx"
ZIPALIGN="$SDK/build-tools/28.0.0/zipalign"
APKSIGNER="$SDK/build-tools/28.0.0/apksigner"
PLATFORM="$SDK/platforms/android-28/android.jar"


echo "Cleaning..."
mkdir -p apk
rm -rf obj/*
rm -rf apk/*
rm -rf java/fr/spidery/moviecollection/R.java


echo "Converting icons..."

declare -A resolutions
resolutions=([ldpi]=36 [mdpi]=48 [hdpi]=72 [xhdpi]=96 [xxhdpi]=144 [xxxhdpi]=192)

ic_launcher=../media/monster-icon-square.svg
ic_launcher_round=../media/monster-icon-round.svg

inkscape_export_svg() {
    local "${@}"

    echo Create $destination @ $size
    inkscape --export-area-page --file $source --export-png $destination --export-width $size --export-height $size
    # Area 0:0:512:512
}

for K in "${!resolutions[@]}"
do
    size=${resolutions[$K]}

    ic_mipmap=res/mipmap-${K}/ic_launcher.png
    ic_mipmap_round=res/mipmap-${K}/ic_launcher_round.png

    mkdir -p ${ic_mipmap%/*}
    mkdir -p ${ic_mipmap_round%/*}

    if [ ! -f $ic_mipmap ] || [ $ic_launcher -nt $ic_mipmap ]; then #not exist or older than
        inkscape_export_svg size=$size source=$ic_launcher destination=$ic_mipmap
    fi
    if [ ! -f $ic_mipmap_round ] || [ $ic_launcher -nt $ic_mipmap_round ]; then
        inkscape_export_svg size=$size source=$ic_launcher_round destination=$ic_mipmap_round
    fi
done

echo "Create Store icon..."
if [ ! -f store-icon.png ]; then
    inkscape_export_svg size=512 source=$ic_launcher destination=store-icon.png
fi


echo "Generating R.java file..."
$AAPT package -f -m -J java -M AndroidManifest.xml -S res -I $PLATFORM

echo "Compiling..." #java-1.8.0-openjdk-devel
javac -d obj -classpath java -bootclasspath $PLATFORM -source 1.7 -target 1.7 java/fr/spidery/moviecollection/MainActivity.java
javac -d obj -classpath java -bootclasspath $PLATFORM -source 1.7 -target 1.7 java/fr/spidery/moviecollection/R.java

echo "Translating in Dalvik bytecode..."
$DX --dex --output=classes.dex obj

echo "Making APK..."
$AAPT package -f -m -F apk/hello.unaligned.apk -M AndroidManifest.xml -S res -I $PLATFORM -A assets
$AAPT add apk/hello.unaligned.apk classes.dex

echo "Aligning APK..."
$ZIPALIGN -f 4 apk/hello.unaligned.apk apk/simplemoviecollection.apk

if [ ! -f mykey.keystore ]; then
	echo "Generate Key..."
	keytool -genkey -v -keystore mykey.keystore -keyalg RSA -keysize 2048 -validity 365 -alias app
	
    touch keystorepass
	echo "==> Don't forget to add the password to the 'android/keystorepass' file."
    echo "[Press a key to continue]"
    read a
fi

echo "Signing APK..."
$APKSIGNER sign --ks mykey.keystore --ks-pass file:keystorepass apk/simplemoviecollection.apk


if [ "$1" == "test" ]; then
	echo "Launching..."
	adb install -r apk/simplemoviecollection.apk
	adb shell am start -n fr.spidery.moviecollection/.MainActivity

	adb logcat -c # clear log
	adb logcat -v color -s "CONSOLE"
	#adb logcat -v color --pid=`adb shell pidof -s fr.spidery.moviecollection`
fi







