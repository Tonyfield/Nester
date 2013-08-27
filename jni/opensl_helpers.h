/*
 * opensl_helpers.h
 *
 *  Created on: 30.9.2011
 *      Author: Janne Kytomaki
 */

#ifndef OPENSL_HELPERS_H_
#define OPENSL_HELPERS_H_

#include <android/log.h>
#include <stdlib.h>
#include "SLES/OpenSLES.h"
#include "SLES/OpenSLES_Android.h"


#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "NES_Sound"
//#define LOGI(...)  outputLog(__FILE__, __LINE__, __VA_ARGS__)
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGV(...)  __android_log_print(ANDROID_LOG_VERBOSE,LOG_TAG,__VA_ARGS__)


static const char *result_strings[] = {
    "SUCCESS",
    "PRECONDITIONS_VIOLATED",
    "PARAMETER_INVALID",
    "MEMORY_FAILURE",
    "RESOURCE_ERROR",
    "RESOURCE_LOST",
    "IO_ERROR",
    "BUFFER_INSUFFICIENT",
    "CONTENT_CORRUPTED",
    "CONTENT_UNSUPPORTED",
    "CONTENT_NOT_FOUND",
    "PERMISSION_DENIED",
    "FEATURE_UNSUPPORTED",
    "INTERNAL_ERROR",
    "UNKNOWN_ERROR",
    "OPERATION_ABORTED",
    "CONTROL_LOST"
};

// Convert result to string; FIXME should move to common test library



// Same as above but automatically adds the source code line number

#define check(result) check2(result, __FILE__, __LINE__)


// FIXME: GCC compiles OK, but Eclipse CDT displays errors for OpenSL slCreateEngine references.
// Redefining the function here seems to remove the error. Try commenting the declaration out
// after few CDT updates.

SLresult slCreateEngine(
        SLObjectItf             *pEngine,
        SLuint32                numOptions,
        const SLEngineOption    *pEngineOptions,
        SLuint32                numInterfaces,
        const SLInterfaceID     *pInterfaceIds,
        const SLboolean         * pInterfaceRequired
);

#endif /* OPENSL_HELPERS_H_ */
