/*
 * GENERATED FILE - DO NOT EDIT
 * Copyright (c) 2008-2013 Code Red Technologies Ltd,
 * Copyright 2015, 2018-2019 NXP
 * (c) NXP Semiconductors 2013-2019
 * Generated linker script file for LPC1549
 * Created from memory.ldt by FMCreateLinkMemory
 * Using Freemarker v2.3.23
 * MCUXpresso IDE v11.0.0 [Build 2516] [2019-06-05] on Aug 27, 2019 11:46:48 AM
 */

MEMORY
{
  /* Define each memory region */
  MFlash256 (rx) : ORIGIN = 0x0, LENGTH = 0x40000 /* 256K bytes (alias Flash) */  
  Ram0_16 (rwx) : ORIGIN = 0x2000000, LENGTH = 0x4000 /* 16K bytes (alias RAM) */  
  Ram1_16 (rwx) : ORIGIN = 0x2004000, LENGTH = 0x4000 /* 16K bytes (alias RAM2) */  
  Ram2_4 (rwx) : ORIGIN = 0x2008000, LENGTH = 0x1000 /* 4K bytes (alias RAM3) */  
}

  /* Define a symbol for the top of each memory region */
  __base_MFlash256 = 0x0  ; /* MFlash256 */  
  __base_Flash = 0x0 ; /* Flash */  
  __top_MFlash256 = 0x0 + 0x40000 ; /* 256K bytes */  
  __top_Flash = 0x0 + 0x40000 ; /* 256K bytes */  
  __base_Ram0_16 = 0x2000000  ; /* Ram0_16 */  
  __base_RAM = 0x2000000 ; /* RAM */  
  __top_Ram0_16 = 0x2000000 + 0x4000 ; /* 16K bytes */  
  __top_RAM = 0x2000000 + 0x4000 ; /* 16K bytes */  
  __base_Ram1_16 = 0x2004000  ; /* Ram1_16 */  
  __base_RAM2 = 0x2004000 ; /* RAM2 */  
  __top_Ram1_16 = 0x2004000 + 0x4000 ; /* 16K bytes */  
  __top_RAM2 = 0x2004000 + 0x4000 ; /* 16K bytes */  
  __base_Ram2_4 = 0x2008000  ; /* Ram2_4 */  
  __base_RAM3 = 0x2008000 ; /* RAM3 */  
  __top_Ram2_4 = 0x2008000 + 0x1000 ; /* 4K bytes */  
  __top_RAM3 = 0x2008000 + 0x1000 ; /* 4K bytes */  
