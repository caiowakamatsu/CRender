<h3 align="center">CRender</h3>
<p align="center">
A high performance 3d rendering application
<br />
<br />
<a href="https://github.com/LegendWasTaken/CRender/issues">Report Bug</a>
·
<a href="https://github.com/LegendWasTaken/CRender/issues">Request Feature</a>
·
<a href="https://discord.gg/ZjrRyKXpWg">Discord</a>
</p>

<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#built-with">Built With</a></li>
      </ul>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Installation</a></li>
      </ul>
    </li>
    <li><a href="#usage">Usage</a></li>
    <li><a href="#roadmap">Roadmap</a></li>
    <li><a href="#contributing">Contributing</a></li>
    <li><a href="#license">License</a></li>
    <li><a href="#contact">Contact</a></li>
    <li><a href="#acknowledgments">Acknowledgments</a></li>
  </ol>
</details>



<!-- ABOUT THE PROJECT -->
## About The Project

I like graphics programming, I like writing open source software, I like writing fast code. Writing an open source, hobbyist path tracer seems like a good fit for me.

Why you should use CRender
* Fast, very fast. Unlike (some) other renderers, CRender doesn't have any model modification (barring transforms). It's trades off features (which I don't think are required, for performance)
* Open source, open source is good. 
* Fast feedback to users, I try my hardest to listen to all users complaints / suggestions to make this as easy to use as possible :smile:

Of course depending on your situation. Other path tracers might be the way to go, this project does not attempt to compete with them. Instead as mentioned before. Merely give users an option to trade off features for performance.

<p align="right">(<a href="#top">back to top</a>)</p>

### Built With

Thanks to the following open source projects for making this possible:
<br/>
*Note: This is not an exhaustive list*

* [tinygltf](https://github.com/syoyo/tinygltf)
* [imgui](https://github.com/ocornut/imgui)
* [glad](https://github.com/Dav1dde/glad)
* [stb](https://github.com/nothings/stb)
* [cmake](https://cmake.org/)
* [embree](https://www.embree.org/)

<p align="right">(<a href="#top">back to top</a>)</p>



<!-- GETTING STARTED -->
## Getting Started

Todo: Better getting started guide

To get CRender running locally, head to releases and download the latest release.

### Linux/Unix
Get intel embree, oneapi, and Threaded Building blocks.
Install gcc & g++ (or whatever compiler you want), whatever C library you use is (libc6-dev on Debian & Ubuntu), make, and cmake.

On linux you can just install `build-essential` on Debian & Ubuntu derivatives, `base-devel` for Arch & derivatives.
If you are on Debian/Ubuntu derivatives you also need to install `libxi-dev build-essential xorg libglfw3 libglfw3-dev libxinerama-dev libxcursor-dev`. For Arch & derivatives you need to install their equivilents.

Then 
1) Clone this repository with `git clone https://github.com/LegendWasTaken/CRender.git`
2) If you're compiling from the command line
    1) `cd CRender` (Or what the directory is called)
    2) `source /opt/intel/oneapi/setvars.sh` to set the variables from embree, intel TBB, and such.
    3) `cmake .` To configure the build
    4) `cmake --build .` to actually compile it
    4) `./crebon` You should find your executable in the folder you ran these commands in
3) If you're using an IDE with cmake integration, just build the project as you normally would.
## Usage

Todo: Better usage guide

<p align="right">(<a href="#top">back to top</a>)</p>

<!-- ROADMAP -->
## Roadmap

v4.0.0 (Rewrite caused version to bump to 3)
- [ ] UI (3.0.0)
- [ ] Image exporting (3.0.0)
- [ ] Denoising (3.1.0)
- [ ] Obj support (3.2.0)
- [ ] Low res preview mode (3.3.0)

v5.0.0
- [ ] Multi model support (4.1.0)
- [ ] Headless mode (4.2.0)
- [ ] Scene saving / loading (4.3.0)
- [ ] Render dump (stop a render midway) (4.4.0)

v6.0.0
- [ ] Vulkan RTX integration

v7.0.0 
- [ ] Native minecraft world loading

v8.0.0
- [ ] React based UI

See the [open enhancement issues](https://github.com/LegendWasTaken/CRender/labels/enhancement) for a full list of proposed features.

<p align="right">(<a href="#top">back to top</a>)</p>



<!-- CONTRIBUTING -->
## Contributing

Contributions are what make the open source community such an amazing place to learn, inspire, and create. Any contributions you make are **greatly appreciated**.

If you have a suggestion that would make this better, please fork the repo and create a pull request. You can also simply open an issue with the tag "enhancement".
Don't forget to give the project a star! Thanks again!

1. Fork the Project
2. Create your Feature Branch (`git checkout -b <yourname>/AmazingFeature`)
3. Commit your Changes (`git commit -m '<yourname>:Add some AmazingFeature'`) (Please be sure to squash your commits)
4. Push to the Branch (`git push origin <yourname>/AmazingFeature`)
5. Open a Pull Request

<p align="right">(<a href="#top">back to top</a>)</p>

<!-- LICENSE -->
## License

Distributed under the MIT License. See `LICENSE.txt` for more information.

<p align="right">(<a href="#top">back to top</a>)</p>



<!-- CONTACT -->
## Contact

Use the [community discord](https://discord.gg/ZjrRyKXpWg) for any questions or concerns that you may have (Replies will generally be faster there).
<br/>
(Also, if you submit bugs / features via discord, you'll get paid a $5 bounty for your contribution!)

<p align="right">(<a href="#top">back to top</a>)</p>