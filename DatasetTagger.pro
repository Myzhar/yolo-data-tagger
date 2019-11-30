#-------------------------------------------------
#
# Project created by QtCreator 2017-12-11T15:47:29
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DatasetTagger
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

unix{
    LIBS += \
        -lopencv_core \
        -lopencv_imgproc \
        -lopencv_highgui
}


MOC_DIR             = temp/moc
RCC_DIR             = temp/rcc
UI_DIR              = temp/ui
OBJECTS_DIR         = temp/obj
DESTDIR             = bin

INCLUDEPATH += C:\VS2017-OpenCV300\install\include\
               C:\VS2017-OpenCV300\install\include\opencv\
               C:\VS2017-OpenCV300\install\include\opencv2

LIBS += C:\VS2017-OpenCV300\install\lib\opencv_calib3d300.lib\
        C:\VS2017-OpenCV300\install\lib\opencv_core300.lib\
        C:\VS2017-OpenCV300\install\lib\opencv_features2d300.lib\
        C:\VS2017-OpenCV300\install\lib\opencv_flann300.lib\
        C:\VS2017-OpenCV300\install\lib\opencv_highgui300.lib\
        C:\VS2017-OpenCV300\install\lib\opencv_imgcodecs300.lib\
        C:\VS2017-OpenCV300\install\lib\opencv_imgproc300.lib\
        C:\VS2017-OpenCV300\install\lib\opencv_ml300.lib\
        C:\VS2017-OpenCV300\install\lib\opencv_objdetect300.lib\
        C:\VS2017-OpenCV300\install\lib\opencv_photo300.lib\
        C:\VS2017-OpenCV300\install\lib\opencv_shape300.lib\
        C:\VS2017-OpenCV300\install\lib\opencv_stitching300.lib\
        C:\VS2017-OpenCV300\install\lib\opencv_superres300.lib\
        C:\VS2017-OpenCV300\install\lib\opencv_video300.lib\
        C:\VS2017-OpenCV300\install\lib\opencv_videoio300.lib\
        C:\VS2017-OpenCV300\install\lib\opencv_videostab300.lib











INCLUDEPATH += ./include

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    src/qtrainsetexample.cpp \
    src/qobjbbox.cpp \
    src/qimagescene.cpp \
    src/qenhgraphicsview.cpp

HEADERS += \
    mainwindow.h \
    include/qtrainsetexample.h \
    include/qobjbbox.h \
    include/qimagescene.h \
    include/qenhgraphicsview.h

FORMS += \
    mainwindow.ui

RESOURCES += \
#    styles/styles.qrc \
    qss.qrc
