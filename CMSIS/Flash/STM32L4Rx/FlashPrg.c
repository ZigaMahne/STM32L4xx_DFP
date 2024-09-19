/* -----------------------------------------------------------------------------
 * Copyright (c) 2014 - 2023 ARM Ltd.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software. Permission is granted to anyone to use this
 * software for any purpose, including commercial applications, and to alter
 * it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *
 * $Date:        28. March 2023
 * $Revision:    V1.1.0
 *
 * Project:      Flash Programming Functions for ST STM32L4Rx/Sx Flash
 * --------------------------------------------------------------------------- */

/* History:
 *  Version 1.1.0
 *    Reworked algorithms
 *  Version 1.0.0
 *    Initial release
 */

/* Note:
   Flash has 4k (dual bank), 8K (single bank) sector size.
   STM32L4Rx devices have Dual Bank Flash configuration. */

#include "..\FlashOS.h"        /* FlashOS Structures */

typedef volatile unsigned long    vu32;
typedef          unsigned long     u32;

#define M32(adr) (*((vu32 *) (adr)))

/* Peripheral Memory Map */
#define FLASH_BASE       (0x40022000)
#define DBGMCU_BASE      (0xE0042000)
#define FLASHSIZE_BASE   (0x1FFF75E0)
#define IWDG_BASE        (0x40003000)
#define WWDG_BASE        (0x40002C00)

#define FLASH           ((FLASH_TypeDef  *) FLASH_BASE)
#define DBGMCU          ((DBGMCU_TypeDef *) DBGMCU_BASE)
#define IWDG            ((IWDG_TypeDef   *) IWDG_BASE)
#define WWDG            ((WWDG_TypeDef   *) WWDG_BASE)

/* Debug MCU */
typedef struct
{
  vu32 IDCODE;
} DBGMCU_TypeDef;

/* IWDG Registers */
typedef struct
{
  vu32 KR;               /* offset: 0x000 IWDG Key register */
  vu32 PR;               /* offset: 0x004 IWDG Prescaler register */
  vu32 RLR;              /* offset: 0x008 IWDG Reload register */
  vu32 SR;               /* offset: 0x00C IWDG Status register */
} IWDG_TypeDef;

/* WWDG Registers */
typedef struct
{
  vu32 CR;               /* offset: 0x000 WWDG Control register */
  vu32 CFR;              /* offset: 0x004 WWDG Configuration register */
  vu32 SR;               /* offset: 0x008 WWDG Status register */
} WWDG_TypeDef;

/* Flash Registers */
typedef struct
{
  vu32 ACR;              /* offset: 0x000 FLASH access control register */
  vu32 PDKEYR;           /* offset: 0x004 FLASH power down key register */
  vu32 KEYR;             /* offset: 0x008 FLASH key register */
  vu32 OPTKEYR;          /* offset: 0x00C FLASH option key register */
  vu32 SR;               /* offset: 0x010 FLASH status register */
  vu32 CR;               /* offset: 0x014 FLASH control register */
  vu32 ECCR;             /* offset: 0x018 FLASH ECC register */
  vu32 RESERVED1;
  vu32 OPTR;             /* offset: 0x020 FLASH option register */
  vu32 PCROP1SR;         /* offset: 0x024 FLASH bank1 PCROP start address register */
  vu32 PCROP1ER;         /* offset: 0x028 FLASH bank1 PCROP end address register */
  vu32 WRP1AR;           /* offset: 0x02C FLASH bank1 WRP area A address register */
  vu32 WRP1BR;           /* offset: 0x030 FLASH bank1 WRP area B address register */
  vu32 RESERVED2[4];
  vu32 PCROP2SR;         /* offset: 0x044 FLASH bank2 PCROP start address register */
  vu32 PCROP2ER;         /* offset: 0x048 FLASH bank2 PCROP end address register */
  vu32 WRP2AR;           /* offset: 0x04C FLASH bank2 WRP area A address register */
  vu32 WRP2BR;           /* offset: 0x050 FLASH bank2 WRP area B address register */
  vu32 RESERVED3[55];    /* offset: 0x054-0x12C Reserved3 */
  vu32 CFGR;             /* offset: 0x130 FLASH configuration register */
} FLASH_TypeDef;

/* Flash Keys */
#define FLASH_KEY1               0x45670123
#define FLASH_KEY2               0xCDEF89AB
#define FLASH_OPTKEY1            0x08192A3B
#define FLASH_OPTKEY2            0x4C5D6E7F

/* Flash Control Register definitions */
#define FLASH_CR_PG             ((u32)(  1U      ))
#define FLASH_CR_PER            ((u32)(  1U <<  1))
#define FLASH_CR_MER1           ((u32)(  1U <<  2))
#define FLASH_CR_PNB_MSK        ((u32)(0xFF <<  3))
#define FLASH_CR_BKER           ((u32)(  1U << 11))
#define FLASH_CR_MER2           ((u32)(  1U << 15))
#define FLASH_CR_STRT           ((u32)(  1U << 16))
#define FLASH_CR_OPTSTRT        ((u32)(  1U << 17))
#define FLASH_CR_FSTPG          ((u32)(  1U << 18))
#define FLASH_CR_OBL_LAUNCH     ((u32)(  1U << 27))
#define FLASH_CR_OPTLOCK        ((u32)(  1U << 30))
#define FLASH_CR_LOCK           ((u32)(  1U << 31))

/* Flash Status Register definitions */
#define FLASH_SR_EOP            ((u32)(  1U      ))
#define FLASH_SR_OPERR          ((u32)(  1U <<  1))
#define FLASH_SR_PROGERR        ((u32)(  1U <<  3))
#define FLASH_SR_WRPERR         ((u32)(  1U <<  4))
#define FLASH_SR_PGAERR         ((u32)(  1U <<  5))
#define FLASH_SR_SIZERR         ((u32)(  1U <<  6))
#define FLASH_SR_PGSERR         ((u32)(  1U <<  7))
#define FLASH_SR_MISSERR        ((u32)(  1U <<  8))
#define FLASH_SR_FASTERR        ((u32)(  1U <<  9))
#define FLASH_SR_RDERR          ((u32)(  1U << 14))
#define FLASH_SR_OPTVERR        ((u32)(  1U << 15))
#define FLASH_SR_BSY            ((u32)(  1U << 16))

#define FLASH_PGERR             (FLASH_SR_OPERR   | FLASH_SR_PROGERR | FLASH_SR_WRPERR  | \
                                 FLASH_SR_PGAERR  | FLASH_SR_SIZERR  | FLASH_SR_PGSERR  | \
                                 FLASH_SR_MISSERR | FLASH_SR_FASTERR | FLASH_SR_RDERR   | FLASH_SR_OPTVERR )

/* Flash option register definitions */
#define FLASH_OPTR_RDP          ((u32)(0xFF      ))
#define FLASH_OPTR_RDP_NO       ((u32)(0xAA      ))
#define FLASH_OPTR_IWDG_SW      ((u32)(  1U << 16))
#define FLASH_OPTR_WWDG_SW      ((u32)(  1U << 19))
#define FLASH_OPTR_DB1M         ((u32)(  1U << 21))
#define FLASH_OPTR_DBANK        ((u32)(  1U << 22))


#if defined FLASH_MEM
static u32 gFlashBase;                 /* Flash base address */
static u32 gFlashSize;                 /* Flash size in bytes */
static u32 gFlashPageSize;             /* Flash page size in bytes */
#endif /* FLASH_MEM */

static void DSB(void)
{
  __asm ("dsb");
}

static void NOP(void)
{
  __asm ("nop");
}

//static void __enable_irq(void)
//{
//  __asm volatile ("cpsie i" : : : "memory");
//}

static void __disable_irq(void)
{
  __asm volatile ("cpsid i" : : : "memory");
}



/*
 * Get Flash Type
 *    Return Value:   0 = Single-Bank flash
 *                    1 = Dual-Bank Flash (configurable)
 */

#if defined FLASH_MEM
static u32 GetFlashType (void)
{
//  u32 flashType = 0U;
//
//  switch ((DBGMCU->IDCODE & 0xFFFU))
//  {
//    case 0x470:             /* Flash Category ? devices, 4k or 8k sectors (STM32L4Rxxx and STM32L4Sxxx) */
//    case 0x471:             /* Flash Category ? devices, 4k or 8k sectors (STM32L4P5xx and STM32L4Q5xx) */
//    default:                /* devices have a dual bank flash */
//      flashType = 1U;       /* Dual-Bank Flash type */
//    break;
//  }
//
//  return (flashType);
  return (1U);              /* always Dual-Bank Flash */
}
#endif /* FLASH_MEM */


/*
 * Get Flash Bank Mode
 *    Return Value:   0 = Single-Bank mode
 *                    1 = Dual-Bank mode
 */

#if defined FLASH_MEM
static u32 GetFlashBankMode (void)
{
  u32 flashBankMode;

  /* For 1-Mbyte Flash memory devices, do not care about DBANK */
  if (gFlashSize > 0x00100000)
  {
    flashBankMode = (FLASH->OPTR & FLASH_OPTR_DBANK) ? 1U : 0U;
  }
  else
  {
    flashBankMode = (FLASH->OPTR & FLASH_OPTR_DB1M)  ? 1U : 0U;
  }

  return (flashBankMode);
}
#endif /* FLASH_MEM */


/*
 * Get Flash Bank Number
 *    Parameter:      adr:  Sector Address
 *    Return Value:   Bank Number (0..1)
 *                    Flash bank size is always the half of the Flash size
 */

#if defined FLASH_MEM
static u32 GetFlashBankNum(u32 adr)
{
  u32 flashBankNum;

  if (GetFlashType() == 1U)
  {
    /* Dual-Bank Flash */
    if (GetFlashBankMode() == 1U)
    {
      /* Dual-Bank Flash configured as Dual-Bank */
      if (adr >= (gFlashBase + (gFlashSize >> 1)))
      {
        flashBankNum = 1U;
      }
      else
      {
        flashBankNum = 0U;
      }
    }
    else
    {
      /* Dual-Bank Flash configured as Single-Bank */
      flashBankNum = 0U;
    }
  }
  else
  {
    /* Single-Bank Flash */
    flashBankNum = 0U;
  }

  return (flashBankNum);
}
#endif /* FLASH_MEM */


/*
 * Get Flash Page Number
 *    Parameter:      adr:  Page Address
 *    Return Value:   Page Number (0..127/255)
 *                    Flash bank size is always the half of the Flash size
 */

#if defined FLASH_MEM
static u32 GetFlashPageNum (unsigned long adr)
{
  u32 flashPageNum;

  if (GetFlashType() == 1U)
  {
    /* Dual-Bank Flash */
    if (GetFlashBankMode() == 1U)
    {
      /* Dual-Bank Flash configured as Dual-Bank */
      flashPageNum = (((adr & ((gFlashSize >> 1) - 1U)) ) >> 12); /* 4K sector size */
    }
    else
    {
      /* Dual-Bank Flash configured as Single-Bank */
      flashPageNum = (((adr & (gFlashSize        - 1U)) ) >> 13); /* 8K sector size */
    }
  }
  else
  {
      /* Single-Bank Flash */
      flashPageNum = (((adr & (gFlashSize        - 1U)) ) >> 13); /* 8K sector size */
  }

  return (flashPageNum);
}
#endif /* FLASH_MEM */


/*
 * Get Flash Page Size
 *    Return Value:   flash page size (in Bytes)
 */

#if defined FLASH_MEM
static u32 GetFlashPageSize (void)
{
  u32 flashPageSize;

  if (GetFlashType() == 1U)
  {
    /* Dual-Bank Flash */
    if (GetFlashBankMode() == 1U)
    {
      /* Dual-Bank Flash configured as Dual-Bank */
      flashPageSize = 0x1000;                            /* 4K sector size */
    }
    else
    {
      /* Dual-Bank Flash configured as Single-Bank */
      flashPageSize = 0x2000;                            /* 8K sector size */
    }
  }
  else
  {
    /* Single-Bank Flash */
      flashPageSize = 0x2000;                            /* 8K sector size */
  }

  return (flashPageSize);
}
#endif /* FLASH_MEM */


/*
 *  Initialize Flash Programming Functions
 *    Parameter:      adr:  Device Base Address
 *                    clk:  Clock Frequency (Hz)
 *                    fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int Init (unsigned long adr, unsigned long clk, unsigned long fnc)
{
  (void)clk;
  (void)fnc;

  __disable_irq();

#if defined FLASH_MEM
  /* Unlock Flash */
  FLASH->KEYR = FLASH_KEY1;
  FLASH->KEYR = FLASH_KEY2;

  /* Wait until the flash is ready */
  while (FLASH->SR & FLASH_SR_BSY) NOP();

  /* get Flash chraracteristics */
  gFlashBase = adr;
  gFlashSize = (M32(FLASHSIZE_BASE) & 0x0000FFFF) << 10;
  gFlashPageSize = GetFlashPageSize();
#endif /* FLASH_MEM */

#if defined FLASH_OPT
  (void)adr;

  /* not yet coded */
#endif /* FLASH_OPT */

  if ((FLASH->OPTR & FLASH_OPTR_IWDG_SW) == 0U) {        /* Test if IWDG is running (IWDG in HW mode) */
    /* Set IWDG time out to ~32.768 second */
    IWDG->KR  = 0xAAAA; 
    IWDG->KR  = 0x5555;                                  /* Enable write access to IWDG_PR and IWDG_RLR */
    IWDG->PR  = 6;                                       /* Set prescaler to 256 */
    IWDG->RLR = 4095;                                    /* Set reload value to 4095 */
  }

  if ((FLASH->OPTR & FLASH_OPTR_WWDG_SW) == 0U) {        /* Test if WWDG is running (WWDG_SW in HW mode) */
    WWDG->CFR = 0x1FF;
    WWDG->CR  = 0x7F;
  }

  return (0);
}


/*
 *  De-Initialize Flash Programming Functions
 *    Parameter:      fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int UnInit (unsigned long fnc)
{
  (void)fnc;

#if defined FLASH_MEM
  /* Lock Flash operation */
  FLASH->CR = FLASH_CR_LOCK;
  DSB();
  while (FLASH->SR & FLASH_SR_BSY) NOP();                /* Wait until operation is finished */
#endif /* FLASH_MEM */

#if defined FLASH_OPT
  /* not yet coded */
#endif /* FLASH_OPT */

  return (0);
}


/*
 *  Blank Check Checks if Memory is Blank
 *    Parameter:      adr:  Block Start Address
 *                    sz:   Block Size (in bytes)
 *                    pat:  Block Pattern
 *    Return Value:   0 - OK,  1 - Failed
 */

int BlankCheck (unsigned long adr, unsigned long sz, unsigned char pat)
{
  (void)adr;
  (void)sz;
  (void)pat;

#if defined FLASH_MEM
  /* force erase even if the content is 'Initial Content of Erased Memory'.
     Only a erased sector can be programmed. I think this is because of ECC */
  return (1);
#endif /* FLASH_MEM */

#if defined FLASH_OPT
  /* For OPT algorithm Flash is always erased */
  return (0);
#endif /* FLASH_OPT */
}


/*
 *  Erase complete Flash Memory
 *    Return Value:   0 - OK,  1 - Failed
 */

#if defined FLASH_MEM
int EraseChip (void)
{
  FLASH->SR = FLASH_PGERR;                               /* Reset Error Flags */

  FLASH->CR  = (FLASH_CR_MER1 | FLASH_CR_MER2);          /* Bank A/B mass erase enabled */
  FLASH->CR |=  FLASH_CR_STRT;                           /* Start erase */
  DSB();

  while (FLASH->SR & FLASH_SR_BSY) NOP();                /* Wait until operation is finished */

  if (FLASH->SR & FLASH_PGERR) {                         /* Check for Error */
    FLASH->SR  = FLASH_PGERR;                            /* Reset Error Flags */
    return (1);                                          /* Failed */
  }

  return (0);                                            /* Done */
}
#endif /* FLASH_MEM */

#ifdef FLASH_OPT
int EraseChip (void)
{

  /* not yet coded */
  return (0);                                            /* Done */
}
#endif /* FLASH_OPT */


/*
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

#if defined FLASH_MEM
int EraseSector (unsigned long adr)
{
  u32 b, p;

  b = GetFlashBankNum(adr);                              /* Get Bank Number 0..1  */
  p = GetFlashPageNum(adr);                              /* Get Page Number 0..127 */

  while (FLASH->SR & FLASH_SR_BSY) NOP();                /* Wait until operation is finished */

  FLASH->SR  = FLASH_PGERR;                              /* Reset Error Flags */

  FLASH->CR  = (FLASH_CR_PER |                           /* Page Erase Enabled */
                 (p <<  3)   |                           /* Page Number. 0 to 127 for each bank */
                 (b << 11)    );
  FLASH->CR |=  FLASH_CR_STRT;                           /* Start Erase */
  DSB();

  while (FLASH->SR & FLASH_SR_BSY) NOP();                /* Wait until operation is finished */

  if (FLASH->SR & FLASH_PGERR) {                         /* Check for Error */
    FLASH->SR  = FLASH_PGERR;                            /* Reset Error Flags */
    return (1);                                          /* Failed */
  }

  return (0);                                            /* Done */
}
#endif /* FLASH_MEM */

#if defined FLASH_OPT
int EraseSector (unsigned long adr)
{
  /* erase sector is not needed for Flash Option bytes */

  (void)adr;

  return (0);                                            /* Done */
}
#endif /* FLASH_OPT */


/*
 *  Program Page in Flash Memory
 *    Parameter:      adr:  Page Start Address
 *                    sz:   Page Size
 *                    buf:  Page Data
 *    Return Value:   0 - OK,  1 - Failed
 */

#if defined FLASH_MEM
int ProgramPage (unsigned long adr, unsigned long sz, unsigned char *buf)
{

  sz = (sz + 7) & ~7U;                                   /* Adjust size for four words */

  while (FLASH->SR & FLASH_SR_BSY) NOP();                /* Wait until operation is finished */

  FLASH->SR = FLASH_PGERR;                               /* Reset Error Flags */

  FLASH->CR = FLASH_CR_PG ;	                             /* Programming Enabled */

  while (sz)
  {
//    M32(adr    ) = *((u32 *)(buf + 0));                  /* Program the 1st word of the double-word */
//    M32(adr + 4) = *((u32 *)(buf + 4));                  /* Program the 2nd word of the double-word */

    M32(adr    ) = (u32)((*(buf+ 0)      ) |
                         (*(buf+ 1) <<  8) |
                         (*(buf+ 2) << 16) |
                         (*(buf+ 3) << 24) );            /* Program the 1st word of the double-word */
    M32(adr + 4) = (u32)((*(buf+ 4)      ) |
                         (*(buf+ 5) <<  8) |
                         (*(buf+ 6) << 16) |
                         (*(buf+ 7) << 24) );            /* Program the 2nd word of the double-word */
    DSB();

    while (FLASH->SR & FLASH_SR_BSY) NOP();              /* Wait until operation is finished */

    if (FLASH->SR & FLASH_PGERR) {                       /* Check for Error */
      FLASH->SR  = FLASH_PGERR;                          /* Reset Error Flags */
      return (1);                                        /* Failed */
    }

    adr += 8;                                            /* Next double-word */
    buf += 8;
    sz  -= 8;
  }

  FLASH->CR = 0U;                                        /* Reset CR */

  return (0);                                            /* Done */
}
#endif /* FLASH_MEM */


#ifdef FLASH_OPT
int ProgramPage (unsigned long adr, unsigned long sz, unsigned char *buf)
{
  (void)adr;
  (void)sz;

  /* not yet coded */
  return (0);                                            /* Done */
}
#endif /* FLASH_OPT */


/*
 *  Verify Flash Contents
 *    Parameter:      adr:  Start Address
 *                    sz:   Size (in bytes)
 *                    buf:  Data
 *    Return Value:   (adr+sz) - OK, Failed Address
 */

#ifdef FLASH_OPT
unsigned long Verify (unsigned long adr, unsigned long sz, unsigned char *buf)
{
  (void)adr;
  (void)sz;

  /* not yet coded */
  return (adr + sz);
}
#endif /* FLASH_OPT */
