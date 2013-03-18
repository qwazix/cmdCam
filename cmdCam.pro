#-------------------------------------------------
#
# Project created by QtCreator 2013-03-18T16:54:32
#
#-------------------------------------------------

QT       += core

QT       -= gui

LIBS += -L ../.. -ljpeg -lpthread -lpulse-simple -lFCam

TARGET = cmdCam
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    SoundPlayer.cpp

contains(MEEGO_EDITION,harmattan) {
    syspart.path = /usr/share/policy/etc/syspart.conf.d/
    syspart.files = cmdCam.syspart.conf

    pulse.path = /usr/share/policy/etc/pulse/xpolicy.conf.d/
    pulse.files = cmdCam.pulse.conf

    INSTALLS += syspart pulse
}

target.path = /opt/cmdCam/bin
INSTALLS += target


OTHER_FILES += \
    qtc_packaging/debian_harmattan/rules \
    qtc_packaging/debian_harmattan/README \
    qtc_packaging/debian_harmattan/manifest.aegis \
    qtc_packaging/debian_harmattan/copyright \
    qtc_packaging/debian_harmattan/control \
    qtc_packaging/debian_harmattan/compat \
    qtc_packaging/debian_harmattan/changelog \


HEADERS += \
    SoundPlayer.h
