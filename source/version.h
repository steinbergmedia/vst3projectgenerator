#pragma once

#include "version_buildnumber.h"

#define MAJOR_VERSION_STR "0"
#define MAJOR_VERSION_INT 0

#define SUB_VERSION_STR "1"
#define SUB_VERSION_INT 1

#define RELEASE_NUMBER_STR "0"
#define RELEASE_NUMBER_INT 0

#define BUILD_NUMBER_INT TAG_BUILDNR
#define BUILD_NUMBER_STR TAG_BUILDNR_STR

#define BUILD_STRING MAJOR_VERSION_STR "." SUB_VERSION_STR "." RELEASE_NUMBER_STR
