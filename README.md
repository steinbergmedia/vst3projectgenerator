# VST 3 Project Generator

## How to clone and build

```
git clone https://github.com/steinbergmedia/vst3_project_generator.git
mkdir build
cd build
cmake -GXcode ../vst3_project_generator
cmake --build .
```

## Project Structure

The VST 3 Project Generator repository contains an app which uses VSTGUI and a cmake script, which is invoked by the app.