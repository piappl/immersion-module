#Project configuration
QT += core gui opengl network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OculusRenderer
TEMPLATE = app

DEFINES += GST_USE_UNSTABLE_API # Gstreamer uses 'bad plugins' - OpenGL

#Libs configuration
win32 {
    DEFINES += WIN32

    #Gstreamer
    GST_LIB_PATH = E:/Libraries/gstreamer/1.0/x86
    INCLUDEPATH += $${GST_LIB_PATH}/include \
                   $${GST_LIB_PATH}/include/gstreamer-1.0 \
                   $${GST_LIB_PATH}/include/libxml2 \
                   $${GST_LIB_PATH}/include/glib-2.0 \
                   $${GST_LIB_PATH}/lib/gstreamer-1.0/include \
                   $${GST_LIB_PATH}/lib/glib-2.0/include

    LIBS += -L$${GST_LIB_PATH}/lib -lgobject-2.0 -lgmodule-2.0 -lglib-2.0 -lgstbase-1.0 \
                                   -lgstapp-1.0 -lgstreamer-1.0 -lgthread-2.0 \
                                   -lgstgl-1.0 -lgstvideo-1.0

    #GLM
    GLM_LIB_PATH = E:/Libraries/glm-0.9.8.0/glm
    INCLUDEPATH += $${GLM_LIB_PATH}

    #GLEW
    GLEW_LIB_PATH = E:/Libraries/glew-2.0.0
    INCLUDEPATH += $${GLEW_LIB_PATH}/include

    LIBS += -L$${GLEW_LIB_PATH}/lib/Release/Win32 -lglew32

    #SOIL
    SOIL_LIB_PATH = E:/Libraries/soil
    INCLUDEPATH += $${SOIL_LIB_PATH}

    LIBS += -L$${SOIL_LIB_PATH}/lib -lSOIL

    #System libs
    LIBS += -lopengl32 \
            -luser32 \
            -lgdi32 \
            -lcomdlg32 \
            -lshell32
}

#Sources configuration
SOURCES += main.cpp\
    launch_dialog.cpp \
    main_window.cpp \
    gl_renderer.cpp \
    gst_streamer.cpp \
    configure_dialog.cpp

HEADERS += \
    launch_dialog.h \
    main_window.h \
    exception.h \
    gl_renderer.h \
    gst_streamer.h \
    configure_dialog.h \
    udp_receiver.h \
    robot_state.h

FORMS += \
    launch_dialog.ui \
    configure_dialog.ui \
    main_window.ui

RESOURCES += \
    resources.qrc