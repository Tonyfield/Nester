
#include <jni.h>
#include "utils/jniutils.h"
#include "InfoNES.h"
#include "InfoNES_System_Android.h"

jint JNI_OnLoad(JavaVM* pVM, void* reserved)
{
	NES_CONTEXT* pContext = InfoNES_GetContext();
	pContext->pJavaVM = pVM;
	JNIEnv *lEnv;
	if ((*pVM)->GetEnv(pVM, (void**) &lEnv, JNI_VERSION_1_6) != JNI_OK) {
		// A problem occured
		return -1;
	}
	return JNI_VERSION_1_6;
}

JNIEXPORT jboolean JNICALL Java_com_falcon_nesSimon_NesSimu_Initialize(
		JNIEnv * pEnv,
		jobject pThis,
		jobject objBitmap)
{
	return InfoNES_CreateContext(pEnv, InfoNES_GetContext(), pThis, objBitmap);
}

JNIEXPORT void JNICALL Java_com_falcon_nesSimon_NesSimu_destroy(
		JNIEnv * pEnv,
		jobject pThis,
		jboolean bSaveRam)
{
	LOGV("NesSimu.destroy");
	InfoNES_StopThread( pEnv, InfoNES_GetContext(),bSaveRam );
	InfoNES_DestroyContext( pEnv, InfoNES_GetContext() );
}

JNIEXPORT jboolean JNICALL Java_com_falcon_nesSimon_NesSimu_loadGame(
		JNIEnv * pEnv,
		jobject pThis,
		jstring strFilename,
		jboolean bLoadRam)
{
	jboolean ret = JNI_FALSE;
	const char* pFilename = (*pEnv)->GetStringUTFChars(pEnv, strFilename, NULL);

	jsize lStringLength = (*pEnv)->GetStringUTFLength(pEnv, strFilename );

	if (pFilename == NULL) {
		LOGV("NesSimu.loadGame failed 1");
		return JNI_FALSE;
	}

	ret = InfoNES_LoadRomFile(pEnv, InfoNES_GetContext() , pFilename, bLoadRam);

	(*pEnv)->ReleaseStringUTFChars(pEnv, strFilename, pFilename);

	return ret;
}

JNIEXPORT jboolean JNICALL Java_com_falcon_nesSimon_NesSimu_isRamExisted(
		JNIEnv * pEnv,
		jobject pThis,
		jstring strFilename)
{
	BOOL ret;
	const char* pFilename = (*pEnv)->GetStringUTFChars(pEnv, strFilename, NULL);
	ret = InfoNES_IsSaveValid(pFilename);
	(*pEnv)->ReleaseStringUTFChars(pEnv, strFilename, pFilename);
	return ret;

}

JNIEXPORT jboolean JNICALL Java_com_falcon_nesSimon_NesSimu_loadSRAM(
		JNIEnv * pEnv,
		jobject pThis,
		jstring filename)
{
	int ret;
	const char* pFilename = (*pEnv)->GetStringUTFChars(pEnv, filename, NULL);
	ret = InfoNES_LoadSRAM(pFilename);
	(*pEnv)->ReleaseStringUTFChars(pEnv, filename, pFilename);
	return (0 == ret);
}

JNIEXPORT jboolean JNICALL Java_com_falcon_nesSimon_NesSimu_saveSRAM(
		JNIEnv * pEnv,
		jobject pThis,
		jstring filename)
{
	int ret;
	const char* pFilename = (*pEnv)->GetStringUTFChars(pEnv, filename, NULL);
	ret = InfoNES_SaveSRAM(pFilename);
	(*pEnv)->ReleaseStringUTFChars(pEnv, filename, pFilename);
	return (0 == ret);
}

JNIEXPORT jbyteArray JNICALL Java_com_falcon_nesSimon_NesSimu_getSaveRAM(
		JNIEnv *pEnv,
		jobject pThis)
{
	struct SRAM_DATA SramData;
	DWORD bufferlen = sizeof(struct SRAM_DATA);
	jbyteArray bArray = (*pEnv)->NewByteArray( pEnv, bufferlen );

	InfoNES_GetSRAMData( &SramData );
	(*pEnv)->SetByteArrayRegion( pEnv, bArray, 0, bufferlen, &SramData );

//	(*pEnv)->ReleaseByteArrayElements( pEnv, bArray, jBytes, 0);
	return bArray;
}


JNIEXPORT void JNICALL Java_com_falcon_nesSimon_NesSimu_reset(
		JNIEnv * pEnv,
		jobject pThis)
{
	InfoNES_ResetThread( pEnv, InfoNES_GetContext()  );
}

JNIEXPORT void JNICALL Java_com_falcon_nesSimon_NesSimu_setMute(
		JNIEnv * pEnv,
		jobject pThis,
		jboolean bMute)
{
	InfoNES_SetMute( bMute );
}

JNIEXPORT void JNICALL Java_com_falcon_nesSimon_NesSimu_setVolume(
		JNIEnv * pEnv,
		jobject pThis,
		jfloat vol)
{
	InfoNES_SetVolume( vol );
}

JNIEXPORT void JNICALL Java_com_falcon_nesSimon_NesSimu_stop(
		JNIEnv * pEnv,
		jobject pThis,
		jboolean bSaveRAM)
{
	InfoNES_StopThread( pEnv, InfoNES_GetContext(), bSaveRAM );
}

JNIEXPORT void JNICALL Java_com_falcon_nesSimon_NesSimu_pause(
		JNIEnv * pEnv,
		jobject pThis)
{
	InfoNES_Pause(pEnv, InfoNES_GetContext() );
}

JNIEXPORT void JNICALL Java_com_falcon_nesSimon_NesSimu_resume(
		JNIEnv * pEnv,
		jobject pThis)
{
	InfoNES_Resume(pEnv, InfoNES_GetContext()  );
}

JNIEXPORT jint JNICALL Java_com_falcon_nesSimon_NesSimu_getWidth(
		JNIEnv * pEnv,
		jobject pThis)
{
	return NES_DISP_WIDTH;
}
JNIEXPORT jint JNICALL Java_com_falcon_nesSimon_NesSimu_getHeight(
		JNIEnv * pEnv,
		jobject pThis)
{
	return NES_DISP_HEIGHT;
}

JNIEXPORT void JNICALL Java_com_falcon_nesSimon_NesSimu_setPad1Input(
		JNIEnv * pEnv,
		jobject pThis,
		jint padMask,
		jint padValue)
{
	InfoNES_SetPad1State( padMask, padValue );
}

JNIEXPORT void JNICALL Java_com_falcon_nesSimon_NesSimu_setPad2Input(
		JNIEnv * pEnv,
		jobject pThis,
		jint padMask,
		jint padValue)
{
	InfoNES_SetPad2State( padMask, padValue );
}

JNIEXPORT static jstring JNICALL Java_com_falcon_nesSimon_NesSimu_getDebugInfo(
		JNIEnv* pEnv,
		jobject clazz)
{
	return (*pEnv)->NewStringUTF(pEnv, InfoNES_GetErrInfo());
}
