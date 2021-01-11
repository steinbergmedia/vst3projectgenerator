# VST 3 Project Generator

## Introduction

With this little application (Win / macOS) you can easily create a new VST 3 plug-in project by simply entering some parameters into a GUI.

## How to clone and build

```Example
git clone https://github.com/steinbergmedia/vst3_project_generator.git
mkdir build
cd build
cmake -GXcode ../vst3_project_generator
cmake --build .
```

## Project Structure

The VST 3 Project Generator repository contains an app that uses VSTGUI and a cmake script that the app calls.