# immersion-module

Software package contains of three components:
* oculus_renderer
* oculus_position
* oculus_servos

Relation between software is presented on diagram below:

![Data Flow](https://github.com/piappl/immersion-module/blob/master/Images/data_flow.PNG?raw=true)

#### oculus_position

This software connects to Oculus Rift headset, retrieves headset's position and orientation and next sends these data to VR module computer controlling servos with video cameras attached to them (computer where **oculus_servos** application is running).  

##### Building and running
Major requirements for this software are **Oculus Rift SDK version 0.8.0.0 beta** and **Windows 7**. Other Oculus Rift SDK versions will not work - see remarks in **oculus_renderer** section.

You can use your favorite IDE to build this software, but there are some things you should know:
* add **include path** for Oculus SDK, e.g. *"E:\Libraries\OculusSDK\LibOVR\Include"*,
* add **library path** for Oculus SDK, e.g. *"E:\Libraries\OculusSDK\LibOVR\Lib\Windows\Win32\Release\VS2010"*,
* add following libraries to project: **LibOVR.lib**, **Ws2_32.lib**.

You should also build this application as architecture **x86**.

##### Configuration
In **main.cpp** file there is function called *sendMsg()*. In this function you can find following line:
```
addr.sin_addr.s_addr = inet_addr("192.168.230.70"); // Destination IP address
```
Oculus Rift headset's position and orientation will be sent to this IP address to control servos, so you should set there IP address of your computer where servos are connected to (see diagram at the beginning of this manual).

#### oculus_servos

This software receives Oculus Rift headset's position and orientation and controls servos with video cameras attached to them.

##### Usage
```
python ptrs.py oculus_1.0
```

#### oculus_renderer

This software is responsible for rendering video in stereo mode (from two video cameras) for Oculus Rift headset.

##### How it works?
It utilizes **Gstreamer** library to open video stream like video file (MP4, AVI, etc.) or stream from two video cameras (e.g. web cameras) and then receives video frames from selected video source. Next, each frame is converted to OpenGL texture, which is finally rendered on screen. 

Using OpenGL textures gives a lot of flexibility. Each frame can be distorted or modified in various ways. For example, to fix chromatic aberration in Oculus Rift headset this software separates (splits) color channels and adds some offset to each of them. Chromatic aberration fix parameters can be easily modified, so each user is able to tune this parameters to individual preferences.

##### What is needed to build and run this software?
* **Windows 7**. This software will not work on Windows 10 (see remarks regarding Oculus Rift SDK below). Windows 8 and Windows 8.1 were not tested.
* **Oculus SDK version 0.8.0.0 beta**. It is very important. Versions newer than 0.8.0.0 will not work. In version 0.8.0.0 Oculus Rift behaves as secondary screen in Windows, so you can easily drag this software's window and drop it in on Oculus Rift screen. Newer versions have removed this functionality. To use newer versions of Oculus Rift SDK some fixes in this software are needed. **Important note:** Oculus 0.8.0.0 beta does not work on Windows 10. You should use Windows 7 for this.
* **Gstreamer** library version **>= 1.8.1**. You can download Gstreamer library from this location: https://gstreamer.freedesktop.org/data/pkg/windows/. Version higher than 1.8.1 is needed, because lower versions do not support GL textures [see **Instalation notes** below].
* **GLM** library. GLM provides classes and functions designed and implemented with the same naming conventions and functionalities than GLSL so that anyone who knows GLSL, can use GLM as well in C++ [see **Instalation notes** below].
* **GLEW** library. The OpenGL Extension Wrangler Library (GLEW) is a cross-platform open-source C/C++ extension loading library. GLEW provides efficient run-time mechanisms for determining which OpenGL extensions are supported on the target platform [see **Instalation notes** below].
* **SOIL** library. SOIL is a tiny C library used primarily for uploading textures into OpenGL. You can download it from: http://www.lonesock.net/soil.html. Installation notes are included in downloaded library package.
* **QtCreator**. It is not mandatory, but in this repository you can find already configured QtCreator project file. You have to just change paths to libraries installed on your system in project .pro file. If you do not use QtCreator, you can build it also in VisualStudio or in other favorite software.

**Installation notes [for Gstreamer]:**

You should install two packages: **gstreamer-1.0-devel-x86-1.X.Y.msi** and **gstreamer-1.0-x86-1.X.Y.msi**, where *X* and *Y* mean version number. Notice that version **x86** is needed, not x64. 

After installation please verify if you have system variable **GSTREAMER_1_0_ROOT_X86** set to directory in Gstreamer location, e.g.  *"C:\Libraries\gstreamer\1.0\x86\"*.

You can also add *"C:\ Libraries \gstreamer\1.0\x86\bin"* to **PATH** system variable to use Gstreamer applications like “gst-launch” directly from command line.

**Installation notes [for GLM]:**

Installation notes and more information can be found on GLM website: http://glm.g-truc.net/. Installation of this library is very easy. There is no need to build this library, because it is based only on headers files.

**Installation notes [for GLEW]:**

Installation notes can be found directly on GLEW library website: http://glew.sourceforge.net/install.html. You should build this library from source using CMake.

##### Building and running
Open .pro file in QtCreator and configure paths to libraries installed on your system. Remember to configure project as **x86**, not x64.
```
GST_LIB_PATH = C:/Libs/gstreamer/1.0/x86
GLM_LIB_PATH = C:/Libs/glm
GLEW_LIB_PATH = C:/Libs/glew-1.13.0
SOIL_LIB_PATH = C:/Libs/soil
```
To run this software you should probably also provide **necessary DLLs** files, e.g. **glew32.dll**. It all depends how you built libraries (static or dynamic). Static libraries do not need DLLs. During application startup you will receive information about missing DLLs files. Put them to application’s directory to fix these errors.

##### Software usage
Launch dialog window looks like below:

![Launch Dialog](https://github.com/piappl/immersion-module/blob/master/Images/1.PNG?raw=true)

**Video source** has two possible options:
* Cameras,
* Test MP4 Video. 

**Cameras** option uses Gstreamer pipeline to open two separate video cameras streams. You can modify that pipeline to your preferences in **gst_streamer.cpp** file. 

Gstreamer pipeline for **Logitech web cameras** used in this project looks like below:
```
std::string pipe1{"udpsrc port=1235 caps=\"application/x-rtp, media=(string)video, "
                  "clock-rate=(int)90000, encoding-name=(string)H264\" ! "
                  "rtph264depay ! h264parse ! decodebin ! videoconvert ! "
                  "videoflip method=clockwise ! glupload ! "
                  "gleffects_identity ! identity name=video1 ! glimagesink sync=0"};

std::string pipe2{"udpsrc port=1234 caps=\"application/x-rtp, media=(string)video, "
                  "clock-rate=(int)90000, encoding-name=(string)H264\" ! "
                  "rtph264depay ! h264parse ! decodebin ! videoconvert ! "
                  "videoflip method=clockwise ! glupload ! "
                  "gleffects_identity ! identity name=video2 ! glimagesink sync=0"};
```
**Note** that first stream should be named **video1** and the second **video2**.

**Test MP4 Video** option uses Gstreamer pipeline to open sample MP4 file. By default it opens file in location *"Video/SampleMP4/video.mp4"* You can change this location in **gst_streamer.cpp** file.
```
pipeline_video_0_ = GST_PIPELINE(gst_parse_launch("filesrc location=Video/SampleMP4/video.mp4 ! "
                                                  "qtdemux ! avdec_h264 ! videoconvert ! "
                                                  "glupload ! gleffects effect=0 ! "
                                                  "fakesink sync=1 name=video1", nullptr));
```
**Video size** should be set to video’s frame size. 

**Experimentally**, this software also supports rendering robot's state on Oculus Rift screen. **UDP robot status port** allows to configure network port on which robot sends its status to this software, like battery level, etc.

When program starts two windows will appear. One of these is responsible for rendering video in stereo mode for Oculus Rift headset. The second window is configuration window. This window gives a lot of rendering configuration options. It looks like below:

![Configurtion Dialog](https://github.com/piappl/immersion-module/blob/master/Images/2.PNG?raw=true)

**Chromatic aberration** panel allows to setup parameters for fixing chromatic aberration in Oculus Rift headset. **Mask texture** is very important part. Chromatic aberration is mostly visible in regions some distance away from lens' optical axis. Texture mask simulates lens in headset. White color on this mask mean that in these regions chromatic aberration has to be corrected at maximum level. Black color means that there is no need for chromatic aberration correction.

Chromatic aberration correction is done by splitting color channels and adding a small offset to each of them. It can be seen at picture below:

![Chromatic aberration](https://github.com/piappl/immersion-module/blob/master/Images/3.PNG?raw=true)

Below texture mask there are six values which can be configured to correct chromatic aberration. Feel free to experiment with them to obtain the best results for you.

**GUI** displays robot’s state. If robot is not connected GUI is automatically hid. 

Under **Video camera** panel it is possibility to configure video width, height and position on screen. Two configurations can be set, and then you can select one of them.

**Save settings** button saves all settings to configuration file. This file is then loaded on application startup, and settings are restored.