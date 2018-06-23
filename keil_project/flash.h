#ifndef FLASH_H_H
#define FLASH_H_H

#include "stm32f10x.h"

#define STM32_FLASH_SIZE 256 	 		
#define STM32_FLASH_WREN 1            

#define STM32_FLASH_BASE 0x08000000 	
 
 void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);
#endif
