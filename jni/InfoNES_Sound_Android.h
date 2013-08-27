/*===================================================================*/
/*                                                                   */
/*  InfoNES_Sound_Android.h : The function which depends on a system  */
/*                           (for Android)                           */
/*                                                                   */
/*  2013/05/28  InfoNES Project                                      */
/*                                                                   */
/*===================================================================*/
#ifndef _INFONES_SOUND_ANDROID_H_
#define _INFONES_SOUND_ANDROID_H_
/*-------------------------------------------------------------------*/
/*  Include files                                                    */
/*-------------------------------------------------------------------*/
#include <jni.h>
#include <opensl_helpers.h>
#include <stdio.h>

#include "InfoNES.h"
#include "InfoNES_System.h"

//#include "InfoNES_Sound_Win.h"
#ifdef NES_SOUND_SUPPORT
#include "InfoNES_pAPU.h"
#endif

#define NDKSP_MIN_VOLUME 0.0f
#define NDKSP_MAX_VOLUME 1.0f
#define MIN_VOL_MILLIBEL -2048 // -500

typedef struct tag_ResourceBuffer {
        void* buf;
        int size;
        struct tag_ResourceBuffer *next;
}ResourceBuffer, *ResourceBufferPtr;

typedef struct tag_BufferQueue{
		SLObjectItf playerObj;
        SLBufferQueueItf queue;
        SLPlayItf play;
        SLVolumeItf volume;
        BOOL playing;
}BufferQueue, *BufferQueuePtr;


typedef struct tag_OpenSLSoundPool{
	int maxStreams;
	int SampleNum;
	SLuint32 samplesPerSec; // 11025 / 22050 / 44100
	SLuint32 bitsPerSampe; // 8 bit / 16 bit
	SLuint32 bitrate;

	// device specific min and max volumes
	SLmillibel minVolume;
	SLmillibel maxVolume;

	// engine interfaces
	SLObjectItf engineObject;
	SLEngineItf engineEngine;

	// output mix interfaces
	SLObjectItf outputMixObject;

	// vector for BufferQueues (one for each channel)
	BufferQueue *bufferQueuesArray;

	// vector for samples
	BYTE *sampleBuffer;
	int sampleLength;

	int queueSize;
	int curStreamId;
	BYTE *ptrSampleBuffer; // 计算PCM数据播放队列个数， 如果高于最大队列容量就暂停向队列添加PCM数据
}OpenSLSoundPool, *OpenSLSoundPoolPtr;

OpenSLSoundPool *InfoNES_soundCreate(int nMaxStreams, SLuint32 nSamplingRate, SLuint32 nBitrate);
void InfoNES_soundDestroyEngine(OpenSLSoundPool *pSoundPool);
void InfoNES_soundSetMute(OpenSLSoundPool *pSoundPool, BOOL bMute);
BOOL InfoNES_soundGetMute(OpenSLSoundPool *pSoundPool);
/*************************************************
 *
 *   volume : range in [0.0, 1.0], it is a normalized value
 *
 *************************************************/
void InfoNES_soundSetVolume(OpenSLSoundPool *pSoundPool, float volume );

#if 1
BOOL InfoNES_soundPlay(OpenSLSoundPool *pSoundPool, char* buffer, int length);
#else
int InfoNES_soundLoad(OpenSLSoundPool *pSoundPool, char* buffer, int length);
int InfoNES_soundPlay(OpenSLSoundPool *pSoundPool, int sampleId, float volume);
#endif

#endif // _INFONES_SOUND_ANDROID_H_
