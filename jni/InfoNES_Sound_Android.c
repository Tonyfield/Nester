/*===================================================================*/
/*                                                                   */
/*  InfoNES_Sound_Android.cpp : The function which depends on a system  */
/*                           (for Android)                           */
/*                                                                   */
/*  2013/05/28  InfoNES Project                                      */
/*                                                                   */
/*===================================================================*/

/*-------------------------------------------------------------------*/
/*  Include files                                                    */
/*-------------------------------------------------------------------*/
#include "InfoNES_Sound_Android.h"
#include "InfoNES_System_Android.h"

void InfoNES_soundCreateEngine(OpenSLSoundPool *pSoundPool);
BOOL InfoNES_soundCreateBufferQueueAudioPlayer(OpenSLSoundPool *pSoundPool);
void staticBqPlayerCallback(SLBufferQueueItf bq, void *context);

//#include "InfoNES_Sound_Win.h"
#ifdef NES_SOUND_SUPPORT
#include "InfoNES_pAPU.h"
#endif



static const char *result_to_string(SLresult result)
{
    static char buffer[32];
    if ( /* result >= 0 && */ result < sizeof(result_strings) / sizeof(result_strings[0]))
        return result_strings[result];
    LOGI(buffer, "%d", (int) result);
    return buffer;
}

// Compare result against expected and exit suddenly if wrong

void check2(SLresult result, const char *fn, int line)
{
    if (SL_RESULT_SUCCESS != result) {
        LOGI("\r\n error %s at %s, line %d\n", result_to_string(result), fn, line);
        exit(EXIT_FAILURE);
    }
}

//BufferQueue queues[];

// vector for BufferQueues (one for each channel)
//std::vector<BufferQueue*>* bufferQueues;

// vector for samples
//std::vector<ResourceBuffer*>* samples;

OpenSLSoundPool *InfoNES_soundCreate(int nMaxStreams, SLuint32 samplesPerSec, SLuint32 nBitrate)
{
	int i;
	OpenSLSoundPool *pSoundPool = (OpenSLSoundPool*)malloc(sizeof(OpenSLSoundPool));

	LOGI("InfoNES_soundCreate(%d,%d,%d)", nMaxStreams, samplesPerSec, nBitrate);

	if(  0 != pSoundPool )
	{
		pSoundPool->engineObject = NULL;
		pSoundPool->outputMixObject = NULL;
		pSoundPool->maxStreams = nMaxStreams;
		pSoundPool->samplesPerSec = samplesPerSec * 1000;
		pSoundPool->bitsPerSampe = SL_PCMSAMPLEFORMAT_FIXED_8;
		pSoundPool->bitrate = nBitrate;
		pSoundPool->bufferQueuesArray = (BufferQueue *)0;
		pSoundPool->sampleBuffer = (BYTE *)malloc(samplesPerSec);
		pSoundPool->sampleLength = samplesPerSec; // keep buffer size contains 1 sec sound

		InfoNES_MemorySet(pSoundPool->sampleBuffer, 0, pSoundPool->sampleLength);
		pSoundPool->ptrSampleBuffer = pSoundPool->sampleBuffer;

		pSoundPool->queueSize = 1 + samplesPerSec / rec_freq;
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
		LOGI("Initializing");
		InfoNES_soundCreateEngine(pSoundPool);
		InfoNES_soundCreateBufferQueueAudioPlayer(pSoundPool);

		LOGV("InfoNES_soundCreate() %d", pSoundPool->sampleLength);
		for( i=0 ; i<pSoundPool->queueSize *3/4 ; i++ )
		{
			InfoNES_soundPlay(pSoundPool, pSoundPool->sampleBuffer, pSoundPool->sampleLength / pSoundPool->queueSize );
		}

	}
	return pSoundPool;
}

void InfoNES_soundCreateEngine(OpenSLSoundPool *pSoundPool)
{
	SLresult result;

	const SLInterfaceID engine_ids[] = {SL_IID_ENGINE};
	const SLboolean engine_req[] = {SL_BOOLEAN_TRUE};

	if( 0 == pSoundPool )
	{
		return;
	}
	LOGI("InfoNES_soundCreateEngine");
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

}

// create buffer queue audio player
BOOL InfoNES_soundCreateBufferQueueAudioPlayer(OpenSLSoundPool *pSoundPool)
{
	int i;
	SLresult result;
	BufferQueue* bq;

	LOGI("InfoNES_soundCreateBufferQueueAudioPlayer");
	if( 0 == pSoundPool || 0 == pSoundPool->maxStreams )
	{
		return FALSE;
	}

	// configure audio source
	SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {
		SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, // locatorType
		60               // numBuffers
	};
	loc_bufq.numBuffers = pSoundPool->queueSize;
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

	LOGI("Creating %i streams", pSoundPool->maxStreams);
	pSoundPool->bufferQueuesArray = (BufferQueue *)malloc(pSoundPool->maxStreams*sizeof(BufferQueue));
	if( 0 == pSoundPool->bufferQueuesArray )
	{
		return FALSE;
	}
	bq = pSoundPool->bufferQueuesArray;
	for (i = 0; i < pSoundPool->maxStreams; i++, bq++)
	{
		bq->playing = FALSE;

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

		LOGI("Created stream %i", i);
	}
//	LOGI("Created %i streams", pSoundPool->bufferQueues->size());

	LOGI("createBufferQueueAudioPlayer done");
	return TRUE;
}

void InfoNES_soundDestroyEngine(OpenSLSoundPool *pSoundPool)
{
	int i;
	BufferQueue* bq;
	ResourceBuffer *prev_node, *curr_node;

	if( 0 == pSoundPool )
	{
		return;
	}
	// destroy all buffer queues
	bq = pSoundPool->bufferQueuesArray;
	if( bq )
	{
		for (i = 0; i < pSoundPool->maxStreams; i++){
			(*bq[i].playerObj)->Destroy(bq[i].playerObj);
		}
		free(pSoundPool->bufferQueuesArray);
		pSoundPool->bufferQueuesArray = 0;
		pSoundPool->maxStreams = 0;
	}
	if( pSoundPool->sampleBuffer )
	{
		free(pSoundPool->sampleBuffer);
		pSoundPool->sampleBuffer = (void *)0;
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

}

// a static callback function called every time a buffer finishes playing,
// delegates to the instance function
void staticBqPlayerCallback(SLBufferQueueItf bq, void *context){
	int i;
	OpenSLSoundPool *pSoundPool = (OpenSLSoundPool *)context;
//	LOGI("staticBqPlayerCallback");
	// we might be already destroyed if release() has been called mid-playback
	if( pSoundPool )
	{
        BufferQueue* avail = NULL;

		for (i = 0; i < pSoundPool->maxStreams; i++)
		{
			if (pSoundPool->bufferQueuesArray[i].queue == bq)
			{
				pSoundPool->bufferQueuesArray[i].playing = FALSE;
				break;
			}
		}
	}
}

void InfoNES_soundSetMute(OpenSLSoundPool *pSoundPool, BOOL bMute)
{
	SLresult result;
	BufferQueue* bq;
	
	if( (0 == pSoundPool) || (pSoundPool->curStreamId >= pSoundPool->maxStreams ) )
	{
		return;
	}
	bq = &pSoundPool->bufferQueuesArray[pSoundPool->curStreamId];

	result = (*bq->volume)->SetMute(bq->volume, bMute);
	check(result);
}

BOOL InfoNES_soundGetMute(OpenSLSoundPool *pSoundPool)
{
	SLresult result;
	SLboolean bMute;
	BufferQueue* bq;
	
	if( (0 == pSoundPool) || (pSoundPool->curStreamId >= pSoundPool->maxStreams ) )
	{
		return FALSE;
	}
	bq = &pSoundPool->bufferQueuesArray[pSoundPool->curStreamId];

	result = (*bq->volume)->GetMute(bq->volume, &bMute);
	check(result);
	return bMute;
}


/*************************************************
 *
 *   volume : range in [0.0, 1.0], it is a normalized value
 *
 *************************************************/
void InfoNES_soundSetVolume(OpenSLSoundPool *pSoundPool, float volume )
{
	SLresult result;
	BufferQueue* bq;

	if( (0 == pSoundPool) || (pSoundPool->curStreamId >= pSoundPool->maxStreams ) )
	{
		return;
	}
	bq = &pSoundPool->bufferQueuesArray[pSoundPool->curStreamId];

	// convert requested volume 0.0-1.0 to millibels
	// in range max(minVolume, MIN_VOL_MILLIBEL) - maxVolume
	SLmillibel minvol = pSoundPool->minVolume;
	if (minvol < MIN_VOL_MILLIBEL)
	{
		minvol = MIN_VOL_MILLIBEL;
	}
	if( volume < 0.0)  {volume = 0.0;}
	if( volume > 1.0)  {volume = 1.0;}


	SLmillibel newVolume = ((minvol - pSoundPool->maxVolume) * (1.0 - volume)) + pSoundPool->maxVolume;

	LOGV("Vol min = %d, max = %d, vol = %f, new vol = %d", minvol, pSoundPool->maxVolume, volume, newVolume );

	//adjust volume for the buffer queue
	result = (*bq->volume)->SetVolumeLevel(bq->volume, newVolume);
	check(result);
}

#if 1
BOOL InfoNES_soundPlay(OpenSLSoundPool *pSoundPool, char* buffer, int length)
{
	SLresult result;
	SLuint32 playerState;
	BufferQueue bq;
	int len = 0;

	if( 0 == pSoundPool ||
			0 == pSoundPool->bufferQueuesArray ||
			0 == pSoundPool->maxStreams ||
			0 == buffer ||
			0 == length)
	{
		return FALSE;
	}

	bq = pSoundPool->bufferQueuesArray[0];
	(*bq.playerObj)->GetState(bq.playerObj, &playerState);
	if (SL_OBJECT_STATE_REALIZED == playerState)
	{
		// 计算采样缓冲区指针到 缓冲区尾部的尺寸
		len = pSoundPool->sampleBuffer + pSoundPool->sampleLength - pSoundPool->ptrSampleBuffer;
//		LOGI("OpenSLSoundPool(len = %d, length = %d)",len, length);
		// 如果需要写入缓冲的数据较多，怎分两次写入队列
		if( length >= len )
		{
			InfoNES_MemoryCopy( pSoundPool->ptrSampleBuffer, buffer, len );
			pSoundPool->ptrSampleBuffer = pSoundPool->sampleBuffer;
			result = (*bq.queue)->Enqueue(bq.queue, pSoundPool->ptrSampleBuffer, len);
			if (result != SL_RESULT_SUCCESS) return FALSE;

			buffer += len;
			len = length - len;
		} else {
			len = length;
		}
		if( len > 0 )
		{
			InfoNES_MemoryCopy( pSoundPool->ptrSampleBuffer, buffer, len );
			// Removes any sound from the queue.
			//result = (*bq.queue)->Clear(bq.queue);
			//if (result != SL_RESULT_SUCCESS) return FALSE;
			// Plays the new sound.
			result = (*bq.queue)->Enqueue(bq.queue, pSoundPool->ptrSampleBuffer, len);
			if (result != SL_RESULT_SUCCESS) return FALSE;
			pSoundPool->ptrSampleBuffer += len;
		}
	}
	return TRUE;
}
#else
int InfoNES_soundLoad(OpenSLSoundPool *pSoundPool, char* buffer, int length)
{
	int streamId = 0;
	// find first available buffer queue
	BufferQueue* avail = NULL;
	int i = 0;

	LOGI("InfoNES_soundLoad buf addr = 0x%08X, buf len %d", buffer, length);

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
			InfoNES_MemoryCopy( b->buf, buffer, length);
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
int InfoNES_soundPlay(OpenSLSoundPool *pSoundPool, int sampleId, float volume)
{

	SLresult result;
	int streamId = 0;
	// find first available buffer queue
	BufferQueue* avail = pSoundPool->bufferQueuesArray;
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
