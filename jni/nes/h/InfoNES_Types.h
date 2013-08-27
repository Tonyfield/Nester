/*===================================================================*/
/*                                                                   */
/*  InfoNES_Types.h : Type definitions for InfoNES                   */
/*                                                                   */
/*  2000/5/4    InfoNES Project ( based on pNesX )                   */
/*                                                                   */
/*===================================================================*/

#ifndef InfoNES_TYPES_H_INCLUDED
#define InfoNES_TYPES_H_INCLUDED

/*-------------------------------------------------------------------*/
/*  Type definition                                                  */
/*-------------------------------------------------------------------*/
#ifndef BOOL
typedef unsigned char  BOOL;
#endif /* !DWORD */

#ifndef TRUE
#define TRUE      1
#endif /* !TRUE */
#ifndef FALSE
#define FALSE     0
#endif /* !FALSE */

#ifndef BYTE
#define BYTE unsigned char
#endif

#ifndef WORD
#define WORD  unsigned short
#endif

#ifndef DWORD
#define DWORD  unsigned long
#endif

#ifndef MAX_PATH
#define MAX_PATH 1024
#endif

/* UINT8 defines a single byte */
#ifndef UINT8
#define UINT8 unsigned char
#endif

/* UINT16 defines a two byte word */
#ifndef UINT16
#define UINT16 unsigned short
#endif

/* UINT32 defines a four byte word */
#ifndef UINT32
#define UINT32 unsigned long
#endif

/* INT8 defines a single byte */
#ifndef INT8
#define INT8  char
#endif

/* INT16 defines a two byte word */
#ifndef INT16
#define INT16  short
#endif

/* INT32 defines a two byte word */
#ifndef INT32
#define INT32  long
#endif

/*-------------------------------------------------------------------*/
/*  NULL definition                                                  */
/*-------------------------------------------------------------------*/
#ifndef NULL
#define NULL  0
#endif /* !NULL */

#endif /* !InfoNES_TYPES_H_INCLUDED */
