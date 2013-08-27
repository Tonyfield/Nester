/*    filename : infones_system_android.h       */

#ifndef _INFONES_SYSTEM_ANDROID_H_
#define _INFONES_SYSTEM_ANDROID_H_

#include <jni.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <android/log.h>
#include "InfoNes_System.h"
#include "InfoNES_pAPU.h"

#if ( pAPU_QUALITY > 1 ) 
#define ds_SAMPLERATE 		  44100
#define rec_freq            735
#else
#error DFSFA
#define ds_SAMPLERATE 		  22050
#define rec_freq            367
#endif


#define TAG "NativeNes"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
//#define LOGV(...) outputLog(__FILE__, __LINE__, __VA_ARGS__)
//#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

typedef struct{
	JavaVM *pJavaVM;
	jobject objNesSimu;// java class NesSimu
	jobject objBitmap;// java class NesSimu
	// Classes.
	jclass classNesSimu;
	jclass classBitmap;
	// Methods.
	jmethodID methodUpdateView; // NesSimu.UpdateView(Bitmap bm)
	pthread_t threadNes;
	sigset_t  sigset;
	NES_SEM_TYPE sem;
	// Nes ROM filename
}NES_CONTEXT;

NES_CONTEXT *InfoNES_GetContext(void);
BOOL InfoNES_StartThread(JNIEnv * pEnv, NES_CONTEXT *pNesContext);
BOOL InfoNES_StopThread(JNIEnv * pEnv, NES_CONTEXT *pNesContext);
BOOL InfoNES_LoadRomFile(JNIEnv * pEnv, NES_CONTEXT *pNesContext, const char *pFilename);
void InfoNES_SetMute( BOOL bMute );
void InfoNES_SetVolume( jfloat vol );
/*===================================================================*/
/*                                                                   */
/*             InfoNES_SetPad1State() : Set a joypad 1 state         */
/*                                                                   */
/*===================================================================*/
void InfoNES_SetPad1State( DWORD pdwPadMask1, DWORD pdwPad1 );
/*===================================================================*/
/*                                                                   */
/*             InfoNES_SetPad1State() : Set a joypad 2 state         */
/*                                                                   */
/*===================================================================*/
void InfoNES_SetPad2State( DWORD pdwPadMask2, DWORD pdwPad2 );

int InfoNES_PrintErrInfo(const char *errInfo,...);
const char *InfoNES_GetErrInfo();

#endif // _INFONES_SYSTEM_ANDROID_H_
