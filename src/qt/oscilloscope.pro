QT       += qml quick serialport widgets quickcontrols2

android {
    QT += androidextras
}

CONFIG += c++11 console

VERSION = 0.2

DEFINES += APP_VERSION=\\\"$$VERSION\\\"

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    oscilloscope.cpp

HEADERS += \
    oscilloscope.h

FORMS +=
    # mainwindow.ui \

win32 {
    TARGET = oscilloscope_win
    RC_ICONS = res/oscilloscope.ico
}
unix {
    TARGET = oscilloscope_linux
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

RESOURCES += \
    resources.qrc

DISTFILES += \
    ../../../../../../../../mnt/data/daniel/src/qt/oscilloscope/src/qt/android/AndroidManifest.xml \
    ../../../../../../../../mnt/data/daniel/src/qt/oscilloscope/src/qt/android/build.gradle \
    ../../../../../../../../mnt/data/daniel/src/qt/oscilloscope/src/qt/android/gradle.properties \
    ../../../../../../../../mnt/data/daniel/src/qt/oscilloscope/src/qt/android/gradle/wrapper/gradle-wrapper.jar \
    ../../../../../../../../mnt/data/daniel/src/qt/oscilloscope/src/qt/android/gradle/wrapper/gradle-wrapper.properties \
    ../../../../../../../../mnt/data/daniel/src/qt/oscilloscope/src/qt/android/gradlew \
    ../../../../../../../../mnt/data/daniel/src/qt/oscilloscope/src/qt/android/gradlew.bat \
    ../../../../../../../../mnt/data/daniel/src/qt/oscilloscope/src/qt/android/res/values/libs.xml \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle.properties \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/values/libs.xml \
    android/src/org/qtproject/qt/UsbSerialInterface.java \
