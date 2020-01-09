# VST3 plug-in generator

A cmake script for generating VST 3 plug-ins using cmake's script mode.

> See also: <https://cmake.org/cmake/help/v3.16/manual/cmake-language.7.html#scripts>

## Usage

Execute on command line:

```
$ git clone https://stash.steinberg-intra.net/scm/~rhansen/vst3plugingenerator.git

$ cd vst3plugingenerator
$ cmake -P GenerateVST3Plugin.cmake
```

The script will output all variables and its current values. In order to adapt variables, edit 
```
vst3plugingenerator/cmake/modules/SMTG_VendorSepcifics.cmake
``` 
file to your needs.

After the script has finished you will find a 
```
vst3plugingenerator/myplugin
```
folder in the directory the script was executed in.
