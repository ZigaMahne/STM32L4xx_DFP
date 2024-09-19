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
 * Project:      Flash Device Description for ST STM32L4Rx/Sx Flash
 * --------------------------------------------------------------------------- */

/* History:
 *  Version 1.1.0
 *    Reworked algorithms
 *    Added Single Bank configuration
 *  Version 1.0.0
 *    Initial release
 */

#include "..\FlashOS.h"                /* FlashOS Structures */


#ifdef FLASH_MEM

#ifdef STM32L4Rx_2048_DB
  struct FlashDevice const FlashDevice  =  {
    FLASH_DRV_VERS,                    /* Driver Version, do not modify! */
    "STM32L4Rx 2MB Dual Bank Flash",   /* Device Name */
    ONCHIP,                            /* Device Type */
    0x08000000,                        /* Device Start Address */
    0x00200000,                        /* Device Size in Bytes (2048 kB) */
    1024,                              /* Programming Page Size */
    0,                                 /* Reserved, must be 0 */
    0xFF,                              /* Initial Content of Erased Memory */
    400,                               /* Program Page Timeout 400 mSec */
    400,                               /* Erase Sector Timeout 400 mSec */
    /* Specify Size and Address of Sectors */
    0x1000, 0x000000,                  /* Sector Size  4kB (512 Sectors) */
    SECTOR_END
  };
#endif

#ifdef STM32L4Rx_2048_SB
  struct FlashDevice const FlashDevice  =  {
    FLASH_DRV_VERS,                    /* Driver Version, do not modify! */
    "STM32L4Rx 2MB Single Bank Flash", /* Device Name */
    ONCHIP,                            /* Device Type */
    0x08000000,                        /* Device Start Address */
    0x00200000,                        /* Device Size in Bytes (2048 kB) */
    1024,                              /* Programming Page Size */
    0,                                 /* Reserved, must be 0 */
    0xFF,                              /* Initial Content of Erased Memory */
    400,                               /* Program Page Timeout 400 mSec */
    400,                               /* Erase Sector Timeout 400 mSec */
    /* Specify Size and Address of Sectors */
    0x2000, 0x000000,                  /* Sector Size  8kB (256 Sectors) */
    SECTOR_END
  };
#endif

#ifdef STM32L4Rx_1024_DB
  struct FlashDevice const FlashDevice  =  {
    FLASH_DRV_VERS,                    /* Driver Version, do not modify! */
    "STM32L4Rx 1MB Dual Bank Flash",   /* Device Name */
    ONCHIP,                            /* Device Type */
    0x08000000,                        /* Device Start Address */
    0x00100000,                        /* Device Size in Bytes (1024 kB) */
    1024,                              /* Programming Page Size */
    0,                                 /* Reserved, must be 0 */
    0xFF,                              /* Initial Content of Erased Memory */
    400,                               /* Program Page Timeout 400 mSec */
    400,                               /* Erase Sector Timeout 400 mSec */
    /* Specify Size and Address of Sectors */
    0x1000, 0x000000,                  /* Sector Size  4kB (256 Sectors) */
    SECTOR_END
  };
#endif

#ifdef STM32L4Rx_1024_SB
  struct FlashDevice const FlashDevice  =  {
    FLASH_DRV_VERS,                    /* Driver Version, do not modify! */
    "STM32L4Rx 1MB Single Bank Flash", /* Device Name */
    ONCHIP,                            /* Device Type */
    0x08000000,                        /* Device Start Address */
    0x00100000,                        /* Device Size in Bytes (1024 kB) */
    1024,                              /* Programming Page Size */
    0,                                 /* Reserved, must be 0 */
    0xFF,                              /* Initial Content of Erased Memory */
    400,                               /* Program Page Timeout 400 mSec */
    400,                               /* Erase Sector Timeout 400 mSec */
    /* Specify Size and Address of Sectors */
    0x2000, 0x000000,                  /* Sector Size  8kB (128 Sectors) */
    SECTOR_END
  };
#endif

#endif /* FLASH_MEM */
