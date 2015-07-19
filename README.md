# BinaryOverAudio
This project is inspired by badBIOS, an experiment to prove audio communication can be achieved
This program is written in 2013.12

There are two layers, the bottom layer is windows audio API, note that this
program only runs on windows. It is written in DEV-C++
The top layer builds frame format and has a GUI to make it more user friendly.
it builds and parses frame format of audio communication, the format is as followed:

TYPE: Preamble MACAddr LEN  Data  FCS
LEN:    1B       4B    4B   LEN   1B

Currently, FCS is not implemented because the packet loss rate is very high and 
the communication process is very slow. With improved mechanism, I believe it will
be faster. You can see the current mechanism from the following picture:

![](https://raw.githubusercontent.com/tanhangbo/BinaryOverAudio/master/screenshot2.jpg)

Every bit is represented as a certain duration of sound. Long duration means '1',
short duration means '0'. Because the mic and speaker are not accurate, so I choose
this way to make it work.


The GUI:

![](https://raw.githubusercontent.com/tanhangbo/BinaryOverAudio/master/screenshot1.png)

There is a example video, the origin video is missing, so I downloaded
it from a video website.
