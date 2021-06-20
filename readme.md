# CRender

An offline Rendering Engine, made for learning purposes.

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
## Contributing

If you would like to contribute to this repository, just make a half decent PR. I have no requirements for now...
