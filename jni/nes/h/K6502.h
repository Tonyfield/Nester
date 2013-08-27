/*===================================================================*/
/*                                                                   */
/*  K6502.h : Header file for K6502                                  */
/*                                                                   */
/*  2000/05/29  InfoNES Project ( based on pNesX )                   */
/*                                                                   */
/*===================================================================*/

#ifndef K6502_H_INCLUDED
#define K6502_H_INCLUDED

// Type definition
#ifndef DWORD
typedef unsigned long  DWORD;
#endif

#ifndef WORD
typedef unsigned short WORD;
#endif

#ifndef BYTE
typedef unsigned char  BYTE;
#endif

#ifndef NULL
#define NULL 0
#endif

/* 6502 Flags */
#define FLAG_C 0x01
#define FLAG_Z 0x02
#define FLAG_I 0x04
#define FLAG_D 0x08
#define FLAG_B 0x10
#define FLAG_R 0x20
#define FLAG_V 0x40
#define FLAG_N 0x80

//#define IRQ_NMI_SLIM
/* Stack Address */
#define BASE_STACK   0x100

/* Interrupt Vectors */
#define VECTOR_NMI   0xfffa
#define VECTOR_RESET 0xfffc
#define VECTOR_IRQ   0xfffe

#ifdef IRQ_NMI_SLIM
#define NMI_MASK (1<<1)
#define IRQ_MASK (1<<0)
#endif
// NMI Request
#ifdef IRQ_NMI_SLIM
#define NMI_REQ  IRQ_NMI_State &= ~NMI_MASK;
#else
#define NMI_REQ  NMI_State = 0;
#endif
// IRQ Request
#ifdef IRQ_NMI_SLIM
#define IRQ_REQ  IRQ_NMI_State &= ~IRQ_MASK;
#else
#define IRQ_REQ  IRQ_State = 0;
#endif

// Emulator Operation
void K6502_Init(void);
void K6502_Reset(void);
void K6502_Set_Int_Wiring( BYTE byNMI_Wiring, BYTE byIRQ_Wiring );
void K6502_Step( register WORD wClocks );

struct K6502_REG_T{
	WORD PC;
	BYTE SP;
	BYTE F;
	BYTE A;
	BYTE X;
	BYTE Y;
#ifdef IRQ_NMI_SLIM
	BYTE IRQ_NMI_State; // bit 0: IRQ,   bit 1: NMI
	BYTE IRQ_NMI_Wiring; // bit 0: IRQ,   bit 1: NMI
#else
	// The state of the IRQ pin
	BYTE IRQ_State;
	// Wiring of the IRQ pin
	BYTE IRQ_Wiring;
	// The state of the NMI pin
	BYTE NMI_State;
	// Wiring of the NMI pin
	BYTE NMI_Wiring;
#endif
	WORD g_wPassedClocks;   // The number of the clocks that it passed
};
void K6502_GetReg(struct K6502_REG_T *reg);
void K6502_SetReg(const struct K6502_REG_T *reg);

// I/O Operation (User definition)
static __inline BYTE K6502_Read( WORD wAddr);
static __inline WORD K6502_ReadW( WORD wAddr );
static __inline WORD K6502_ReadW2( WORD wAddr );
static __inline BYTE K6502_ReadZp( BYTE byAddr );
static __inline WORD K6502_ReadZpW( BYTE byAddr );
static __inline BYTE K6502_ReadAbsX(void);
static __inline BYTE K6502_ReadAbsY(void);
static __inline BYTE K6502_ReadIY(void);

static __inline void K6502_Write( WORD wAddr, BYTE byData );
static __inline void K6502_WriteW( WORD wAddr, WORD wData );

#ifdef IRQ_NMI_SLIM
extern BYTE IRQ_NMI_State;
#else
// The state of the IRQ pin
extern BYTE IRQ_State;

// The state of the NMI pin
extern BYTE NMI_State;
#endif

// The number of the clocks that it passed
extern WORD g_wPassedClocks;

#endif /* !K6502_H_INCLUDED */
