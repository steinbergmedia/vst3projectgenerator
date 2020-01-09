# VST3 plug-in generator

A cmake script for generating VST 3 plug-ins using cmake's script mode.

> See also: <https://cmake.org/cmake/help/v3.16/manual/cmake-language.7.html#scripts>

## Usage

```
$ git clone <url>/vst3plugingenerator.git

$ cd vst3plugingenerator
$ cmake -P GenerateVST3Plugin.cmake
```

Adapt variables by editing <tt>cmake/modules/SMTG_VendorSepcifics.cmake</tt> file.
