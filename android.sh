#!/bin/bash

set -e

cd android

SDK="/home/fox/Android/Sdk"

AAPT="$SDK/build-tools/28.0.0/aapt"
DX="$SDK/build-tools/28.0.0/dx"
ZIPALIGN="$SDK/build-tools/28.0.0/zipalign"
APKSIGNER="$SDK/build-tools/28.0.0/apksigner"
PLATFORM="$SDK/platforms/android-28/android.jar"


echo "Cleaning..."
rm -rf obj/*
rm -rf bin/*
rm -rf java/fr/spidery/moviecollection/R.java

echo "Generating R.java file..."
$AAPT package -f -m -J java -M AndroidManifest.xml -S res -I $PLATFORM

echo "Compiling..." #java-1.8.0-openjdk-devel
javac -d obj -classpath java -bootclasspath $PLATFORM -source 1.7 -target 1.7 java/fr/spidery/moviecollection/MainActivity.java
javac -d obj -classpath java -bootclasspath $PLATFORM -source 1.7 -target 1.7 java/fr/spidery/moviecollection/R.java

echo "Translating in Dalvik bytecode..."
$DX --dex --output=classes.dex obj

echo "Making APK..."
$AAPT package -f -m -F bin/hello.unaligned.apk -M AndroidManifest.xml -S res -I $PLATFORM -A assets
$AAPT add bin/hello.unaligned.apk classes.dex

echo "Aligning APK..."
$ZIPALIGN -f 4 bin/hello.unaligned.apk bin/simplemoviecollection.apk

if [ ! -f mykey.keystore ]; then
	echo "Generate Key..."
	keytool -genkey -v -keystore mykey.keystore -keyalg RSA -keysize 2048 -validity 365 -alias app
fi
if [ -f mykey.keystore ]; then
    echo "Signing APK..."
	$APKSIGNER sign --ks mykey.keystore bin/simplemoviecollection.apk --ks-pass file:keystorepass
fi

if [ "$1" == "test" ]; then
	echo "Launching..."
	adb install -r bin/simplemoviecollection.apk
	adb shell am start -n fr.spidery.moviecollection/.MainActivity

	adb logcat -v color -s "CONSOLE"
fi







