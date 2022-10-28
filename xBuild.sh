#!/bin/sh
./gradlew --offline iD
adb shell am start com.youngtr.jnievner/.DLActivity
