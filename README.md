# VST 3 Project Generator

## Introduction

With this little application (Win/macOS) you can easily create a new **VST 3** plug-in project by simply entering some parameters into a GUI.

## How to clone and build

```Example
git clone https://github.com/steinbergmedia/vst3projectgenerator.git
mkdir build
cd build
cmake -GXcode ../vst3projectgenerator
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
