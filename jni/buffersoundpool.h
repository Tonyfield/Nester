/*===================================================================*/
/*                                                                   */
/*  BufferSoundPool.h :  sound play interface for cross platform  */
/*                           (for WIN32 / Android)                           */
/*                                                                   */
/*  2013/05/28  InfoNES Project                                      */
/*                                                                   */
/*===================================================================*/
#ifndef _BufferSound__ANDROID_H_
#define _BufferSound__ANDROID_H_
/*-------------------------------------------------------------------*/
/*  Include files                                                    */
/*-------------------------------------------------------------------*/
#ifdef WIN32
  #include <windows.h>
  #include <mmsystem.h>
  #include "dsound.h"
#elif defined ANDROID
  #include <jni.h>
  #include <opensl_helpers.h>
#else
  #error not support other platform
#endif

#include <stdio.h>
#include "InfoNES_Types.h"

#define MAX_STREAMS_NUM     8

#ifdef WIN32


typedef struct tag_DSoundBuffer{
	LPDIRECTSOUNDBUFFER  lpdsb;
	BOOL mute;
	BOOL playing;
	float volume;
	UINT32 sampleOffset; // ����PCM��ݲ��Ŷ��и��� ��������������������ͣ��������PCM���
}DSoundBuffer, *DSoundBufferPtr;


#elif defined ANDROID

#define NDKSP_MIN_VOLUME 0.0f
#define NDKSP_MAX_VOLUME 1.0f
#define MIN_VOL_MILLIBEL -2048 // -500
/*
typedef struct tag_ResourceBuffer {
	void* buf;
	int size;
	struct tag_ResourceBuffer *next;
}ResourceBuffer, *ResourceBufferPtr;
*/
typedef struct tag_BufferQueue{
	SLObjectItf playerObj;
	SLBufferQueueItf queue;
	SLPlayItf play;
	SLVolumeItf volume;
	BOOL playing;
	UINT32 sampleOffset; // ����PCM��ݲ��Ŷ��и��� ��������������������ͣ��������PCM���
}BufferQueue, *BufferQueuePtr;
#else
#error not support other platform
#endif

typedef struct tag_OpenSLSoundPool{
	UINT16 maxStreams;
	int looping;
	int curStreamId;
	// vector for samples
	BYTE *sampleBuffer;
	int sampleLength;
//	BOOL bMute;
//	double Volume;

#ifdef WIN32
	UINT32 samplesPerSec; // 11025 / 22050 / 44100
	UINT32 bitsPerSampe; // 8 bit / 16 bit
//	UINT32 bitrate;
//	INT16  minVolume;
//	INT16  maxVolume;

	LPDIRECTSOUND lpdirsnd;

	DSoundBuffer *bufferArray;

	/* Used for management of each sound channel  */
//	BYTE  *sound[ds_NUMCHANNELS];
//	DWORD   len[ds_NUMCHANNELS];

#elif defined __linux__

	SLuint32 samplesPerSec; // 11025 / 22050 / 44100
	SLuint32 bitsPerSampe; // 8 bit / 16 bit
//	SLuint32 bitrate;

	// device specific min and max volumes
	SLmillibel minVolume;
	SLmillibel maxVolume;

	// engine interfaces
	SLObjectItf engineObject;
	SLEngineItf engineEngine;

	// output mix interfaces
	SLObjectItf outputMixObject;

	// vector for BufferQueues (one for each channel)
	BufferQueue *bufferArray;
//	UINT32 sampleOffset; // ����PCM��ݲ��Ŷ��и��� ��������������������ͣ��������PCM���


#else
#error not support other platform
#endif
}BufferSoundPool, *BufferSoundPoolPtr;

BufferSoundPool *BufferSound_Create(
#ifdef WIN32
		HWND hWnd,
#endif
		UINT16 nMaxStreams,
		UINT32 samplesPerSec,
		UINT32 bitsPerSampe,
		UINT16 preSoundLength,
		BOOL looping
		);
void BufferSound_Destroy(BufferSoundPool *pSoundPool);
BOOL BufferSound_SetMute(BufferSoundPool *pSoundPool, BOOL bMute);
BOOL BufferSound_GetMute(BufferSoundPool *pSoundPool);
void BufferSound_SetVolume(BufferSoundPool *pSoundPool, float volume );

/*************************************************
 *
 *   volume : range in [0.0, 1.0], it is a normalized value
 *
 *************************************************/
void BufferSound_SetVolume(BufferSoundPool *pSoundPool, float volume );

#if 1
BOOL BufferSound_Play(BufferSoundPool *pSoundPool, BYTE* buffer, int length);
#else
int BufferSound_Load(BufferSoundPool *pSoundPool, BYTE* buffer, int length);
int BufferSound_Play(BufferSoundPool *pSoundPool, int sampleId, float volume);
#endif

#endif // _BufferSound__ANDROID_H_
