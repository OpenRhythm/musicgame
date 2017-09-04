# OpenRhythm
[![Discord](https://img.shields.io/discord/277133146007404544.svg)](https://discord.gg/BgDgs5)

OpenRhythm currently depends on the following libs:
* CMake
* SDL2
* Freetype - I may switch to using stb_freetype haven't decided quite yet.
* GLM
* STB (included in extern/stb)
* Gettext

### Submodules
This project contains submodules. There are two ways to handle this.

To clone the repo and all submodules do:
```
git clone --recursive https://github.com/OpenRhythm/OpenRhythm
```

Otherwise if you have just cloned the repo you will need to run these to clone the submodules:
```
git submodule init
git submodule update
```

## Compiling and packaging
You will need a compiler that supports C++14

* Makedepends : dependencies for compiling but not for executing :
    - CMake (3.0 minimum)
    - GLM   (Graphics, header only)

* Dependencies : for compiling and executing :
    - Freetype2
    - Gettext
    - OggVorbis
    - SDL2
    - cubeb


#### Windows
The following instructions are using "Build Tools for Visual Studio 2017" 

Download the pre-built dependancies and make sure they are in the extern folder.

https://drive.google.com/file/d/0BwN4qYrZTBNbaW1Tb3pycm1aTlE/view?usp=sharing
This are x64 release builds done with VS 2015

Then run win-release-build.bat



#### Unix: (MacOS/Linux)
GCC-4.9+ or Clang-3.4+ (MacOS: download Xcode Command Line Utils)

* Linux
    * Debian / Ubuntu / Linux Mint
    ```
    sudo apt-get install cmake sdl2-dev libfreetype-dev libglm-dev
    ```
    * ArchLinux / Manjaro / …
    ```
    sudo pacman -S cmake sdl2 freetype2 glm
    ```

Then build the code
```
./release-build.sh 
# or
./release-build.sh --verbose
```
