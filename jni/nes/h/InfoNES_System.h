/*===================================================================*/
/*                                                                   */
/*  InfoNES_System.h : The function which depends on a system        */
/*                                                                   */
/*  2000/05/29  InfoNES Project ( based on pNesX )                   */
/*                                                                   */
/*===================================================================*/

#ifndef InfoNES_SYSTEM_H_INCLUDED
#define InfoNES_SYSTEM_H_INCLUDED

/*-------------------------------------------------------------------*/
/*  Include files                                                    */
/*-------------------------------------------------------------------*/

#include "InfoNES_Types.h"

#ifdef WIN32
#define NES_MUTEX_TYPE       CRITICAL_SECTION
#define NES_MUTEX_CREATE(mutex)      do{ InitializeCriticalSection(&mutex); }while(0)
#define NES_MUTEX_LOCK(mutex)           do{ EnterCriticalSection(&mutex); }while(0)
#define NES_MUTEX_UNLOCK(mutex)       do{ LeaveCriticalSection(&mutex); }while(0)
#define NES_MUTEX_DESTROY(mutex)     do{ DeleteCriticalSection(&mutex); }while(0)

//#define NES_SIGNAL_TYPE
//#define NES_SIGNAL_WAIT(sig)     do{ DeleteCriticalSection(&mutex); }while(0)


#define NES_SLEEP_MS(ms)     do{ Sleep(ms); }while(0)

#elif defined __linux__
#include <pthread.h>
#include <semaphore.h>

#define NES_MUTEX_TYPE                       pthread_mutex_t
#define NES_MUTEX_CREATE(mutex)      do{ pthread_mutex_init(&(mutex), NULL); }while(0)
#define NES_MUTEX_LOCK(mutex)          do{ pthread_mutex_lock(&(mutex)); }while(0)
#define NES_MUTEX_UNLOCK(mutex)      do{ pthread_mutex_unlock(&(mutex)); }while(0)
#define NES_MUTEX_DESTROY(mutex)    do{ pthread_mutex_destroy(&(mutex)); }while(0)

#define NES_SEM_TYPE                              sem_t
#define NES_SEM_INIT(sem, init_value)   do{ sem_init (&(sem), 0,  (init_value)); }while(0)
#define NES_SEM_WAIT(sem)                    do{ LOGV("sem_wait");  sem_wait (&(sem)); }while(0)
#define NES_SEM_POST(sem)                    do{ LOGV("sem_post"); sem_post (&(sem)); }while(0)
#define NES_SEM_DESTROY(sem)              do{ sem_destroy (&(sem)); }while(0)

#define NES_SLEEP_MS(ms)     do{ usleep(ms*1000); }while(0)
#else
#error not support your platform!
#endif

/*-------------------------------------------------------------------*/
/*  Palette data                                                     */
/*-------------------------------------------------------------------*/
extern const WORD NesPalette[];

/*-------------------------------------------------------------------*/
/*  Function prototypes                                              */
/*-------------------------------------------------------------------*/

/* Menu screen */
int InfoNES_QuitFlag(void);

/* Read ROM image file */
int InfoNES_ReadRom( const char *pszFileName );

/* Release a memory for ROM */
void InfoNES_ReleaseRom(void);

/* Transfer the contents of work frame on the screen */
void InfoNES_LoadFrame(void);

/* Get a joypad state */
void InfoNES_PadState( DWORD *pdwPad1, DWORD *pdwPad2, DWORD *pdwSystem );

/* memcpy */
void *InfoNES_MemoryCopy( void *dest, const void *src, int count );

/* memset */
void *InfoNES_MemorySet( void *dest, int c, int count );

/* Print debug message */
void InfoNES_DebugPrint( char *pszMsg );

/* Wait */
void InfoNES_Wait(void);

/* Sound Initialize */
void InfoNES_SoundInit( void );

/* Sound Open */
int InfoNES_SoundOpen( int samples_per_sync, int sample_rate );

/* Sound Close */
void InfoNES_SoundClose( void );

/* Sound Output 5 Waves - 2 Pulse, 1 Triangle, 1 Noise, 1 DPCM */
void InfoNES_SoundOutput(int samples, BYTE *wave1, BYTE *wave2, BYTE *wave3, BYTE *wave4, BYTE *wave5);

void * InfoNES_GetFramePointer(void);

/* Print system message */
#ifdef _UNIX
void InfoNES_MessageBox( char *pszMsg, ... );
#else
void InfoNES_MessageBox( char *pszMsg );
#endif


#endif /* !InfoNES_SYSTEM_H_INCLUDED */
