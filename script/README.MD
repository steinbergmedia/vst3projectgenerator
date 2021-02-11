# VST3 Project Generator Script

A cmake script for generating VST 3 plug-in projects using cmake's script mode.

> See also: <https://cmake.org/cmake/help/v3.16/manual/cmake-language.7.html#scripts>

## Usage

Execute on command line:

```console
$ cmake -P GenerateVST3Plugin.cmake
```

The script will output all variables and its current values. In order to adapt variables, edit

```console
vst3plugingenerator/cmake/modules/SMTG_VendorSepcifics.cmake
```

file to your needs.

After the script has finished you will find a

```console
vst3plugingenerator/myplugin
```

folder in the directory the script was executed in.

## Using command line parameters
```console
cmake -DSMTG_VENDOR_NAME_CLI="Test Company" -DSMTG_VENDOR_HOMEPAGE_CLI="https://www.testcompany.com" -DSMTG_VENDOR_EMAIL_CLI="tester@testcompany.com" -DSMTG_PLUGIN_NAME_CLI="TestPlugin" -DSMTG_PREFIX_FOR_FILENAMES_CLI="Test_" -DSMTG_PLUGIN_IDENTIFIER_CLI="com.testcompany.vst3.testplugin" -DSMTG_CMAKE_PROJECT_NAME_CLI="TestPlugin" -DSMTG_VENDOR_NAMESPACE_CLI="TestCompany" -DSMTG_PLUGIN_CLASS_NAME_CLI="CTestPlugin" -DSMTG_PLUGIN_CATEGORY_CLI="TestCategory" -DSMTG_PLUGIN_BUNDLE_NAME_CLI="testpluginbundle" -P GenerateVST3Plugin.cmake
```
