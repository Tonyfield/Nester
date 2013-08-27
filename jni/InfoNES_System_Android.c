/*===================================================================*/
/*                                                                   */
/*  InfoNES_System_Android.c : The function which depends on a system  */
/*                           (for Android)                           */
/*                                                                   */
/*  2000/05/12  InfoNES Project                                      */
/*                                                                   */
/*===================================================================*/

/*-------------------------------------------------------------------*/
/*  Include files                                                    */
/*-------------------------------------------------------------------*/

#ifdef WIN32
  #include <windows.h>
  #include <mmsystem.h>
  #include <stdio.h>
#elif defined ANDROID
  #include <jni.h>
  #include <android/bitmap.h>
  #include "InfoNES_System_Android.h"
  #include "fcntl.h"
#endif

#include "InfoNES.h"
#include "InfoNES_System.h"

//#include "InfoNES_Sound_Win.h"
#ifdef NES_SOUND_SUPPORT
#include "InfoNES_pAPU.h"
#include "buffersoundpool.h"
#endif

extern const DWORD NesPaletteRGB888[];
/*-------------------------------------------------------------------*/
/*  ROM image file information                                       */
/*-------------------------------------------------------------------*/
char szRomName[ MAX_PATH ];
char szSaveName[ MAX_PATH ];
int nSRAM_SaveFlag = 1;
char szErrInfo[512] = {0};

/*-------------------------------------------------------------------*/
/*  Variables for Windows                                            */
/*-------------------------------------------------------------------*/
#define APP_NAME     "InfoNES v0.79J"

//HWND hWndMain;
//WNDCLASS wc;
//HACCEL hAccel;        //���ټ����

unsigned char *pScreenMem;
//HBITMAP hScreenBmp;
//LOGPALETTE *plpal;
//BITMAPINFO *bmi;

// Screen Size Magnification
WORD wScreenMagnification = 2;
#define NES_MENU_HEIGHT     46

/*-------------------------------------------------------------------*/
/*  Variables for Emulation Thread                                   */
/*-------------------------------------------------------------------*/
//HANDLE m_hThread;
//DWORD m_ThreadID = NULL;

/*-------------------------------------------------------------------*/
/*  Variables for Timer & Wait loop                                  */
/*  LINE_PER_TIMER   ��ֵԽ��֡��Խ��                                                                                        */
/*  TIMER_PER_LINE   ��ֵԽС֡��Խ��                                                                                        */
/*  ͬʱ��С������ֵ����ʱ������ʱ���Խ�̣���ϵͳռ�ø��                                      */
/*  ͬʱ�Ŵ�������ֵ��֡�ļ����Խ��Խ���˶��Եò�����                                           */
/*-------------------------------------------------------------------*/
#define LINE_PER_TIMER      789
#define TIMER_PER_LINE      50

WORD wLines;
WORD wLinePerTimer;

BOOL bWaitFlag;
BOOL bPauseFlag;
#ifdef WIN32
MMRESULT uTimerID;
#else
BOOL bQuitFlag = 0;  // 0 : run    non-0 : quit
timer_t uTimerID = 0;
DWORD dwPad1 = 0;
DWORD dwPad2 = 0;
#endif

NES_MUTEX_TYPE WaitFlagCriticalSection;
NES_MUTEX_TYPE PauseFlagCriticalSection;

NES_MUTEX_TYPE NesStateCriticalSection;


BOOL bAutoFrameskip = TRUE;

NES_CONTEXT g_NesContext = {0};


/*-------------------------------------------------------------------*/
/*  Variables for Sound Emulation                                    */
/*-------------------------------------------------------------------*/
//DIRSOUND* lpSndDevice = NULL;
BufferSoundPool *lpSoundPool = NULL;

/*-------------------------------------------------------------------*/
/*  Variables for Expiration                                         */
/*-------------------------------------------------------------------*/
//#define EXPIRED_YEAR    2001
//#define EXPIRED_MONTH   3
//#define EXPIRED_MSG     "This software has been expired.\nPlease download newer one."

/*-------------------------------------------------------------------*/
/*  Function prototypes ( Windows specific )                         */
/*-------------------------------------------------------------------*/

//LRESULT CALLBACK MainWndproc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
//LRESULT CALLBACK AboutDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
//void ShowTitle( HWND hWnd );
//void SetWindowSize( WORD wMag );
//int LoadSRAM();
//int SaveSRAM();
static BOOL InfoNES_StartTimer();
static void InfoNES_StopTimer();
#ifdef WIN32
static void CALLBACK TimerFunc( UINT nID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);
#else
static void TimerFunc(void *arg);
void *InfoNES_Proc(void *pArgs);
#endif
jobject makeGlobalRef(JNIEnv* pEnv, const jobject localRef);
void deleteGlobalRef(JNIEnv* pEnv, jobject* pRef);


void outputLog(const char *fn, int line, const char *format, ...)
{
	FILE * log_file = fopen("/mnt/external_sd/nester.log", "a+");
	if( log_file )
	{
		va_list argp;
		va_start(argp,format);
		__android_log_print(ANDROID_LOG_VERBOSE, TAG, format, argp);
		fprintf(log_file,"\r\n file %s(line %d):", fn, line);
		vfprintf(log_file,format, argp);
		va_end(argp);

		fclose(log_file);
	}
}

NES_CONTEXT *InfoNES_GetContext(void)
{
	return &g_NesContext;
}

BOOL InfoNES_IsSaveValid(const char* pSaveName)
{
	FILE *fd;
	char saveName[ MAX_PATH ];

	if( 0 == pSaveName || 0 == pSaveName[0])
	{
		return FALSE;
	}

	fd = fopen(pSaveName,"rb");
	if( fd )
	{
		fclose( fd );
		return TRUE;
	} else {
		return FALSE;
	}
}

#if 0
/*===================================================================*/
/*                                                                   */
/*           LoadSRAM() : Load a SRAM                                */
/*                                                                   */
/*===================================================================*/
int InfoNES_LoadSRAM()
{
	/*
	 *  Load a SRAM
	 *
	 *  Return values
	 *     0 : Normally
	 *    -1 : SRAM data couldn't be read
	 */

	FILE *fp;
	unsigned char pSrcBuf[ NES_SRAM_SIZE ];
	unsigned char chData;
	unsigned char chTag;
	int nRunLen;
	int nDecoded;
	int nDecLen;
	int nIdx;

	// It doesn't need to save it
	nSRAM_SaveFlag = 1;

	// It is finished if the ROM doesn't have SRAM
	LOGI("InfoNES_LoadSRAM() ROM_SRAM = %d", ROM_SRAM );
//	if ( !ROM_SRAM )
//		return 0;

	// The preparation of the SRAM file name
//	strcpy( szSaveName, szRomName );
//	strcpy( strrchr( szSaveName, '.' ) + 1, "srm" );

	/*-------------------------------------------------------------------*/
	/*  Read a SRAM data                                                 */
	/*-------------------------------------------------------------------*/

	// Open SRAM file
	LOGI("InfoNES_LoadSRAM() szSaveName = %s", szSaveName );
	fp = fopen( szSaveName, "rb" );
	if ( fp == NULL )
		return -1;

	// Read SRAM data
	fread( pSrcBuf, NES_SRAM_SIZE, 1, fp );

	// Close SRAM file
	fclose( fp );

	/*-------------------------------------------------------------------*/
	/*  Extract a SRAM data                                              */
	/*-------------------------------------------------------------------*/

	nDecoded = 0;
	nDecLen = 0;

	chTag = pSrcBuf[ nDecoded++ ];

	while ( nDecLen < 8192 )
	{
		chData = pSrcBuf[ nDecoded++ ];

		if ( chData == chTag )
		{
			chData = pSrcBuf[ nDecoded++ ];
			nRunLen = pSrcBuf[ nDecoded++ ];
			for ( nIdx = 0; nIdx < nRunLen + 1; ++nIdx )
			{
				SRAM[ nDecLen++ ] = chData;
			}
		}
		else
		{
			SRAM[ nDecLen++ ] = chData;
		}
	}
	// There is necessity to save it
	nSRAM_SaveFlag = 1;
	LOGI("InfoNES_LoadSRAM() end" );

	// Successful
	return 0;
}

/*===================================================================*/
/*                                                                   */
/*           SaveSRAM() : Save a SRAM                                */
/*                                                                   */
/*===================================================================*/
int InfoNES_SaveSRAM()
{
	/*
	 *  Save a SRAM
	 *
	 *  Return values
	 *     0 : Normally
	 *    -1 : SRAM data couldn't be written
	 */

	FILE *fp;
	int nUsedTable[ 256 ];
	unsigned char chData;
	unsigned char chPrevData;
	unsigned char chTag;
	int nIdx;
	int nEncoded;
	int nEncLen;
	int nRunLen;
	unsigned char pDstBuf[ NES_SRAM_SIZE ];

	LOGI("InfoNES_SaveSRAM() nSRAM_SaveFlag = %d", nSRAM_SaveFlag );
	if ( !nSRAM_SaveFlag )
		return 0;  // It doesn't need to save it

	/*-------------------------------------------------------------------*/
	/*  Compress a SRAM data                                             */
	/*-------------------------------------------------------------------*/

	memset( nUsedTable, 0, sizeof nUsedTable );

	for ( nIdx = 0; nIdx < NES_SRAM_SIZE; ++nIdx )
	{
		++nUsedTable[ SRAM[ nIdx++ ] ];
	}
	for ( nIdx = 1, chTag = 0; nIdx < 256; ++nIdx )
	{
		if ( nUsedTable[ nIdx ] < nUsedTable[ chTag ] )
			chTag = nIdx;
	}

	nEncoded = 0;
	nEncLen = 0;
	nRunLen = 1;

	pDstBuf[ nEncLen++ ] = chTag;

	chPrevData = SRAM[ nEncoded++ ];

	while ( nEncoded < NES_SRAM_SIZE && nEncLen < NES_SRAM_SIZE - 133 )
	{
		chData = SRAM[ nEncoded++ ];

		if ( chPrevData == chData && nRunLen < 256 )
			++nRunLen;
		else
		{
			if ( nRunLen >= 4 || chPrevData == chTag )
			{
				pDstBuf[ nEncLen++ ] = chTag;
				pDstBuf[ nEncLen++ ] = chPrevData;
				pDstBuf[ nEncLen++ ] = nRunLen - 1;
			}
			else
			{
				for ( nIdx = 0; nIdx < nRunLen; ++nIdx )
					pDstBuf[ nEncLen++ ] = chPrevData;
			}

			chPrevData = chData;
			nRunLen = 1;
		}

	}
	if ( nRunLen >= 4 || chPrevData == chTag )
	{
		pDstBuf[ nEncLen++ ] = chTag;
		pDstBuf[ nEncLen++ ] = chPrevData;
		pDstBuf[ nEncLen++ ] = nRunLen - 1;
	}
	else
	{
		for ( nIdx = 0; nIdx < nRunLen; ++nIdx )
			pDstBuf[ nEncLen++ ] = chPrevData;
	}

	/*-------------------------------------------------------------------*/
	/*  Write a SRAM data                                                */
	/*-------------------------------------------------------------------*/

	// Open SRAM file
	fp = fopen( szSaveName, "wb" );
	LOGI("InfoNES_SaveSRAM() szSaveName = %s", szSaveName );
	if ( fp == NULL )
		return -1;

	// Write SRAM data
	fwrite( pDstBuf, nEncLen, 1, fp );

	// Close SRAM file
	fclose( fp );
	LOGI("InfoNES_SaveSRAM() end" );

	// Successful
	return 0;
}
#else
/*===================================================================*/
/*                                                                   */
/*           LoadSRAM() : Load a SRAM                                */
/*                                                                   */
/*===================================================================*/
int InfoNES_LoadSRAM(const char* pFilename)
{
	/*
	 *  Load a SRAM
	 *
	 *  Return values
	 *     0 : Normally
	 *    -1 : SRAM data couldn't be read
	 */

	int fd;
	struct SRAM_DATA SramData;
	int len;

	outputLog(__FILE__,__LINE__, "\r\n InfoNES_LoadSRAM() pFilename = %s", pFilename );
	//fp = fopen( szSaveName, "rb" );
	fd = open( pFilename, O_RDONLY, 0 );
	if ( fd == 0 )
	{
		outputLog(__FILE__,__LINE__, "\r\n InfoNES_LoadSRAM() open failed" );
		return -1;
	}
	// Read SRAM data
	len = read(fd, &SramData, sizeof(SramData));
	// Close SRAM file
	close( fd );

	if( len != sizeof(SramData) ){
		return -1;
	}
	InfoNES_SetSRAMData( &SramData );

	// Successful
	return 0;
}

/*===================================================================*/
/*                                                                   */
/*           SaveSRAM() : Save a SRAM                                */
/*        write external SD is not invalid                           */
/*===================================================================*/
int InfoNES_SaveSRAM(const char* pFilename)
{
	/*
	 *  Save a SRAM
	 *
	 *  Return values
	 *     0 : Normally
	 *    -1 : SRAM data couldn't be written
	 */
	int fd;
	int len;
	struct SRAM_DATA SramData;

	/*-------------------------------------------------------------------*/
	/*  Compress a SRAM data                                             */
	/*-------------------------------------------------------------------*/
	InfoNES_GetSRAMData( &SramData );
	/*-------------------------------------------------------------------*/
	/*  Write a SRAM data                                                */
	/*-------------------------------------------------------------------*/
	// Open SRAM file
//	fp = fopen( szSaveName, "wb+" );
	outputLog(__FILE__,__LINE__, "\r\n InfoNES_SaveSRAM() pFilename = %s", pFilename );
	fd = creat(pFilename, 0666 );
//	fd = open( pFilename, O_WRONLY|O_CREAT, 0666 );
	if ( fd == 0 )
	{
		outputLog(__FILE__,__LINE__, "\r\n InfoNES_SaveSRAM() create failed" );
		return -1;
	}

	// Write SRAM data
	len = pwrite(fd, &SramData, sizeof(SramData), 0);
	// Close SRAM file
	close( fd );

	// Successful
	return (len == sizeof(SramData))? 0 : -1 ;
}
#endif
/*
int InfoNES_GetSRAMSize()
{
	return sizeof(struct SRAM_DATA);
}
*/

/*===================================================================*/
/*                                                                   */
/*                  InfoNES_QuitFlag() : return 0, keep in loop; non-0, quit loop                     */
/*                                                                   */
/*===================================================================*/
int InfoNES_QuitFlag(void)
{
	return bQuitFlag;
}

/*===================================================================*/
/*                                                                   */
/*               InfoNES_ReadRom() : Read ROM image file             */
/*                                                                   */
/*===================================================================*/
int InfoNES_ReadRom( const char *pszFileName )
{
	/*
	 *  Read ROM image file
	 *
	 *  Parameters
	 *    const char *pszFileName          (Read)
	 *
	 *  Return values
	 *     0 : Normally
	 *    -1 : Error
	 */
	LOGV("InfoNES_ReadRom start");
#ifdef __linux__
	int fd;

	/* Open ROM file */
	fd = open( pszFileName, O_RDONLY, 0 );
	if ( fd == -1 )
	{
		LOGV("InfoNES_ReadRom error -1");
		return -1;
	}

	/* Read ROM Header */
	read( fd, &NesHeader, sizeof( NesHeader ) );
	if ( memcmp( NesHeader.byID, "NES\x1a", 4 ) != 0 )
	{
		/* not .nes file */
		LOGV("InfoNES_ReadRom error -2");
		close( fd );
		return -2;
	}

	/* Clear SRAM */
	memset( SRAM, 0, NES_SRAM_SIZE );

	/* If trainer presents Read Triner at 0x7000-0x71ff */
	if ( NesHeader.byInfo1 & 4 )
	{
		read( fd, &SRAM[ 0x1000 ], 512 );
	}

	LOGV("InfoNES_ReadRom: byRomSize = %d", NesHeader.byRomSize);
	/* Allocate Memory for ROM Image */
	ROM = (BYTE *)malloc( NesHeader.byRomSize * 0x4000 );

	/* Read ROM Image */
	read( fd, ROM, 0x4000 * NesHeader.byRomSize );

	LOGV("InfoNES_ReadRom: byVRomSize = %d", NesHeader.byVRomSize);
	if ( NesHeader.byVRomSize > 0 )
	{
		/* Allocate Memory for VROM Image */
		VROM = (BYTE *)malloc( NesHeader.byVRomSize * 0x2000 );

		/* Read VROM Image */
		read( fd, VROM, 0x2000 * NesHeader.byVRomSize );
	}

	/* File close */
	close( fd );
#else
	FILE *fp;

	/* Open ROM file */
	fp = fopen( pszFileName, "rb" );
	if ( fp == NULL )
		return -1;

	/* Read ROM Header */
	fread( &NesHeader, sizeof NesHeader, 1, fp );
	if ( memcmp( NesHeader.byID, "NES\x1a", 4 ) != 0 )
	{
		/* not .nes file */
		fclose( fp );
		return -2;
	}

	/* Clear SRAM */
	memset( SRAM, 0, NES_SRAM_SIZE );

	/* If trainer presents Read Triner at 0x7000-0x71ff */
	if ( NesHeader.byInfo1 & 4 )
	{
		fread( &SRAM[ 0x1000 ], 512, 1, fp );
	}

	LOGV("InfoNES_ReadRom: byRomSize = %d", NesHeader.byRomSize);
	/* Allocate Memory for ROM Image */
	ROM = (BYTE *)malloc( NesHeader.byRomSize * 0x4000 );

	/* Read ROM Image */
	fread( ROM, 0x4000, NesHeader.byRomSize, fp );

	LOGV("InfoNES_ReadRom: byVRomSize = %d", NesHeader.byVRomSize);
	if ( NesHeader.byVRomSize > 0 )
	{
		/* Allocate Memory for VROM Image */
		VROM = (BYTE *)malloc( NesHeader.byVRomSize * 0x2000 );

		/* Read VROM Image */
		fread( VROM, 0x2000, NesHeader.byVRomSize, fp );
	}

	/* File close */
	fclose( fp );
#endif
	/* Successful */
	return 0;
}

/*===================================================================*/
/*                                                                   */
/*           InfoNES_ReleaseRom() : Release a memory for ROM           */
/*                                                                   */
/*===================================================================*/
void InfoNES_ReleaseRom(void)
{
	/*
	 *  Release a memory for ROM
	 *
	 */
	if ( ROM )
	{
		free( ROM );
		ROM = NULL;
	}

	if ( VROM )
	{
		free( VROM );
		VROM = NULL;
	}
}

/*===================================================================*/
/*                                                                   */
/*            InfoNES_Pause() : Pause                  */
/*                                                                   */
/*===================================================================*/
void InfoNES_Pause(JNIEnv * pEnv, NES_CONTEXT *pNesContext)
{
	LOGV("InfoNES_Pause");
	if( pNesContext && pNesContext->threadNes )
	{
		NES_MUTEX_LOCK( PauseFlagCriticalSection );
		bPauseFlag = TRUE;
		NES_MUTEX_UNLOCK( PauseFlagCriticalSection );
	}
}

/*===================================================================*/
/*                                                                   */
/*            InfoNES_Resume() : Resume                  */
/*                                                                   */
/*===================================================================*/
void InfoNES_Resume(JNIEnv * pEnv, NES_CONTEXT *pNesContext)
{
	LOGV("InfoNES_Resume");
	if( pNesContext && pNesContext->threadNes && bPauseFlag == TRUE )
	{
		NES_MUTEX_LOCK( PauseFlagCriticalSection );
		bPauseFlag = FALSE;
		NES_MUTEX_UNLOCK( PauseFlagCriticalSection );
	//	raise(SIGUSR1);
	//	raise(SIGUSR1);
		NES_SEM_POST(pNesContext->sem);
	}
}

/*===================================================================*/
/*                                                                   */
/*            InfoNES_IsPause() : is Pause                  */
/*                                                                   */
/*===================================================================*/
BOOL InfoNES_IsPause()
{
	NES_CONTEXT *pNesContext = InfoNES_GetContext();
//	LOGV("InfoNES_IsPause() bPauseFlag = %d, bQuitFlag = %d ", bPauseFlag,bQuitFlag );
	if( bPauseFlag || bQuitFlag )
	{
		int signum;
		LOGV("InfoNES_IsPause() waiting for signal......");
		NES_SEM_WAIT(pNesContext->sem);
		LOGV("InfoNES_IsPause()  get signal = %d, quitFlag = %d",signum, InfoNES_QuitFlag());
	}
	return TRUE;
}

/*===================================================================*/
/*                                                                   */
/*      InfoNES_StartThread() :                                     */
/*           �����߳�                                                                 */
/*                                                                   */
/*===================================================================*/
BOOL InfoNES_StartThread(JNIEnv * pEnv, NES_CONTEXT *pNesContext)
{
	pthread_attr_t lAttributes;

	if( 0 == pEnv || 0 == pNesContext || 0 != pNesContext->threadNes )
	{
		LOGV("InfoNES_StartThread failed 1");
		return 0;
	}

	bQuitFlag = 0;
	if(  (0 != pthread_attr_init(&lAttributes))
		|| ( 0 != pthread_create(&(pNesContext->threadNes), &lAttributes, InfoNES_Proc, pNesContext)))
	{
		InfoNES_StopThread(pEnv, pNesContext,FALSE);
		LOGV("InfoNES_StartThread failed 2");
		return 0;
	}
	pthread_attr_destroy(&lAttributes);
	// Start timer
	InfoNES_StartTimer();
	return 1;
}
/*===================================================================*/
/*                                                                   */
/*      InfoNES_StopThread() :                                     */
/*           ֹͣ�߳�                                                                 */
/*                                                                   */
/*===================================================================*/
BOOL InfoNES_StopThread(JNIEnv * pEnv, NES_CONTEXT *pNesContext, BOOL bSaveRAM)
{
	if( 0 == pEnv || 0 == pNesContext )
	{
		LOGV("InfoNES_StopThread()  exit 1");
		return 0;
	}
	if( 0 != pNesContext->threadNes )
	{
		int sem_val;
		// Waits for the watcher thread to stop.
//		pthread_mutex_lock(&QuitFlagCriticalSection);
		bQuitFlag = 1;
//		NES_SLEEP_MS(1); // �ȴ���� �ź���
//		pthread_mutex_unlock(&QuitFlagCriticalSection);

		sem_getvalue(&(pNesContext->sem), &sem_val);
		LOGV("InfoNES_StopThread() semaphore = %d", sem_val);
		NES_SEM_POST(pNesContext->sem);
		pthread_join(pNesContext->threadNes, NULL);

		pNesContext->threadNes = 0;
		LOGV("InfoNES_StopThread()  pthread_join");

	}
	// Save SRAM File
	if( bSaveRAM )
	{
		InfoNES_SaveSRAM(szSaveName);
	}
	// Finalize
//	InfoNES_Fin();
	return 1;
}
/*===================================================================*/
/*                                                                   */
/*      InfoNES_ResetThread() :                                     */
/*           ���������߳�                                                                 */
/*                                                                   */
/*===================================================================*/
void InfoNES_ResetThread(JNIEnv * pEnv, NES_CONTEXT *pNesContext)
{
	LOGV("InfoNES_ResetThread");
	InfoNES_LoadRomFile(pEnv, pNesContext, szRomName,FALSE);
}
/*===================================================================*/
/*                                                                   */
/*      InfoNES_CreateContext() :                                      */
/*           ��ʼ������ Java ����Ͷ���Ľӿ�                                                                      */
/*                                                                   */
/*===================================================================*/
BOOL InfoNES_CreateContext(JNIEnv *pEnv,
						NES_CONTEXT *pNesContext,
						jobject refNesSimu,
						jobject refBitmap)
{
	if( 0 == pEnv || 0 == pNesContext )
	{
		LOGV("InfoNES_CreateContext failed 1");
		return 0;
	}
	// Caches JavaVM.
	if( ( 0 == pNesContext->pJavaVM )
		&& ((*pEnv)->GetJavaVM(pEnv, &(pNesContext->pJavaVM)) != JNI_OK) )
	{
		LOGV("InfoNES_CreateContext failed 2");
		return 0;
	}
	// caches classNesSimu
	if( 0 == pNesContext->classNesSimu )
	{
		jclass clsNesSimu = (*pEnv)->FindClass(pEnv, "com/falcon/nesSimon/NesSimu");
		LOGV("InfoNES_CreateContext\n");
		pNesContext->classNesSimu = makeGlobalRef(pEnv, clsNesSimu);
		if( 0 == pNesContext->classNesSimu )
		{
			LOGV("InfoNES_CreateContext failed 3");
			return 0;
		}
	}
	if( 0 != refNesSimu && (JNI_TRUE != (*pEnv)->IsSameObject(pEnv, pNesContext->objNesSimu, refNesSimu )))
	{
		pNesContext->objNesSimu = makeGlobalRef(pEnv, refNesSimu);
		if( 0 == pNesContext->objNesSimu )
		{
			LOGV("InfoNES_CreateContext failed 4");
			return 0;
		}
	}
	if( 0 != refBitmap && (JNI_TRUE != (*pEnv)->IsSameObject(pEnv, pNesContext->objBitmap, refBitmap )))
	{
		pNesContext->objBitmap = makeGlobalRef(pEnv, refBitmap);
		if( 0 == pNesContext->objBitmap )
		{
			LOGV("InfoNES_CreateContext failed 5");
			return 0;
		}
	}
	if( 0 == pNesContext->methodUpdateView )
	{
		pNesContext->methodUpdateView = (*pEnv)->GetMethodID(pEnv, pNesContext->classNesSimu, "UpdateView", "()V");
		if( 0 == pNesContext->methodUpdateView )
		{
			LOGV("InfoNES_CreateContext failed 6");
			return 0;
		}
	}
	NES_MUTEX_CREATE( WaitFlagCriticalSection );
	NES_MUTEX_CREATE( PauseFlagCriticalSection );
	NES_SEM_INIT(pNesContext->sem, 0);
	{
		int sem_val;
		sem_getvalue(&(pNesContext->sem), &sem_val);
		LOGV("InfoNES_CreateContext success, sem = %d", sem_val);
	}
	return 1;
}


/*===================================================================*/
/*                                                                   */
/*      InfoNES_DestroyContext() :                                     */
/*           ��ٷ��� Java ����Ͷ���Ľӿ�                                                                 */
/*                                                                   */
/*===================================================================*/
void InfoNES_DestroyContext(JNIEnv * pEnv, NES_CONTEXT *pNesContext)
{
	if( 0 == pEnv || 0 == pNesContext )
	{
		LOGV("InfoNES_DestroyContext failed 1");
		return;
	}
	InfoNES_StopTimer();
	NES_SEM_DESTROY(pNesContext->sem);

	NES_MUTEX_DESTROY( WaitFlagCriticalSection );
	NES_MUTEX_DESTROY( PauseFlagCriticalSection );
	deleteGlobalRef(pEnv, &(pNesContext->classNesSimu) );
	deleteGlobalRef(pEnv, &(pNesContext->classBitmap) );
	deleteGlobalRef(pEnv, &(pNesContext->objNesSimu) );
	deleteGlobalRef(pEnv, &(pNesContext->objBitmap) );
	LOGV(" InfoNES_DestroyContext success");

}

/*===================================================================*/
/*                                                                   */
/*      InfoNES_CheckContext() :                                     */
/*           ������ Java ����Ͷ���Ľӿ���Ч                                                                 */
/*                                                                   */
/*===================================================================*/
BOOL InfoNES_CheckContext(NES_CONTEXT *pNesContext)
{
	if( 0 == pNesContext
		|| 0 == pNesContext->pJavaVM
		/*|| 0 == pNesContext->classBitmap*/
		|| 0 == pNesContext->classNesSimu
		|| 0 == pNesContext->objBitmap
		|| 0 == pNesContext->objNesSimu
		|| 0 == pNesContext->methodUpdateView )
	{
		return 0;
	}
	return 1;
}


BOOL InfoNES_LoadRomFile(
		JNIEnv * pEnv,
		NES_CONTEXT *pNesContext,
		const char *pFilename,
		BOOL bLoadRAM)
{
	LOGV("InfoNES_LoadRomFile start");
	if( (0 == pEnv) 
		|| (0 == pNesContext)
		|| (0 == InfoNES_CheckContext(pNesContext)) 
		||(NULL == pFilename) )
	{
//		InfoNES_PrintErrInfo("InfoNES_LoadRomFile failed 1");
		return FALSE;
	}

	// �����NES�߳������У��ȹر�
	InfoNES_StopThread(pEnv, pNesContext, FALSE);

	if ( sizeof(szRomName) < strlen(pFilename) ) {
//		InfoNES_PrintErrInfo("InfoNES_LoadRomFile failed 2");
		return FALSE;
	}

	// Load cassette
	if ( 0 != InfoNES_Load( pFilename ) )
	{
//		InfoNES_PrintErrInfo("InfoNES_LoadRomFile failed 3");
		return FALSE;
	}
	// Set a ROM image name
	strcpy( szRomName, pFilename );
	// The preparation of the SRAM file name
	strcpy( szSaveName, pFilename );
//	strcpy( strrchr( szSaveName, '.' ) + 1, "srm" );
	strcat( szSaveName, ".srm" );
	// Load SRAM
	LOGI("InfoNES_LoadRomFile() bLoadRAM = %d", bLoadRAM );
	// Initialize InfoNES
	InfoNES_Init();
	if( bLoadRAM )
	{
		InfoNES_LoadSRAM(szSaveName);
	}

	return InfoNES_StartThread(pEnv, pNesContext);
}

/*===================================================================*/
/*                                                                   */
/*      InfoNES_LoadFrame() :                                        */
/*           Transfer the contents of work frame on the screen       */
/*                                                                   */
/*===================================================================*/
WORD s_Frame[ NES_DISP_WIDTH * NES_DISP_HEIGHT ];

void InfoNES_LoadFrame(void)
{
	/*
	 *  Transfer the contents of work frame on the screen
	 *
	 */
	NES_CONTEXT * pNesContext = &g_NesContext;
	JNIEnv *pEnv;
	AndroidBitmapInfo lBitmapInfo;

	if( 0 == InfoNES_CheckContext(pNesContext) )
	{
		LOGV("InfoNES_LoadFrame failed 1");
		return;
	}

	if( JNI_OK != (*(pNesContext->pJavaVM))->GetEnv(pNesContext->pJavaVM, (void**) &pEnv, JNI_VERSION_1_6))
	{
		LOGV("InfoNES_LoadFrame failed 2");
		return;
	}
	if (AndroidBitmap_getInfo(pEnv, pNesContext->objBitmap, &lBitmapInfo) < 0) {
		return;
	}
	if (lBitmapInfo.format != ANDROID_BITMAP_FORMAT_RGBA_8888
		|| NES_DISP_WIDTH != lBitmapInfo.width
		|| NES_DISP_HEIGHT != lBitmapInfo.height )
	{
		LOGV("InfoNES_LoadFrame failed 3");
		return;
	}
	uint32_t* lBitmapContent;
	if (AndroidBitmap_lockPixels(pEnv, pNesContext->objBitmap,	(void**)&lBitmapContent) < 0) {
		LOGV("InfoNES_LoadFrame failed 4");
		return;
	}

	{
		// Set screen data
	#ifdef NES_WORKFRAME_SLIM
		WORD k;
		BYTE *src;
		DWORD *dst;

		dst = (DWORD *)lBitmapContent;
		src = WorkFrame;
		for (k=0;k<NES_DISP_WIDTH * NES_DISP_HEIGHT; k++)
		{
			{
				*dst++ = NesPaletteRGB888[0x3F&(*src++)];
			}
		}
	#else
	#error not support RGB565
		memcpy( pScreenMem, WorkFrame, NES_DISP_WIDTH * NES_DISP_HEIGHT * 2 );
	#endif
	}
	AndroidBitmap_unlockPixels(pEnv, pNesContext->objBitmap);

	(*pEnv)->CallVoidMethod(pEnv,pNesContext->objNesSimu, pNesContext->methodUpdateView );

	//  ���²��Դ��뱻����
	if( 0 /*== s_FrameCnt*/ )
	{
		char frm_file[256];
		FILE *fp;
		int k;

//		sprintf(frm_file, "d:\\d2\\nes_frame_%d.log", s_FrameCnt);
		fp = fopen(frm_file,"w+");

		if(fp)
		{
//			fwrite(WorkFrame, 2, NES_DISP_WIDTH*NES_DISP_HEIGHT, fp );
			for(k=0; k<64 ; k++)
			{
				fprintf(fp, "0x%04X, ",  NesPalette[k] );
//					(NesPalette[k]>>7)&0xF8,(NesPalette[k]>>2)&0xF8, (NesPalette[k]<<3)&0xF8, (NesPalette[k]>>15)&0x01 );
				if( 0x07 == (k&0x07) ) {	fprintf(fp, "\r\n" );	}
			}
			fclose(fp);
		}
	}
}

/*===================================================================*/
/*                                                                   */
/*             InfoNES_SetPad1State() : Set a joypad 1 state               */
/*                                                                   */
/*===================================================================*/
void InfoNES_SetPad1State( DWORD pdwPadMask1, DWORD pdwPad1 )
{
	dwPad1 = (dwPad1 & ~pdwPadMask1) | (pdwPad1& pdwPadMask1);
}
/*===================================================================*/
/*                                                                   */
/*             InfoNES_SetPad2State() : Set a joypad 2 state               */
/*                                                                   */
/*===================================================================*/
void InfoNES_SetPad2State( DWORD pdwPadMask2, DWORD pdwPad2 )
{
	dwPad2 = (dwPad2 & ~pdwPadMask2) | (pdwPad2& pdwPadMask2);
}
/*===================================================================*/
/*                                                                   */
/*             InfoNES_PadState() : Get a joypad state               */
/*                                                                   */
/*===================================================================*/
void InfoNES_PadState( DWORD *pdwPad1, DWORD *pdwPad2, DWORD *pdwSystem )
{
	DWORD tmpPad1;
	static DWORD pad1_Acnt = 0;
	static DWORD pad1_Bcnt = 0;
	/*
	 *  Get a joypad state
	 *
	 *  Parameters
	 *    DWORD *pdwPad1                   (Write)
	 *      Joypad 1 State
	 *
	 *    DWORD *pdwPad2                   (Write)
	 *      Joypad 2 State
	 *
	 *    DWORD *pdwSystem                 (Write)
	 *      Input for InfoNES
	 *
	 */

#ifdef WIN32
	static DWORD dwSysOld;
	DWORD dwTemp;
	/* Joypad 1 */
	*pdwPad1 =   ( GetAsyncKeyState( 'X' )        < 0 ) |
				 ( ( GetAsyncKeyState( 'Z' )        < 0 ) << 1 ) |
				 ( ( GetAsyncKeyState( 'A' )        < 0 ) << 2 ) |
				 ( ( GetAsyncKeyState( 'S' )        < 0 ) << 3 ) |
				 ( ( GetAsyncKeyState( VK_UP )      < 0 ) << 4 ) |
				 ( ( GetAsyncKeyState( VK_DOWN )    < 0 ) << 5 ) |
				 ( ( GetAsyncKeyState( VK_LEFT )    < 0 ) << 6 ) |
				 ( ( GetAsyncKeyState( VK_RIGHT )   < 0 ) << 7 );

	*pdwPad1 = *pdwPad1 | ( *pdwPad1 << 8 );

	/* Joypad 2 */
	*pdwPad2 = 0;

	/* Input for InfoNES */
	dwTemp = ( GetAsyncKeyState( VK_ESCAPE )  < 0 );

	/* Only the button pushed newly should be inputted */
	*pdwSystem = ~dwSysOld & dwTemp;

	/* keep this input */
	dwSysOld = dwTemp;

	/* Deal with a message */
	MSG msg;
	while ( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
	{
		if ( GetMessage( &msg, NULL, 0, 0 ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
	}
#else

	/* Joypad 1 */
	*pdwPad1 = ( *pdwPad1 << 8 ) | dwPad1 ;
//	*pdwPad1 = ( dwPad1 << 8 ) | dwPad1 ;
	/* Joypad 2 */
	*pdwPad2 = *pdwPad2 | ( dwPad2 << 8 );
	*pdwSystem = 0;

#endif
}

/*===================================================================*/
/*                                                                   */
/*             InfoNES_MemoryCopy() : memcpy                         */
/*                                                                   */
/*===================================================================*/
void *InfoNES_MemoryCopy( void *dest, const void *src, int count )
{
	/*
	 *  memcpy
	 *
	 *  Parameters
	 *    void *dest                       (Write)
	 *      Points to the starting address of the copied block�fs destination
	 *
	 *    const void *src                  (Read)
	 *      Points to the starting address of the block of memory to copy
	 *
	 *    int count                        (Read)
	 *      Specifies the size, in bytes, of the block of memory to copy
	 *
	 *  Return values
	 *    Pointer of destination
	 */

	memcpy( dest, src, count );
	return dest;
}

/*===================================================================*/
/*                                                                   */
/*             InfoNES_MemorySet() : Get a joypad state              */
/*                                                                   */
/*===================================================================*/
void *InfoNES_MemorySet( void *dest, int c, int count )
{
	/*
	 *  memset
	 *
	 *  Parameters
	 *    void *dest                       (Write)
	 *      Points to the starting address of the block of memory to fill
	 *
	 *    int c                            (Read)
	 *      Specifies the byte value with which to fill the memory block
	 *
	 *    int count                        (Read)
	 *      Specifies the size, in bytes, of the block of memory to fill
	 *
	 *  Return values
	 *    Pointer of destination
	 */

	memset( dest, c, count );
	return dest;
}

/*===================================================================*/
/*                                                                   */
/*                DebugPrint() : Print debug message                 */
/*                                                                   */
/*===================================================================*/
void InfoNES_DebugPrint( char *pszMsg )
{
#ifdef WIN32
	_RPT0( _CRT_WARN, pszMsg );
#endif
}

/*===================================================================*/
/*                                                                   */
/*        InfoNES_SoundInit() : Sound Emulation Initialize           */
/*                                                                   */
/*===================================================================*/
void InfoNES_SoundInit( void )
{
}

/*===================================================================*/
/*                                                                   */
/*        InfoNES_SoundOpen() : Sound Open                           */
/*                                                                   */
/*===================================================================*/
int InfoNES_SoundOpen( int samples_per_sync, int sample_rate )
{
  //lpSndDevice = new DIRSOUND( hWndMain );
  lpSoundPool = BufferSound_Create(
#ifdef WIN32
		  hWndMain,
#endif
		  1,
		  sample_rate,
		  8,
		  sample_rate * 3 / 4,
		  TRUE );

//  if ( !lpSndDevice->SoundOpen( samples_per_sync, sample_rate ) )
	if( 0 == lpSoundPool )
	{
//		InfoNES_MessageBox( "SoundOpen() Failed." );
//		exit(0);
		return FALSE;
	}

	// if sound mute, stop sound
	if ( APU_Mute )
	{
//		if (!lpSndDevice->SoundMute( APU_Mute ) )
		if( !BufferSound_SetMute(lpSoundPool, APU_Mute ) )
		{
//			InfoNES_MessageBox( "SoundMute() Failed." );
//			exit(0);
			return FALSE;
		}
	}
	return(TRUE);

}

/*===================================================================*/
/*                                                                   */
/*        InfoNES_SoundClose() : Sound Close                         */
/*                                                                   */
/*===================================================================*/
void InfoNES_SoundClose( void )
{
/*
#ifdef WIN32
	lpSndDevice->SoundClose();
	delete lpSndDevice;
#else
	InfoNES_soundDestroyEngine(soundPool);
#endif
*/
	BufferSound_Destroy( lpSoundPool );
	lpSoundPool = NULL;
}

/*===================================================================*/
/*                                                                   */
/*            InfoNES_SoundOutput4() : Sound Output 4 Waves          */
/*                                                                   */
/*===================================================================*/
void InfoNES_SoundOutput( int samples, 
								BYTE *wave1,
								BYTE *wave2,
								BYTE *wave3,
								BYTE *wave4,
								BYTE *wave5 )
{
	int i;
	BYTE wave[ rec_freq ];

	if( APU_Mute )
	{
		return;
	}

	for (i = 0; i < rec_freq; i++)
	{
//		wave[i] = ( wave1[i] + wave2[i] + wave3[i] + wave4[i] ) >> 2;
		wave[i] = ( wave1[i] + wave2[i] + wave3[i] + wave4[i] + wave5[i] ) / 5;
	}
#if 1
//  if (!lpSndDevice->SoundOutput( samples, wave ) )
  if( !BufferSound_Play( lpSoundPool, wave, samples ))
#else
//  if (!lpSndDevice->SoundOutput( samples, wave3 ) )
  if( !BufferSound_Play( lpSoundPool, wave3, samples ) )
#endif
	{
		return;
//		InfoNES_MessageBox( "SoundOutput() Failed." );
//		exit(0);
	}
}

/*===================================================================*/
/*                                                                   */
/*        InfoNES_SoundMute() :            */
/*                                                                   */
/*===================================================================*/
void InfoNES_SetMute( BOOL bMute )
{
	APU_Mute = bMute;
//	InfoNES_soundSetMute(soundPool, bMute);
	BufferSound_SetMute(lpSoundPool, bMute );
}

/*===================================================================*/
/*                                                                   */
/*        InfoNES_SoundVolume() :          */
/*        vol : range in [0.0, 1.0]                                                      */
/*===================================================================*/
void InfoNES_SetVolume( jfloat vol )
{
//	InfoNES_soundSetVolume(soundPool, vol);
	BufferSound_SetVolume(lpSoundPool, vol );
}

/*===================================================================*/
/*                                                                   */
/*            InfoNES_StartTimer() : Start MM Timer                  */
/*                                                                   */
/*===================================================================*/
static BOOL InfoNES_StartTimer()
{
#ifdef WIN32
	TIMECAPS caps;

	timeGetDevCaps( &caps, sizeof(caps) );
	timeBeginPeriod( caps.wPeriodMin );

	uTimerID =
		timeSetEvent( caps.wPeriodMin * TIMER_PER_LINE, caps.wPeriodMin, TimerFunc, 0, (UINT)TIME_PERIODIC );

	// Calculate proper timing
	wLinePerTimer = LINE_PER_TIMER * caps.wPeriodMin;



#else

	struct sigevent sev;
	struct itimerspec its;
	long long freq_nanosecs = TIMER_PER_LINE*1000000;

	/* Create the timer */
	LOGV("InfoNES_StartTimer");
	if( 0 == uTimerID )
	{
		sev.sigev_notify = SIGEV_THREAD;
		sev.sigev_value.sival_ptr = &uTimerID;
		sev.sigev_notify_function = TimerFunc;
		sev.sigev_notify_attributes = NULL;
		if (timer_create(CLOCK_REALTIME, &sev, &uTimerID) == -1)
		{
			return 0;
		}
		/* Start the timer */
		its.it_value.tv_sec = freq_nanosecs / 1000000000;
		its.it_value.tv_nsec = freq_nanosecs % 1000000000;
		its.it_interval.tv_sec = its.it_value.tv_sec;
		its.it_interval.tv_nsec = its.it_value.tv_nsec;
		if (timer_settime(uTimerID, 0, &its, NULL) == -1)
		{
			return 0;
		}
	}
	LOGV("InfoNES_StartTimer finished: uTimerID=%d", uTimerID);

	// Calculate proper timing
	wLinePerTimer = LINE_PER_TIMER;
#endif
	// Initialize timer variables
	wLines = 0;
	bWaitFlag = TRUE;

	bPauseFlag = FALSE;
	// Initialize Critical Section Object

	return 1;
}


/*===================================================================*/
/*                                                                   */
/*            InfoNES_StopTimer() : Stop MM Timer                    */
/*                                                                   */
/*===================================================================*/
static void InfoNES_StopTimer()
{
	LOGV("InfoNES_StopTimer");
	if ( 0 != uTimerID )
	{
#ifdef WIN32
		TIMECAPS caps;
		timeKillEvent( uTimerID );
		uTimerID = 0;
		timeGetDevCaps( &caps, sizeof(caps) );
		timeEndPeriod( caps.wPeriodMin * TIMER_PER_LINE );
#else
		timer_delete( uTimerID );
		uTimerID = 0;
#endif
	}
	// Delete Critical Section Object
	LOGV("InfoNES_StopTimer finished");
}

/*===================================================================*/
/*                                                                   */
/*           TimerProc() : MM Timer Callback Function                */
/*                                                                   */
/*===================================================================*/
#ifdef WIN32
static void CALLBACK TimerFunc( UINT nID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	if ( NULL != m_hThread )
	{
		NES_MUTEX_LOCK( WaitFlagCriticalSection );
		bWaitFlag = FALSE;
		NES_MUTEX_UNLOCK( WaitFlagCriticalSection );
	}
}
#else
static void TimerFunc(void *arg)
{
	NES_CONTEXT *pNesContext = InfoNES_GetContext();
	if ( 0 != pNesContext->threadNes )
	{
		NES_MUTEX_LOCK( WaitFlagCriticalSection );
		bWaitFlag = FALSE;
		NES_MUTEX_UNLOCK( WaitFlagCriticalSection );
	}
}
#endif
/*===================================================================*/
/*                                                                   */
/*            InfoNES_Wait() : Wait Emulation if required            */
/*                                                                   */
/*===================================================================*/
void InfoNES_Wait(void)
{
	wLines++;
	if ( wLines < wLinePerTimer )
		return;
	wLines = 0;

	if ( bAutoFrameskip )
	{
		// Auto Frameskipping
		if ( !bWaitFlag )
		{
			// Increment Frameskip Counter
			FrameSkip++;
		}
#if 1
		else
		{
			// Decrement Frameskip Counter
			if ( FrameSkip > 2 )
			{
				FrameSkip--;
			}
		}
#endif
	}

	// Wait if bWaitFlag is TRUE
	if ( bAutoFrameskip )
	{
		while ( bWaitFlag )
		{
			NES_SLEEP_MS(1); // wait 1 ms
		}
	}

	// set bWaitFlag is TRUE
	NES_MUTEX_LOCK( WaitFlagCriticalSection );
	bWaitFlag = TRUE;
	NES_MUTEX_UNLOCK( WaitFlagCriticalSection );

}

/*===================================================================*/
/*                                                                   */
/*            InfoNES_MessageBox() : Print System Message            */
/*                                                                   */
/*===================================================================*/
void InfoNES_MessageBox( char *pszMsg )
{
#ifdef WIN32
	MessageBox( hWndMain, pszMsg, APP_NAME, MB_OK | MB_ICONSTOP );
#endif
}

#ifdef _DEBUG
void DebugTrace( const char * lpszFormat, ... )
{

 va_list argList;
 va_start( argList, lpszFormat );
 
 char chInput[1024] = {0};
 vsprintf( chInput, lpszFormat, argList );
 
 va_end(argList);

 OutputDebugString( chInput );
 OutputDebugString( "\n" );
}

#endif

jobject makeGlobalRef(JNIEnv* pEnv, const jobject localRef) {
	LOGV("makeGlobalRef(%08X)\n", localRef);
	if (localRef != NULL) {
		jobject lGlobalRef = (*pEnv)->NewGlobalRef(pEnv, localRef);
		// No need for a local reference any more.
		(*pEnv)->DeleteLocalRef(pEnv, localRef);
		// Here, lGlobalRef may be null.
		return lGlobalRef;
	}
	return NULL;
}
void deleteGlobalRef(JNIEnv* pEnv, jobject* pRef) {
	if (*pRef != NULL) {
		(*pEnv)->DeleteGlobalRef(pEnv, *pRef);
		*pRef = NULL;
	}
}

void *InfoNES_Proc(void *pArgs)
{
	JNIEnv* pEnv;
	NES_CONTEXT *pNesContext = (NES_CONTEXT *)pArgs;
	LOGV("InfoNES_Proc start, pNesContext = %x, thread id = %d", pNesContext, pthread_self() );

	JavaVMAttachArgs lJavaVMAttachArgs;
	lJavaVMAttachArgs.version = JNI_VERSION_1_6;
	lJavaVMAttachArgs.name = "NativeThread";
	lJavaVMAttachArgs.group = NULL;
	if ((*(pNesContext->pJavaVM))->AttachCurrentThread(pNesContext->pJavaVM, &pEnv, &lJavaVMAttachArgs)
			!= JNI_OK) {
		LOGV("InfoNES_Proc failed 1");
		pthread_exit(NULL);
	}

	InfoNES_Main();
//	pthread_exit((void *)1);
	LOGV("InfoNES_Proc finish, pNesContext = %x, thread id = %d", pNesContext, pNesContext->threadNes );
	(*(pNesContext->pJavaVM))->DetachCurrentThread(pNesContext->pJavaVM);
	pthread_exit(NULL);
}

int InfoNES_PrintErrInfo(const char *errInfo,...)
{
	int ret = 0;
	char dbgfilename[MAX_PATH];
	if( errInfo)
	{
		FILE *fp;
		va_list args;

		va_start( args, errInfo );
		ret = vsnprintf( szErrInfo, sizeof(szErrInfo), errInfo, args );

		strcpy( dbgfilename, "/mnt/external_sd/nes_debug.log" );
		fp = fopen(dbgfilename, "a+");
		if( fp )
		{
			vfprintf(fp, errInfo, args);
			fclose(fp);
		}
		LOGV(errInfo, args);
		va_end( args );
	}
	return ret;
}

const char *InfoNES_GetErrInfo()
{
	return szErrInfo;
}
