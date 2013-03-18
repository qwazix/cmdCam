#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <FCam/N9.h>

#include "SoundPlayer.h"
#include <linux/videodev2.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <QString>
#include <QStringList>
#include <QtCore/QCoreApplication>
#include <QDebug>
#include <QDateTime>

//SDK doesn't have these defines
#ifndef V4L2_CID_FLASH_LED_MODE
#define V4L2_CID_FLASH_LED_MODE 10225921
#endif

#ifndef V4L2_FLASH_LED_MODE_FLASH
#define V4L2_FLASH_LED_MODE_FLASH 1
#endif


/** \file */

// Select the platform
namespace Plat = FCam::N9;

/***********************************************************/
/* Shutter sound                                           */
/*                                                         */
/* This example shows how to declare and attach a device,  */
/* and write the appropriate actions. In this example, the */
/* camera will trigger two actions at the beginning of the */
/* exposure: a flash, and a shutter sound.                 */
/* See SoundPlayer class for more information.             */
/***********************************************************/
int main(int argc, char **argv) {
    QCoreApplication a(argc, argv);

    QString path;
    int interval;
    int shutter = 50000;
    int whitebalance = 3500;
    int ISO = 100;
    int width = 2592;
    int height = 1968;
    float focus = -1.0f;
    bool flashon = false;
    bool soundon = true;


    QStringList arguments = a.arguments();
    for (int i = 0; i < arguments.count(); ++i) {
        QString parameter = arguments.at(i);
        if (parameter == "--path") {
            if (i + 1 >= arguments.count())
                qFatal("--path requires an argument");
            path = arguments.at(i + 1);
            i++;
        } else if (parameter == "--interval") {
            if (i + 1 >= arguments.count())
                qFatal("--interval requires an argument");
            interval = arguments.at(i + 1).toInt();
            i++;
        } else if (parameter == "--shutter") {
            if (i + 1 >= arguments.count())
                qFatal("--shutter requires an argument");
            shutter = arguments.at(i + 1).toInt();
            i++;
        } else if (parameter == "--whitebalance") {
            if (i + 1 >= arguments.count())
                qFatal("--whitebalance requires an argument");
            whitebalance = arguments.at(i + 1).toInt();
            i++;
        } else if (parameter == "--focus") {
            if (i + 1 >= arguments.count() || arguments.at(i + 1).toFloat() < 0.001 )
                qFatal("--focus requires a non-zero argument");
            focus = 1/arguments.at(i + 1).toFloat();
            i++;
        } else if (parameter == "--iso") {
            if (i + 1 >= arguments.count())
                qFatal("--whitebalance requires an argument");
            ISO = arguments.at(i + 1).toInt();
            i++;
        } else if (parameter == "--flash") {
            flashon = true;
        } else if (parameter == "--nosound") {
            soundon = false;
        } else if (parameter == "--help") {
            qDebug() << "qt command line camera";
            qDebug() << "--path - path to save photos defaults to /home/user/MyDocs/DCIM/";
            qDebug() << "         the path must exist";
            qDebug() << "--shutter - shutter time in μs";
            qDebug() << "--whitebalance - white balance in K";
            qDebug() << "--focus - focus distance in m";
            qDebug() << "--flash - fire the flash";
            qDebug() << "--nosound - disable clicking sound";
            exit(0);
        }
    }


    // Devices
    Plat::Sensor sensor;
    Plat::Flash flash;
    Plat::Lens lens;

    if (focus == -1.0f) focus = lens.farFocus();

    // We defined a custom device to play a sound during the
    // exposure. See SoundPlayer.h/cpp for details.
    SoundPlayer audio;

    sensor.attach(&flash); // Attach the flash to the sensor
    sensor.attach(&audio); // Attach the sound player to the sensor

    // Explicitly power up the sensor
    if (sensor.initialize(0) == -1){
        printf("Error powering up the sensor.\n");
        return 1;
    }

    // Tell flash to use LED Flash
    int fd = open("/dev/v4l-subdev10", O_RDWR);
    struct v4l2_control ctrl;
    ctrl.id = V4L2_CID_FLASH_LED_MODE;
    ctrl.value = V4L2_FLASH_LED_MODE_FLASH;
    ioctl(fd, VIDIOC_S_CTRL, &ctrl);

    // Set the shot parameters
    FCam::Shot shot1;
    shot1.exposure = shutter;
    shot1.gain = (float)ISO/100;
    shot1.whiteBalance = whitebalance;
    shot1.image = FCam::Image(2592, 1968, FCam::UYVY);
    lens.setFocus(std::max(lens.nearFocus(), focus) , lens.maxFocusSpeed());

    // Action (Flash)
    FCam::Flash::FireAction fire(&flash);
    fire.duration = flash.minDuration();
    fire.time = 0;//shot1.exposure - fire.duration;
    fire.brightness = flash.maxBrightness();

    // Action (Sound)
    SoundPlayer::SoundAction click(&audio);
    click.time = 0; // Start at the beginning of the exposure
    click.setWavFile("/usr/share/sounds/ui-tones/snd_camera_shutter.wav");

    // Attach actions
    if (flashon) shot1.addAction(fire);
    if (soundon) shot1.addAction(click);

    // Order the sensor to capture a shot.
    // The flash and the shutter sound should happen simultaneously.
    sensor.capture(shot1);
    assert(sensor.shotsPending() == 1); // There should be exactly one shot
    // Retrieve the frame from the sensor
    FCam::Frame frame = sensor.getFrame();
    assert(frame.shot().id == shot1.id); // Check the source of the request

    // Write out the file
    if (!path.length()) path = QString("/home/user/MyDocs/DCIM/");
    QString filename = path + QString::number(QDateTime::currentMSecsSinceEpoch()) + QString(".jpg");
    FCam::saveJPEG(frame, filename.toStdString());

    // Check that the pipeline is empty
    assert(sensor.framesPending() == 0);
    assert(sensor.shotsPending() == 0);


}
