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