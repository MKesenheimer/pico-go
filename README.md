# C++ Code for the Waveshare Pico-Go
C++ project for the Waveshare [Pico-Go robot](https://www.waveshare.com/wiki/PicoGo).
Up to this point, only the line-follower has been ported to C++.

## Why don't you use the Python code from Waveshare?
Because Python is slow and I like C++. The performance of the robot is much better with this implementation.

## Building and flashing
Use the following commands to build and flash the code:
```
cd pico-go
mkdir build && cd build
cmake ..
make -j4
make pflash
```
The latter command makes use of [picotool](https://github.com/raspberrypi/picotool) to flash the binary.


## Video (not sped up)
You can see the robot in action below. The footage is not sped up.
![](https://github.com/MKesenheimer/pico-go/blob/master/files/pico-go.gif)
