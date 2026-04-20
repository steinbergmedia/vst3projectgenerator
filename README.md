# VST 3 Project Generator

-----------------------------------------------------------------------------------------------------------------------
## - Features added -

* Project now builds with a .vscode folder with tasks.json to clean+compile+build and launch.json to debug 
* Sets up a bypass parameter and demonstrates the complete logic for adding more
* Manages setting bus arangements manually and exposes params not available at initiate
* Includes the following tools, *should* compile properly:
    * FFTW64 (https://www.fftw.org/)
    * DSP Filters (details: https://github.com/vinniefalco/DSPFilters)
* Includes basic C++ .gitignore which prevents the LicenseSpring VST SDK from pushing as its OS/configuration dependent. Simply download the corresponding SDK from https://docs.licensespring.com/sdks/cpp if the one here isn't correct. In my code, it is LicenseSpringCppSDK-v7.41.0-VC142.zip.
* LicenseSpring tools can be found in the LicenseSpring-Tutorials folder and implimenting them is extremely straightforwards (details: https://github.com/kbrandon17/LicenseSpring-SampleProjects)
* Editor is overrided to give control
* EditorDelegate allows for deep UI control and allows listeners to be applied to UI objects created from editor.uidesc
-----------------------------------------------------------------------------------------------------------------------


## How to Build and Use the project generator
Either download or clone and cd into the repo then create a build folder. From in the build folder:
```
cmake -G "Visual Studio 17 2022" ..
cmake --build . --config Release  
```
Then the project generator will be in ROOT/build/Release/VST3_Project_Generator/VST3_Project_Generator.exe

From here it acts exactly like the old generator and even remembers your preferences/paths. Takes a lot longer to build but its worth it.  

Definitely unlink or remove all the libraries you aren't using so you dont compile a ton of bloatware. If you really don't want something in the project, just folder from the \script\cmake\templates folder in the Project Generator, and then edit script\cmake\templates\vst3plugin_folder\CMakeLists.txt.in to remove the linking to the associated libary, then rebuild the Project Generator.


## LicenseSpring Notes

To use LicenseSpring features within a project (not implimented in the template), it takes some more setting up. I am compiling with MSVC with "-DCMAKE_TOOLCHAIN_FILE=path\to\msvc_toolchain.cmake" as I need the following libraries to complete an online node unlocking:

* libcurl         (https://curl.se/libcurl/)
* OpenSSL         (https://slproweb.com/products/Win32OpenSSL.html)
* yubico-piv-tool (https://github.com/Yubico/yubico-piv-tool/)

Simple way to add them to the toolchain is using vcpkg (https://github.com/microsoft/vcpkg), then when its added as a global executable:
```
vcpkg install curl openssl:x64-windows
vcpkg integrate install
```
You can now use.

find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)

to find the libraries in your CMakeLists.txt. 

yubico-piv-tool is a bit more tedious. You can download the windows binary directly (https://developers.yubico.com/yubico-piv-tool/Releases/) or follow the steps here (https://developers.yubico.com/yubico-piv-tool/).

-----------------------------------------------------------------------------------------------------------------------

## Original README
-----------------------------------------------------------------------------------------------------------------------
# VST 3 Project Generator


## Introduction


With this little application you can easily create a new **VST 3** plug-in project by simply entering some parameters into a GUI (Win/macOS).  
Linux users can generate a new plug-in project by [running the cmake script directly](https://github.com/steinbergmedia/vst3projectgenerator/tree/master/script).

## How to clone and build

### Example with macOS using XCode
```Example
git clone https://github.com/steinbergmedia/vst3projectgenerator.git
mkdir build
cd build
cmake -GXcode ../vst3projectgenerator
cmake --build .
```

### Example with Windows using Visual Studio 18 2026
```Example
git clone https://github.com/steinbergmedia/vst3projectgenerator.git
mkdir build
cd build
cmake.exe -G "Visual Studio 18 2026" -A x64 ../vst3projectgenerator
cmake --build .
```

## Project Structure

The **VST 3 Project Generator** repository contains an app that uses [VSTGUI](https://steinbergmedia.github.io/vst3_dev_portal/pages/What+is+the+VST+3+SDK/VSTGUI.html) and a cmake script that the app calls.

## Online Documentation

Please visit the [VST 3 Project Generator](https://steinbergmedia.github.io/vst3_dev_portal/pages/What+is+the+VST+3+SDK/Project+Generator.html).

## Usage guidelines

More details are found at [Usage guidelines](https://steinbergmedia.github.io/vst3_dev_portal/pages/VST+3+Licensing/Usage+guidelines.html).

## License

BSD style

    VST3ProjectGenerator LICENSE
    (c) Steinberg Media Technologies, All Rights Reserved

    Redistribution and use in source and binary forms, with or without modification,
    are permitted provided that the following conditions are met:

      * Redistributions of source code must retain the above copyright notice, 
        this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright notice,
        this list of conditions and the following disclaimer in the documentation 
        and/or other materials provided with the distribution.
      * Neither the name of the Steinberg Media Technologies nor the names of its
        contributors may be used to endorse or promote products derived from this 
        software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
    IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
    LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
    OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
    OF THE POSSIBILITY OF SUCH DAMAGE.
