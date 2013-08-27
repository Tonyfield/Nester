/*===================================================================*/
/*                                                                   */
/*  Sound_play.cpp : The function which depends on a system  */
/*                           (for Android)                           */
/*                                                                   */
/*  2013/05/28  InfoNES Project                                      */
/*                                                                   */
/*===================================================================*/

/*-------------------------------------------------------------------*/
/*  Include files                                                    */
/*-------------------------------------------------------------------*/
#include "buffersoundpool.h"

BOOL BufferSound_CreateEngine(
							  BufferSoundPool *pSoundPool
#ifdef WIN32
							  ,HWND hWnd
#endif
);
BOOL BufferSound_CreateBuffer(BufferSoundPool *pSoundPool);

#ifdef WIN32 
BOOL BufferSound_Start(BufferSoundPool *pSoundPool);
void BufferSound_Stop(BufferSoundPool *pSoundPool);

#endif
#if defined __linux__
void staticBqPlayerCallback(SLBufferQueueItf bq, void *context);
#endif

#ifdef __linux__
static const char *result_to_string(SLresult result)
{
    static char buffer[32];
    if ( /* result >= 0 && */ result < sizeof(result_strings) / sizeof(result_strings[0]))
        return result_strings[result];
    //TRACE1(buffer, "%d", (int) result);
    return buffer;
}

// Compare result against expected and exit suddenly if wrong

void check2(SLresult result, const char *fn, int line)
{
    if (SL_RESULT_SUCCESS != result) {
        //TRACE3("\r\n error %s at %s, line %d\n", result_to_string(result), fn, line);
        exit(EXIT_FAILURE);
    }
}
#endif
//BufferQueue queues[];

// vector for BufferQueues (one for each channel)
//std::vector<BufferQueue*>* bufferQueues;

// vector for samples
//std::vector<ResourceBuffer*>* samples;

/*    function : BufferSound_Create  创建缓冲音频播放对象
 *     parameter : 
 *     #ifdef WIN32
 *     		HWND hWnd,  WIN32 下DirectSound 需要HWND 参数
 *     #endif
 *     		UINT16 nMaxStreams,     最大音频通道数
 *     		UINT32 samplesPerSec,   每秒采样数，也作为缓冲音频长度基准
 *     		UINT32 bitsPerSampe,     每采样位数
 *     		UINT16 cacheSoundLength  需要缓冲音频长度，对于NES 模拟器，打开游戏应用音频就不断播放，需要缓冲75%
 *                                                                          对于断续音频播放数据来说，可以置 0
 */
BufferSoundPool *BufferSound_Create(
#ifdef WIN32
		HWND hWnd,
#endif
		UINT16 nMaxStreams,
		UINT32 samplesPerSec,
		UINT32 bitsPerSampe,
		UINT16 preSoundLength,
		BOOL looping
		)
{
	BufferSoundPool *pSoundPool = (BufferSoundPool*)malloc(sizeof(BufferSoundPool));

//	TRACE2("BufferSound_Create(%d,%d)", nMaxStreams, samplesPerSec);

	if(  0 == pSoundPool )
	{
		return pSoundPool;
	}
	pSoundPool->maxStreams = nMaxStreams;
	pSoundPool->bitsPerSampe = bitsPerSampe;
	pSoundPool->looping = looping;

#ifdef WIN32
	pSoundPool->samplesPerSec = samplesPerSec;

	pSoundPool->sampleLength = bitsPerSampe * samplesPerSec/8; // keep buffer size contains 1 sec sound
	pSoundPool->sampleBuffer = (BYTE *)malloc(10*pSoundPool->sampleLength);
	if( pSoundPool->sampleBuffer )
	{
		memset( pSoundPool->sampleBuffer, 0, pSoundPool->sampleLength );
	}

	pSoundPool->curStreamId = 0;
//	pSoundPool->minVolume = -1024;
//	pSoundPool->maxVolume = 0;
	
	if( FALSE == BufferSound_CreateEngine(pSoundPool, hWnd ) 
		|| FALSE == BufferSound_CreateBuffer(pSoundPool) )
	{
		BufferSound_Destroy(pSoundPool);
		return 0;
	}

	if( 0 < preSoundLength && preSoundLength < pSoundPool->sampleLength )
	{
		BufferSound_Play(pSoundPool, pSoundPool->sampleBuffer, preSoundLength );
	}


#elif defined __linux__
	pSoundPool->engineObject = NULL;
	pSoundPool->outputMixObject = NULL;

//	pSoundPool->loops = loops;
	pSoundPool->samplesPerSec = samplesPerSec * 1000;

	pSoundPool->bitsPerSampe = SL_PCMSAMPLEFORMAT_FIXED_8;
	pSoundPool->bitsPerSampe = bitsPerSampe;
	pSoundPool->bufferArray = (BufferQueue *)0;
	pSoundPool->sampleBuffer = (BYTE *)malloc(bitsPerSampe * samplesPerSec/8);
	pSoundPool->sampleLength = samplesPerSec; // keep buffer size contains 1 sec sound

	memset(pSoundPool->sampleBuffer, 0, pSoundPool->sampleLength);
//	pSoundPool->sampleOffset = 0;

//	pSoundPool->queueSize = 1 + samplesPerSec / rec_freq;
	pSoundPool->curStreamId = 0;
	pSoundPool->minVolume = SL_MILLIBEL_MIN;
	pSoundPool->maxVolume = 0;

	// see if OpenSL library is available
/*	void* handle = dlopen("libOpenSLES.so", RTLD_LAZY);
	if (handle == NULL){
		LOGI("OpenSLES not available");
		exit(EXIT_FAILURE);
	}
*/
//	TRACE0("Initializing");
	BufferSound_CreateEngine(pSoundPool);
	BufferSound_CreateBuffer(pSoundPool);

//	TRACE1("BufferSound_Create() %d", pSoundPool->sampleLength);
//	for( i=0 ; i<pSoundPool->queueSize *3/4 ; i++ )
	{
//		BufferSound_Play(pSoundPool, pSoundPool->sampleBuffer, pSoundPool->sampleLength / pSoundPool->queueSize );
	}
//	BufferSound_Play(pSoundPool, pSoundPool->sampleBuffer, pSoundPool->sampleLength * 3 / 4 );
	if( preSoundLength )
	{
		BufferSound_Play(pSoundPool, pSoundPool->sampleBuffer, preSoundLength );
	}

#else
#error not support other platform
#endif

	return pSoundPool;
}

BOOL BufferSound_CreateEngine(
							  BufferSoundPool *pSoundPool
#ifdef WIN32
							  ,HWND hWnd
#endif
)
{
#ifdef WIN32

	DWORD ret;

	if( 0 == pSoundPool )
	{
		return FALSE;
	}

	// init DirectSound
	ret = DirectSoundCreate(NULL, &pSoundPool->lpdirsnd, NULL);

	if ( DS_OK != ret )
	{
		return FALSE;
	}

	// set cooperative level
#if 1
	ret = pSoundPool->lpdirsnd->SetCooperativeLevel(hWnd, DSSCL_PRIORITY);
#else
	ret = pSoundPool->lpdirsnd->SetCooperativeLevel( hWnd, DSSCL_NORMAL );
#endif

	if ( DS_OK != ret )
	{
		return FALSE;
	}

	return TRUE;
	
#elif defined __linux__

	SLresult result;

	const SLInterfaceID engine_ids[] = {SL_IID_ENGINE};
	const SLboolean engine_req[] = {SL_BOOLEAN_TRUE};

	if( 0 == pSoundPool )
	{
		return FALSE;
	}
	result = slCreateEngine(&pSoundPool->engineObject, 0, NULL, 0, engine_ids, engine_req);
	check(result);

	// realize the engine
	result = (*pSoundPool->engineObject)->Realize(pSoundPool->engineObject, SL_BOOLEAN_FALSE);
	check(result);

	// get the engine interface, which is needed in order to create other objects
	result = (*pSoundPool->engineObject)->GetInterface(pSoundPool->engineObject, SL_IID_ENGINE, &pSoundPool->engineEngine);
	check(result);

	// create output mix, with environmental reverb specified as a non-required interface
	const SLInterfaceID ids[1] = {SL_IID_NULL};
	const SLboolean req[1] = {SL_BOOLEAN_FALSE};

	result = (*pSoundPool->engineEngine)->CreateOutputMix(pSoundPool->engineEngine, &pSoundPool->outputMixObject, 0, ids, req);
	check(result);

	// realize the output mix
	result = (*pSoundPool->outputMixObject)->Realize(pSoundPool->outputMixObject, SL_BOOLEAN_FALSE);
	check(result);

	return TRUE;
#endif

}

// create buffer queue audio player
BOOL BufferSound_CreateBuffer(BufferSoundPool *pSoundPool)
{
#ifdef WIN32
/*-------------------------------------------------------------------*/
/*  CreateBuffer() : Create IDirectSoundBuffer                       */
/*-------------------------------------------------------------------*/
	DSBUFFERDESC dsbdesc;
	PCMWAVEFORMAT pcmwf;
	HRESULT hr;
	int channel;
	DSoundBuffer* sb;

//	TRACE0("BufferSound_CreateBuffer");
	if( 0 == pSoundPool || 0 == pSoundPool->maxStreams )
	{
		return FALSE;
	}

	memset(&pcmwf, 0, sizeof(PCMWAVEFORMAT));
	pcmwf.wf.wFormatTag 		 = WAVE_FORMAT_PCM;
	pcmwf.wf.nChannels			 = 1; // ds_CHANSPERSAMPLE;
	pcmwf.wf.nSamplesPerSec  = pSoundPool->samplesPerSec;
	pcmwf.wf.nBlockAlign		 = 1 /*ds_CHANSPERSAMPLE*/ * pSoundPool->bitsPerSampe / 8;
	pcmwf.wf.nAvgBytesPerSec = pcmwf.wf.nSamplesPerSec * pcmwf.wf.nBlockAlign;
	pcmwf.wBitsPerSample		 = pSoundPool->bitsPerSampe ;

	memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize				= sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags 			= 0;
//	dsbdesc.dwBufferBytes = pSoundPool->len[channel]*pSoundPool->loops;
	dsbdesc.dwBufferBytes = pSoundPool->samplesPerSec;
	dsbdesc.lpwfxFormat 	= (LPWAVEFORMATEX)&pcmwf;

	pSoundPool->bufferArray = (DSoundBuffer *)malloc(pSoundPool->maxStreams*sizeof(DSoundBuffer));
	if( 0 == pSoundPool->bufferArray )
	{
		return FALSE;
	}
	sb = pSoundPool->bufferArray;
	for( channel = 0 ; channel < pSoundPool->maxStreams ; channel++, sb++ ) 
	{
		sb->mute = FALSE;
		sb->playing = FALSE;
		sb->volume = 0.7;
		sb->sampleOffset = 0;
		
		hr = pSoundPool->lpdirsnd->CreateSoundBuffer(&dsbdesc, &(sb->lpdsb), NULL);

		if ( DS_OK != hr )
		{
			return FALSE;
		}
		pSoundPool->curStreamId = channel;
		BufferSound_SetVolume(pSoundPool, sb->volume );
		BufferSound_Start( pSoundPool );
	}
	pSoundPool->curStreamId = 0;
	return TRUE;
	
#elif defined __linux__
	int i;
	SLresult result;
	BufferQueue* bq;

	if( 0 == pSoundPool || 0 == pSoundPool->maxStreams )
	{
		return FALSE;
	}

	// configure audio source
	SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {
		SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, // locatorType
		8  // 60               // numBuffers
	};
	//loc_bufq.numBuffers = pSoundPool->queueSize;
	SLDataFormat_PCM format_pcm = {
		SL_DATAFORMAT_PCM,         // formatType
		1,                         // numChannels
		pSoundPool->samplesPerSec,  // samplesPerSec, etc. SL_SAMPLINGRATE_44_1
		pSoundPool->bitsPerSampe,  // bitsPerSample, etc. SL_PCMSAMPLEFORMAT_FIXED_16
		pSoundPool->bitsPerSampe,  // containerSize
		SL_SPEAKER_FRONT_CENTER,   // channelMask
		SL_BYTEORDER_LITTLEENDIAN  // endianness
	};

	SLDataSource audioSrc = {&loc_bufq, &format_pcm};

	// create audio player
	const SLInterfaceID player_ids[] = {SL_IID_BUFFERQUEUE, SL_IID_PLAY, SL_IID_VOLUME};
	const SLboolean player_req[] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

	pSoundPool->bufferArray = (BufferQueue *)malloc(pSoundPool->maxStreams*sizeof(BufferQueue));
	if( 0 == pSoundPool->bufferArray )
	{
		return FALSE;
	}
	bq = pSoundPool->bufferArray;
	for (i = 0; i < pSoundPool->maxStreams; i++, bq++)
	{
		bq->playing = FALSE;
		bq->sampleOffset = 0;

		// configure audio sink
		SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, pSoundPool->outputMixObject};
		SLDataSink audioSink = {&loc_outmix, NULL};

		//SLVolumeItf volume;

		LOGI("Creating SLAndroidSimpleBufferQueueItf");

		result = (*pSoundPool->engineEngine)->CreateAudioPlayer(
				pSoundPool->engineEngine,
				&(bq->playerObj),
				&audioSrc,
				&audioSink,
				3, player_ids, player_req);
		check(result);

		// realize the player
		result = (*bq->playerObj)->Realize(bq->playerObj, SL_BOOLEAN_FALSE);
		check(result);

		// get the play interface
		result = (*bq->playerObj)->GetInterface(bq->playerObj, SL_IID_PLAY, &bq->play);
		check(result);

		// get the buffer queue interface
		result = (*bq->playerObj)->GetInterface(bq->playerObj, SL_IID_BUFFERQUEUE, &bq->queue);
		check(result);

		// register callback on the buffer queue
		result = (*bq->queue)->RegisterCallback(bq->queue, staticBqPlayerCallback, (void *)pSoundPool);
		check(result);

		result = (*bq->playerObj)->GetInterface(bq->playerObj, SL_IID_VOLUME, &bq->volume);
		check(result);

		if (i == 0){
			result = (*bq->volume)->GetMaxVolumeLevel(bq->volume, &pSoundPool->maxVolume);
			check(result);
		}

		result = (*bq->play)->SetPlayState(bq->play, SL_PLAYSTATE_PLAYING);
		check(result);

		//TRACE1("Created stream %i", i);
	}
//	LOGI("Created %i streams", pSoundPool->bufferQueues->size());
	LOGI("createBufferQueueAudioPlayer done");
	return TRUE;

#endif
}


void BufferSound_Destroy(BufferSoundPool *pSoundPool)
{
#ifdef WIN32

	DWORD hr;
	int channel;

//	TRACE0("CBufferSound_Destroy");

	if( 0 == pSoundPool )
	{
		return;
	}

	if( pSoundPool->bufferArray )
	{
		DSoundBuffer* sb = pSoundPool->bufferArray;
		for ( channel=0; channel < pSoundPool->maxStreams ; channel++, sb++ )
		{
			if ( sb->lpdsb != NULL )
			{
	//			DestroyBuffer( x );
				sb->lpdsb->Stop();
				hr = sb->lpdsb->Release();

				if (hr != DS_OK)
				{
					//channelnfoNES_MessageBox( "Release() Fachannelled." );
					exit(-1);
				}

	/*	
				sb->lpdsb = NULL;
				if (pSoundPool->sound[channel] != NULL)
				{
					free( pSoundPool->sound[channel]);
					pSoundPool->sound[channel] = 0;
				}
				*/
			}
		}
		free(pSoundPool->bufferArray);
		pSoundPool->bufferArray = 0;
		pSoundPool->maxStreams = 0;
	}

	
	if( pSoundPool->sampleBuffer )
	{
		free(pSoundPool->sampleBuffer );
		pSoundPool->sampleBuffer  = 0;
	}
	pSoundPool->lpdirsnd->Release();	/* Nothing to do, if it errors */
	free( pSoundPool );


#elif defined __linux__
	int i;
//	ResourceBuffer *prev_node, *curr_node;

	if( 0 == pSoundPool )
	{
		return;
	}
	// destroy all buffer queues
	if( pSoundPool->bufferArray )
	{
		BufferQueue* bq = pSoundPool->bufferArray;
		for (i = 0; i < pSoundPool->maxStreams; i++){
			(*bq[i].playerObj)->Destroy(bq[i].playerObj);
		}
		free(pSoundPool->bufferArray);
		pSoundPool->bufferArray = 0;
		pSoundPool->maxStreams = 0;
	}
	if( pSoundPool->sampleBuffer )
	{
		free(pSoundPool->sampleBuffer);
		pSoundPool->sampleBuffer = 0;
	}
	pSoundPool->sampleLength = 0;

	// destroy output mix object
	if (pSoundPool->outputMixObject != NULL)
	{
		(*pSoundPool->outputMixObject)->Destroy(pSoundPool->outputMixObject);
		pSoundPool->outputMixObject = NULL;
	}

	// destroy engine object, and invalidate all associated interfaces
	if (pSoundPool->engineObject != NULL)
	{
		(*pSoundPool->engineObject)->Destroy(pSoundPool->engineObject);
		pSoundPool->engineObject = NULL;
		pSoundPool->engineEngine = NULL;
	}
	free( pSoundPool );
#endif

}

/*-------------------------------------------------------------------*/
/*  Play() : Play Sound                                              */
/*-------------------------------------------------------------------*/
BOOL BufferSound_Start(BufferSoundPool *pSoundPool)
{
#ifdef WIN32
	HRESULT hr;
	DSoundBuffer* sb;

	if( (0 == pSoundPool) || (pSoundPool->curStreamId >= pSoundPool->maxStreams ) )
	{
		return FALSE;
	}
	sb = &pSoundPool->bufferArray[pSoundPool->curStreamId];

	hr = sb->lpdsb->Play( 0, 0, pSoundPool->looping == TRUE ? DSBPLAY_LOOPING : 0 );

	if ( hr != DS_OK )
	{
		return FALSE;
	}
	return TRUE;
#else
	return FALSE;
#endif
}

/*-------------------------------------------------------------------*/
/*  Stop() : Stop Sound                                              */
/*-------------------------------------------------------------------*/
void BufferSound_Stop(BufferSoundPool *pSoundPool)
{
#ifdef WIN32
	DSoundBuffer* sb;

	if( (0 == pSoundPool) || (pSoundPool->curStreamId >= pSoundPool->maxStreams ) )
	{
		return;
	}
	sb = &pSoundPool->bufferArray[pSoundPool->curStreamId];
	sb->lpdsb->Stop();
#else
#endif
}

#ifdef __linux__
// a static callback function called every time a buffer finishes playing,
// delegates to the instance function
void staticBqPlayerCallback(SLBufferQueueItf bq, void *context)
{
	int i;
	BufferSoundPool *pSoundPool = (BufferSoundPool *)context;
//	LOGI("staticBqPlayerCallback");
	// we might be already destroyed if release() has been called mid-playback
	if( pSoundPool )
	{
		BufferQueue* avail = NULL;

		for (i = 0; i < pSoundPool->maxStreams; i++)
		{
			if (pSoundPool->bufferArray[i].queue == bq)
			{
				pSoundPool->bufferArray[i].playing = FALSE;
				break;
			}
		}
	}
}
#endif

BOOL BufferSound_SetMute(BufferSoundPool *pSoundPool, BOOL bMute)
{
#ifdef WIN32
	DSoundBuffer* sb;
	if( (0 == pSoundPool) || (pSoundPool->curStreamId >= pSoundPool->maxStreams ) )
	{
		return FALSE;
	}
	sb = &pSoundPool->bufferArray[pSoundPool->curStreamId];
	sb->mute = bMute;
	BufferSound_SetVolume( pSoundPool, bMute? 0.0f : sb->volume );

#elif defined __linux__
	SLresult result;
	BufferQueue* bq;
	
	if( (0 == pSoundPool) || (pSoundPool->curStreamId >= pSoundPool->maxStreams ) )
	{
		return FALSE;
	}
	bq = &pSoundPool->bufferArray[pSoundPool->curStreamId];

	result = (*bq->volume)->SetMute(bq->volume, bMute);
	check(result);
#endif
}

BOOL BufferSound_GetMute(BufferSoundPool *pSoundPool)
{
#ifdef WIN32
	DSoundBuffer* sb;
	if( (0 == pSoundPool) || (pSoundPool->curStreamId >= pSoundPool->maxStreams ) )
	{
		return FALSE;
	}
	sb = &pSoundPool->bufferArray[pSoundPool->curStreamId];
	return sb->mute;
	
#elif defined __linux__

	SLresult result;
	SLboolean bMute;
	BufferQueue* bq;
	
	if( (0 == pSoundPool) || (pSoundPool->curStreamId >= pSoundPool->maxStreams ) )
	{
		return FALSE;
	}
	bq = &pSoundPool->bufferArray[pSoundPool->curStreamId];

	result = (*bq->volume)->GetMute(bq->volume, &bMute);
	check(result);
	return bMute;
#else
#error not support other platform
#endif
	
}


/*************************************************
 *
 *   volume : range in [0.0, 1.0], it is a normalized value
 *
 *************************************************/
void BufferSound_SetVolume(BufferSoundPool *pSoundPool, float volume )
{
#ifdef WIN32
	LONG  dsVolume;
	DSoundBuffer* sb;

//	TRACE1("\r\n BufferSound_SetVolume(..., %f)", volume );

	if( (0 == pSoundPool) || (pSoundPool->curStreamId >= pSoundPool->maxStreams ) )
	{
		return;
	}
	if( volume < 0.0)  {volume = 0.0;}
	if( volume > 1.0)  {volume = 1.0;}

	sb = &pSoundPool->bufferArray[pSoundPool->curStreamId];
	dsVolume = (DSBVOLUME_MAX - DSBVOLUME_MIN) * volume + DSBVOLUME_MIN;
	sb->lpdsb->SetVolume(dsVolume);

#elif defined __linux__

	SLresult result;
	BufferQueue* bq;

	if( (0 == pSoundPool) || (pSoundPool->curStreamId >= pSoundPool->maxStreams ) )
	{
		return;
	}
	bq = &pSoundPool->bufferArray[pSoundPool->curStreamId];

	// convert requested volume 0.0-1.0 to millibels
	// in range max(minVolume, MIN_VOL_MILLIBEL) - maxVolume
	SLmillibel minvol = pSoundPool->minVolume;
	if (minvol < MIN_VOL_MILLIBEL)
	{
		minvol = MIN_VOL_MILLIBEL;
	}
	if( volume < 0.0)  {volume = 0.0;}
	if( volume > 1.0)  {volume = 1.0;}


//	SLmillibel newVolume = ((minvol - pSoundPool->maxVolume) * (1.0 - volume)) + pSoundPool->maxVolume;
	SLmillibel newVolume = (pSoundPool->maxVolume - minvol) * volume + minvol;

	//LOGV("Vol min = %d, max = %d, vol = %f, new vol = %d", minvol, pSoundPool->maxVolume, volume, newVolume );

	//adjust volume for the buffer queue
	result = (*bq->volume)->SetVolumeLevel(bq->volume, newVolume);
	check(result);
#endif
}

#if 1
BOOL BufferSound_Play(BufferSoundPool *pSoundPool, BYTE* buffer, int length)
{
	int remaining = 0;

#ifdef WIN32
	LPVOID write1;
	DWORD length1;
	LPVOID write2;
	DWORD length2;
	HRESULT hr;
	DSoundBuffer *sb;
	DWORD length_to_wrtie;
	

	if( 0 == pSoundPool ||
			0 == pSoundPool->maxStreams ||
			0 == buffer ||
			0 == length)
	{
//		TRACE0("\r\n BufferSound_Play failed 1" );
		return FALSE;
	}

//	TRACE1("\r\n BufferSound_Play(..., length = %d)", length );
	/* Copying to sound data buffer */
	/*	FillBuffer( ch1 );  ************/

	sb = &pSoundPool->bufferArray[pSoundPool->curStreamId];

	BufferSound_Start( pSoundPool);
	while( length > 0 )
	{
		if( length > pSoundPool->samplesPerSec )
		{
			length_to_wrtie = pSoundPool->samplesPerSec;
		} else {
			length_to_wrtie = length;
		}
	/******************************************************************************/
		hr = sb->lpdsb->Lock( sb->sampleOffset, length_to_wrtie, &write1, &length1, &write2, &length2, 0/*DSBLOCK_FROMWRITECURSOR*/  );

//		TRACE2("\r\n BufferSound_Play() ====== write1 = %x, length1 = %d,", write1, length1 );
//		TRACE2(" write2 = %x, length2 = %d", write2, length2);

		if (hr == DSERR_BUFFERLOST)
		{
			sb->lpdsb->Restore();

			hr = sb->lpdsb->Lock( sb->sampleOffset, length_to_wrtie, &write1, &length1, &write2, &length2, 0/*DSBLOCK_FROMWRITECURSOR*/ );
//			TRACE1("\r\n BufferSound_Play warning, hr = %x", hr );
//			TRACE2("\r\n BufferSound_Play() ------ write1 = %x, length1 = %d,", write1, length1 );
//			TRACE2(" write2 = %x, length2 = %d", write2, length2);
		}

		if (hr != DS_OK)
		{
//			TRACE1("\r\n BufferSound_Play failed 2, hr = %x", hr );
			return FALSE;
		}

		memcpy( write1, buffer, length1 );

		if (write2 != NULL)
		{
			memcpy(write2, buffer + length1, length2);
		}

		hr = sb->lpdsb->Unlock(write1, length1, write2, length2);

		if (hr != DS_OK)
		{
//			TRACE1("\r\n BufferSound_Play failed 3, hr = %d", hr );
			return FALSE;
		}

//		TRACE2("\r\n BufferSound_Play() pSoundPool->sampleLength = %d, sb->sampleOffset = %d ",
//			pSoundPool->sampleLength, sb->sampleOffset );

		// 计算采样缓冲区指针到 缓冲区尾部的尺寸
		remaining = pSoundPool->sampleLength - sb->sampleOffset;
	//	重新计算采样缓冲区偏移量
		if( length_to_wrtie >= remaining )
		{
			sb->sampleOffset = length_to_wrtie - remaining;
		} else {
			sb->sampleOffset += length_to_wrtie;
		}

		buffer += length_to_wrtie;
		length -= length_to_wrtie;
	/******************************************************************************/
	}

//	TRACE1("\r\n BufferSound_Play() after move sb->sampleOffset = %d ",	sb->sampleOffset );

	return TRUE;

#elif defined __linux__

	SLresult result;
	SLuint32 playerState;
	BufferQueue *bq;

	if( 0 == pSoundPool ||
			0 == pSoundPool->bufferArray ||
			0 == pSoundPool->maxStreams ||
			0 == buffer ||
			0 == length)
	{
		return FALSE;
	}

	bq = &pSoundPool->bufferArray[pSoundPool->curStreamId];
	(*bq->playerObj)->GetState(bq->playerObj, &playerState);
	if (SL_OBJECT_STATE_REALIZED == playerState)
	{
		// 计算采样缓冲区指针到 缓冲区尾部的尺寸
		remaining = pSoundPool->sampleLength - bq->sampleOffset;
//		LOGI("BufferSoundPool(len = %d, length = %d)",len, length);
		// 如果需要写入缓冲的数据较多，则分两次写入队列
		if( length >= remaining )
		{
			memcpy( pSoundPool->sampleBuffer + bq->sampleOffset, buffer, remaining );
			result = (*bq->queue)->Enqueue(bq->queue, pSoundPool->sampleBuffer + bq->sampleOffset, remaining);
			if (result != SL_RESULT_SUCCESS) return FALSE;
			bq->sampleOffset = 0;
			buffer += remaining;
			remaining = length - remaining;
		} else {
			remaining = length;
		}
		if( remaining > 0 )
		{
			memcpy( pSoundPool->sampleBuffer + bq->sampleOffset, buffer, remaining );
			// Removes any sound from the queue.
			//result = (*bq.queue)->Clear(bq.queue);
			//if (result != SL_RESULT_SUCCESS) return FALSE;
			// Plays the new sound.
			result = (*bq->queue)->Enqueue(bq->queue, pSoundPool->sampleBuffer+ bq->sampleOffset, remaining);
			if (result != SL_RESULT_SUCCESS) return FALSE;
			bq->sampleOffset += remaining;
		}
	}
#else
#error not support other platform
#endif
	return TRUE;
}
#else
int BufferSound_Load(BufferSoundPool *pSoundPool, char* buffer, int length)
{
	int streamId = 0;
	// find first available buffer queue
	BufferQueue* avail = NULL;
	int i = 0;

	LOGI("BufferSound_Load buf addr = 0x%08X, buf len %d", buffer, length);

	if( 0 == pSoundPool )
	{
		return FALSE;
	}

	ResourceBuffer* b = (ResourceBuffer*)malloc(sizeof(ResourceBuffer));
	if( b )
	{
		b->buf = (char*)malloc(length);
		if( b->buf )
		{
			memcpy( b->buf, buffer, length);
			b->size = length;
			b->next = 0;
			pSoundPool->sampleListEnd->next = b;
			pSoundPool->sampleListEnd = b;
			pSoundPool->SampleNum++;
		} else {
			free(b);
		}
	}
	return pSoundPool->SampleNum;
}

// ret : failed : -1,    found sampleId to play 0 ~ (pSoundPool->maxStreams -1 )
int BufferSound_Play(BufferSoundPool *pSoundPool, int sampleId, float volume)
{

	SLresult result;
	int streamId = 0;
	// find first available buffer queue
	BufferQueue* avail = pSoundPool->bufferArray;
	ResourceBuffer* buf = pSoundPool->sampleList;
	int i;

	LOGI("Play sample %i at volume %f", sampleId, volume);

	if( 0 == pSoundPool )
	{
		return -1;
	}

	for (i = 0; i < pSoundPool->maxStreams; i++, avail++)
	{
		if( 0 == buf )
		{
			LOGI("No sample found");
			return -1;
		}
		if ( FALSE == avail->playing )
		{
			avail->playing = TRUE;
			streamId = i;
			break;
		}
		buf = buf->next;
	}

	if (avail != NULL){
		LOGI("Playing on channel %i", streamId);

		// find the requested sample
		if ( (sampleId > pSoundPool->SampleNum )
				/*|| &pSoundPool->sampleList[sampleId] == NULL*/){
			LOGI("No sample found");
			return 0;
		}
		// convert requested volume 0.0-1.0 to millibels
		// in range max(minVolume, MIN_VOL_MILLIBEL) - maxVolume
		SLmillibel minvol = pSoundPool->minVolume;
		if (minvol < MIN_VOL_MILLIBEL){
			minvol = MIN_VOL_MILLIBEL;
		}

		SLmillibel newVolume = ((minvol - pSoundPool->maxVolume) * (1.0 - volume)) + pSoundPool->maxVolume;

		//adjust volume for the buffer queue
		result = (*avail->volume)->SetVolumeLevel(avail->volume, newVolume);
		check(result);

		//enqueue the sample
		result = (*avail->queue)->Enqueue(avail->queue, (void*)buf->buf, buf->size);
		if (SL_RESULT_SUCCESS != result) {
			LOGI("Enqueue result not OK");
		}

	}else{
		LOGI("No channels available for playback");
	}
	return streamId;
}
#endif
