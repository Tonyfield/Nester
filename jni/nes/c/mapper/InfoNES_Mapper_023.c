/*===================================================================*/
/*                                                                   */
/*                  Mapper 23 (Konami VRC2 type B)                   */
/*                                                                   */
/*===================================================================*/

BYTE Map23_Regs[ 9 ];

BYTE Map23_IRQ_Enable;
BYTE Map23_IRQ_Cnt;
BYTE Map23_IRQ_Latch;

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 23                                             */
/*-------------------------------------------------------------------*/
void Map23_Init()
{
  int nPage;

  /* Initialize Mapper */
  MapperInit = Map23_Init;

  /* Write to Mapper */
  MapperWrite = Map23_Write;

  /* Write to SRAM */
  MapperSram = Map0_Sram;

  /* Write to APU */
  MapperApu = Map0_Apu;

  /* Read from APU */
  MapperReadApu = Map0_ReadApu;

  /* Callback at VSync */
  MapperVSync = Map0_VSync;

  /* Callback at HSync */
  MapperHSync = Map23_HSync;

  /* Callback at PPU */
  MapperPPU = Map0_PPU;

  /* Callback at Rendering Screen ( 1:BG, 0:Sprite ) */
  MapperRenderScreen = Map0_RenderScreen;

  /* Set SRAM Banks */
  SRAMBANK = SRAM;

  /* Set ROM Banks */
  ROMBANK0 = ROMPAGE( 0 );
  ROMBANK1 = ROMPAGE( 1 );
  ROMBANK2 = ROMLASTPAGE( 1 );
  ROMBANK3 = ROMLASTPAGE( 0 );

  /* Set PPU Banks */
  if ( NesHeader.byVRomSize > 0 )
  {
    for ( nPage = 0; nPage < 8; ++nPage )
      PPUBANK[ nPage ] = VROMPAGE( nPage );
    InfoNES_SetupChr();
  }

  /* Initialize State Flag */
  /* Initialize State Registers */
  for ( nPage = 0; nPage < 8; nPage++ )
  {
    Map23_Regs[ nPage ] = nPage;
  }
  Map23_Regs[ 8 ] = 0;

  Map23_IRQ_Enable = 0;
  Map23_IRQ_Cnt = 0;
  Map23_IRQ_Latch = 0;

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 23 Write Function                                         */
/*-------------------------------------------------------------------*/
void Map23_Write( WORD wAddr, BYTE byData )
{
  int bank;
  switch ( wAddr )
  {
    case 0x8000:
    case 0x8004:
    case 0x8008:
    case 0x800c:
      byData %= ( NesHeader.byRomSize << 1 );

      if ( Map23_Regs[ 8 ] )
      {
        ROMBANK2 = ROMPAGE( byData );
      } else {
        ROMBANK0 = ROMPAGE( byData );
      }
      break;

    case 0x9000:
      switch ( byData & 0x03 )
      {
       case 0x00:
          InfoNES_Mirroring( 1 );
          break;
        case 0x01:
          InfoNES_Mirroring( 0 );
          break;
        case 0x02:
          InfoNES_Mirroring( 3 );
          break;
        case 0x03:
          InfoNES_Mirroring( 2 );
          break;
      }
      break;

    case 0x9008:
      Map23_Regs[ 8 ] = byData & 0x02;
      break;

    case 0xa000:
    case 0xa004:
    case 0xa008:
    case 0xa00c:
      byData %= ( NesHeader.byRomSize << 1 );
      ROMBANK1 = ROMPAGE( byData );
      break;

    case 0xB000:
    case 0xB002:
    case 0xB008:
    case 0xC000:
    case 0xC002:
    case 0xC008:
    case 0xD000:
    case 0xD002:
    case 0xD008:
    case 0xE000:
    case 0xE002:
    case 0xE008:
	  bank=((wAddr>>12)-0xB)*2+((wAddr&2)>>1)+((wAddr&8)>>3);
      Map23_Regs[ bank ] = ( Map23_Regs[ bank ] & 0xf0 ) | ( byData & 0x0f );
      PPUBANK[ bank ] = VROMPAGE( Map23_Regs[ bank ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    case 0xB001:
    case 0xB004:
    case 0xB003:
    case 0xB00C:
    case 0xC001:
    case 0xC004:
    case 0xC003:
    case 0xC00C:
    case 0xD001:
    case 0xD004:
    case 0xD003:
    case 0xD00C:
    case 0xE001:
    case 0xE004:
    case 0xE003:
    case 0xE00C:
	  bank=((wAddr>>12)-0xB)*2+((wAddr&2)>>1)+((wAddr&8)>>3);
      Map23_Regs[ bank ] = ( Map23_Regs[ bank ] & 0x0f ) | ( ( byData & 0x0f ) << 4 );
      PPUBANK[ bank ] = VROMPAGE( Map23_Regs[ bank ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    case 0xf000:
      Map23_IRQ_Latch = ( Map23_IRQ_Latch & 0xf0 ) | ( byData & 0x0f );
      break;

    case 0xf004:
      Map23_IRQ_Latch = ( Map23_IRQ_Latch & 0xf0 ) | ( ( byData & 0x0f ) << 4 );
      break;

    case 0xf008:
      Map23_IRQ_Enable = byData & 0x03;
      if ( Map23_IRQ_Enable & 0x02 )
      {
        Map23_IRQ_Cnt = Map23_IRQ_Latch;
      }
      break;

    case 0xf00c:
      if ( Map23_IRQ_Enable & 0x01 )
      {
        Map23_IRQ_Enable |= 0x02;
      } else {
        Map23_IRQ_Enable &= 0x01;
      }
      break;
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 23 H-Sync Function, no reference                                    */
/*-------------------------------------------------------------------*/
void Map23_HSync()
{
/*
 *  Callback at HSync
 *
 */
  if ( Map23_IRQ_Enable & 0x02 )
  {
	if ( Map23_IRQ_Cnt == 0xff )
	{
	  IRQ_REQ;

	  Map23_IRQ_Cnt = Map23_IRQ_Latch;
	  if ( Map23_IRQ_Enable & 0x01 )
	  {
		Map23_IRQ_Enable |= 0x02; // set bit1 = 1
	  } else {
		Map23_IRQ_Enable &= 0x01; // set bit1 = 0
	  }
	} else {
	  Map23_IRQ_Cnt++;
	}
  }

}
