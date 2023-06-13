# VST3 Project Generator Script

A cmake script for generating VST 3 plug-in projects using cmake's script mode.

> See also: <https://cmake.org/cmake/help/v3.16/manual/cmake-language.7.html#scripts>

## Usage

First, edit the variables in `vst3projectgenerator/script/cmake/modules/SMTG_VendorSpecifics.cmake` to your needs.  

Then run:

```
cmake -P GenerateVST3Plugin.cmake
```
This assumes `vst3projectgenerator` and `vst3sdk` have been cloned side-by-side.  
The script will output all variables and their current values.  
You will find the generated plugin in the `vst3projectgenerator/script/output/` directory.  
You may want to add the following parameters:
 * `-DSMTG_VST3_SDK_SOURCE_DIR_CLI=""` to set a different path to the vst3sdk
 * `-DSMTG_GENERATOR_OUTPUT_DIRECTORY_CLI=""` to set a different output dir

## Using command line parameters
Should you instead want to configure everything through the cmake command:
```console
cmake -DSMTG_VENDOR_NAME_CLI="Test Company" -DSMTG_VENDOR_HOMEPAGE_CLI="https://www.testcompany.com" -DSMTG_VENDOR_EMAIL_CLI="tester@testcompany.com" -DSMTG_PLUGIN_NAME_CLI="TestPlugin" -DSMTG_PREFIX_FOR_FILENAMES_CLI="Test_" -DSMTG_PLUGIN_IDENTIFIER_CLI="com.testcompany.vst3.testplugin" -DSMTG_CMAKE_PROJECT_NAME_CLI="TestPlugin" -DSMTG_VENDOR_NAMESPACE_CLI="TestCompany" -DSMTG_PLUGIN_CLASS_NAME_CLI="CTestPlugin" -DSMTG_PLUGIN_CATEGORY_CLI="TestCategory" -DSMTG_PLUGIN_BUNDLE_NAME_CLI="testpluginbundle" -DSMTG_VST3_SDK_SOURCE_DIR_CLI="" -DSMTG_GENERATOR_OUTPUT_DIRECTORY_CLI="" -P GenerateVST3Plugin.cmake
```
