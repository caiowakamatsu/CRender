# CRender

An offline Rendering Engine.<br>
[Discord server!](https://discord.gg/ZjrRyKXpWg)

***
## Getting Started
If you're a developer, and want to get started and [contribute](#Contributing) to this. The instructions are below.

**WARNING:** If you just want to use the application, look at [Installation](#Installation)

1) Make sure you have [Embree](https://github.com/embree/embree) 3.0 installed.
2) Clone this repository with `git clone https://github.com/LegendWasTaken/CRender.git`
3) If you're compiling from the command line
    1) `cd CRender` (Or what the directory is called)
    2) `cmake .` To configure the build
    3) `cmake --build . --target {TARGET}` `TARGET` can be Release or Debug
    4) You should find your executable in `/cmake-build-{TARGET}/CRender`
4) If you're using an IDE with cmake integration, just build the project as you normally would.

***
### Installation

1) Download the latest release for your operating system from [here](https://github.com/LegendWasTaken/CRender/releases)
2) Extract to destination
3) Download and install [Embree](https://github.com/embree/embree/releases/tag/v3.13.0)
<br />Note: On Windows, make sure to add `bin` to the  `PATH` variable - **follow the instructions on the Embree repository for your OS**. 
5) Run `CRender.exe` and enjoy!

***
## Building

### Linux

1) To build this project you will require [Intel Rendering Toolkit](https://software.intel.com/content/www/us/en/develop/tools/oneapi/rendering-toolkit/download.html) (online version), CMake, and `libxi-dev build-essential xorg libglfw3 libglfw3-dev libxinerama-dev libxcursor-dev` (these are the packages for Debian/Ubuntu)

2) You will need to run `source /opt/intel/oneapi/setvars.sh` so it sets the global vars needed for building. If this doesn't work please look for this file, and source that file in the same terminal window you compile in
3) Next you will need to run `cmake -DCMAKE_BUILD_TYPE=RELEASE .` this will build the cache
4) Finally you need to build it using `cmake --build . --target CRender`, now this will take a while.
5) Run `./CRender`

#### Bugs/issues with building:
If you get an error such as `./CRender: symbol lookup error: /opt/intel/oneapi/oidn/1.4.0/lib/libOpenImageDenoise.so.1: undefined symbol: _ZN3tbb6detail2r15spawnERNS0_2d14taskERNS2_18task_group_contextE` you need to remove every tbb package except the intel one.
If if you get a `glenable` etc error you need to get new drivers
If it cannot find embree you will need to go to `/opt/intel/oneapi` and move embree to a folder called `embree` instead of `embreeX.XXX`

***
## Contributing
If you would like to contribute to this repository, just make a half decent PR. I have no requirements for now...
OB[3~[3~