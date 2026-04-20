# VST3 Project Generator Script

A cmake script for generating VST 3 plug-in projects using cmake's script mode.

> See also: <https://cmake.org/cmake/help/v3.16/manual/cmake-language.7.html#scripts>



## Usage

### basic

```console
cmake -P GenerateVST3Plugin.cmake
```

This uses the variables in `vst3projectgenerator/script/cmake/modules/SMTG_VendorSpecifics.cmake` and generates the project inside the `vst3projectgenerator/script/output/` directory.  

### advanced

Copy and modify the single command below to suit your needs, then run it:
#### linux / macOS
```console
cmake \
    -DSMTG_VENDOR_NAME_CLI="Test Company" \
    -DSMTG_VENDOR_HOMEPAGE_CLI="https://www.testcompany.com" \
    -DSMTG_VENDOR_EMAIL_CLI="tester@testcompany.com" \
    -DSMTG_PLUGIN_NAME_CLI="TestPlugin" \
    -DSMTG_PREFIX_FOR_FILENAMES_CLI="Test_" \
    -DSMTG_PLUGIN_IDENTIFIER_CLI="com.testcompany.vst3.testplugin" \
    -DSMTG_CMAKE_PROJECT_NAME_CLI="TestPlugin" \
    -DSMTG_VENDOR_NAMESPACE_CLI="TestCompany" \
    -DSMTG_PLUGIN_CLASS_NAME_CLI="CTestPlugin" \
    -DSMTG_PLUGIN_CATEGORY_CLI="TestCategory" \
    -DSMTG_PLUGIN_BUNDLE_NAME_CLI="testpluginbundle" \
    -DSMTG_VST3_SDK_SOURCE_DIR_CLI="path_to/vst3sdk" \
    -DSMTG_GENERATOR_OUTPUT_DIRECTORY_CLI="path_to/plugin\ projects\ dir" \
    -DSMTG_ENABLE_VSTGUI_SUPPORT_CLI=ON \
    -P GenerateVST3Plugin.cmake
```
#### windows
```console
cmake ^
    -DSMTG_VENDOR_NAME_CLI="Test Company" ^
    -DSMTG_VENDOR_HOMEPAGE_CLI="https://www.testcompany.com" ^
    -DSMTG_VENDOR_EMAIL_CLI="tester@testcompany.com" ^
    -DSMTG_PLUGIN_NAME_CLI="TestPlugin" ^
    -DSMTG_PREFIX_FOR_FILENAMES_CLI="Test_" ^
    -DSMTG_PLUGIN_IDENTIFIER_CLI="com.testcompany.vst3.testplugin" ^
    -DSMTG_CMAKE_PROJECT_NAME_CLI="TestPlugin" ^
    -DSMTG_VENDOR_NAMESPACE_CLI="TestCompany" ^
    -DSMTG_PLUGIN_CLASS_NAME_CLI="CTestPlugin" ^
    -DSMTG_PLUGIN_CATEGORY_CLI="TestCategory" ^
    -DSMTG_PLUGIN_BUNDLE_NAME_CLI="testpluginbundle" ^
    -DSMTG_VST3_SDK_SOURCE_DIR_CLI="path_to\vst3sdk" ^
    -DSMTG_GENERATOR_OUTPUT_DIRECTORY_CLI="path_to\plugin^ projects^ dir" ^
    -DSMTG_ENABLE_VSTGUI_SUPPORT_CLI=ON ^
    -P GenerateVST3Plugin.cmake
```

This is the recommended way to manually generate a plugin project as it does not require modifying any file of the vst3projectgenerator.  
Relative paths may be specified using `../` or `..\`, spaces need to be escaped: `\ ` or `^ `.
