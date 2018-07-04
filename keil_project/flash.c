#include "stm32f10x_flash.h"
#include "flash.h"
#include "sys.h"

#if STM32_FLASH_SIZE<256
#define STM_SECTOR_SIZE 1024 //字节
#else 
#define STM_SECTOR_SIZE	2048
#endif	

u16 STMFLASH_BUF[STM_SECTOR_SIZE/2];//最多是2K字节

void STMFLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)   
{ 			 		 
	u16 i;
	for(i=0;i<NumToWrite;i++)
	{
		FLASH_ProgramHalfWord(WriteAddr,pBuffer[i]);
		//  Delay_ms(100);
	    WriteAddr+=2;//地址增加2.
	}  
} 

u16 STMFLASH_ReadHalfWord(u32 faddr)
{
	return *(vu16*)faddr; 
}

void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead)   	
{
	u16 i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadHalfWord(ReadAddr);//读取2个字节.
		ReadAddr+=2;//偏移2个字节.	
	}
}

void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)
{
	u32 secpos;	   //????
	u16 secoff;	   //???????(16????)
	u16 secremain; //???????(16????)	   
 	u16 i;    
	u32 offaddr;   //??0X08000000????
	if(WriteAddr<STM32_FLASH_BASE||(WriteAddr>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)))return;//????
	FLASH_Unlock();						//??
	offaddr=WriteAddr-STM32_FLASH_BASE;		//??????.
	secpos=offaddr/STM_SECTOR_SIZE;			//????  0~127 for STM32F103RBT6
	secoff=(offaddr%STM_SECTOR_SIZE)/2;		//???????(2????????.)
	secremain=STM_SECTOR_SIZE/2-secoff;		//????????   
	if(NumToWrite<=secremain)secremain=NumToWrite;//????????
	while(1) 
	{	
		STMFLASH_Read(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//?????????
		for(i=0;i<secremain;i++)//????
		{
			if(STMFLASH_BUF[secoff+i]!=0XFFFF)break;//????  	  
		}
		if(i<secremain)//????
		{
			FLASH_ErasePage(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE);//??????
			for(i=0;i<secremain;i++)//??
			{
				STMFLASH_BUF[i+secoff]=pBuffer[i];	  
			}
			STMFLASH_Write_NoCheck(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,secoff + secremain);//??????  
		}else STMFLASH_Write_NoCheck(WriteAddr,pBuffer,secremain);//???????,??????????. 				   
		if(NumToWrite==secremain)break;//?????
		else//?????
		{
			secpos++;				//?????1
			secoff=0;				//?????0 	 
		   	pBuffer+=secremain;  	//????
			WriteAddr+=secremain*2;	//?????	   
		   	NumToWrite-=secremain;	//??(16?)???
			if(NumToWrite>(STM_SECTOR_SIZE/2))secremain=STM_SECTOR_SIZE/2;//??????????
			else secremain=NumToWrite;//??????????
		}	 
	};	
	FLASH_Lock();//??
}
