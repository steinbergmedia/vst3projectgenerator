#pragma once

#include "version_buildnumber.h"

#define MAJOR_VERSION_STR "2020"
#define MAJOR_VERSION_INT 2020

#define SUB_VERSION_STR "6"
#define SUB_VERSION_INT 6

#define RELEASE_NUMBER_STR "0"
#define RELEASE_NUMBER_INT 0

#define BUILD_NUMBER_STR TAG_BUILDNR_STR
#define BUILD_NUMBER_INT TAG_BUILDNR

#define BUILD_STRING MAJOR_VERSION_STR "." SUB_VERSION_STR "." RELEASE_NUMBER_STR "." BUILD_NUMBER_STR

#ifdef __plist_preprocessor__
#define CAT_PRIVATE_DONT_USE(a,b)			a ## b
#define CAT(a,b)							CAT_PRIVATE_DONT_USE(a,b)
#define MAKE_BUNDLE_VERSION(m,s,r,b)		CAT(m, CAT(., CAT(s,CAT(.,CAT(r,CAT(.,b))))))
#define BUNDLE_VERSION MAKE_BUNDLE_VERSION(MAJOR_VERSION_INT,SUB_VERSION_INT,RELEASE_NUMBER_INT,BUILD_NUMBER_INT)
#endif
