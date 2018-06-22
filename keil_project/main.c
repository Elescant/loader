#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stdio.h"
#include "stm32f10x_flash.h"
#include "stdbool.h"
#include "flash.h"
#include "uart.h"
#include "sys.h"
#include "ymodem.h"

#define FLASH_APP_ADDR (0x08010000)


void test_flash_bank(void);

u32 FLASH_WriteBank(u8 *pData, u32 addr, u16 size);
void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 appsize);
void iap_load_app(u32 appxaddr);

typedef void (*iapfun)(void);
iapfun jump2app;
void USART1_UserHandler(u8 data);
void USART2_UserHandler(u8 data);

//__asm void MSR_MSP(u32 addr)
//{
//	MSR MSP,r0
//	BX r14
//}

//__set_MSP(*(__IO uint32_t *)1234);

u8 RxBuf[4096];
u16 bufcnt=0;
bool update=false;
u16 applen=0;


#define YM_FILE_INFO            (2)
#define YM_FILE_DATA            (1)
#define YM_VOIDER               (0)
#define YM_EXIT                 (-1)

#define USART1_BUFF_LANGTH     1048

typedef struct {
	u32 len;
	u16 ind;	
	u8  buf[USART1_BUFF_LANGTH];
}SerialBuffType;		//·￠?í?Y′???
#define SerialBuffDefault() {\
	{0,},\
	0,\
	0,\
}

static volatile SerialBuffType m_ReceData = SerialBuffDefault();


int main(void)
{

	GPIO_InitTypeDef gpio;
	USART_InitTypeDef usart;

	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Pin = GPIO_Pin_13;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOC, &gpio);
	

	GPIO_ResetBits(GPIOC,GPIO_Pin_13);
	

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);

	
	gpio.GPIO_Pin = GPIO_Pin_7;
	gpio.GPIO_Mode = GPIO_Mode_IPU;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC,&gpio);
	

	SysTick_Init();
	SysTick_Cmd(ENABLE);
	
//	test_flash_bank();
	Uart1_Init(USART1_UserHandler);
	Uart2_Init(USART2_UserHandler);
	u16 oldcnt=0;

	
	int len=0;
	int len2=0;
	u8 pArray[1028] = {0,};
	u8 u_buf[1048]={0,};
	
	while(1)
	{
		switch(YmodemReceive((char *)m_ReceData.buf, &m_ReceData.len, (char *)pArray, &len))
		{
			case YM_FILE_INFO:
				printf("file info:\n");
				for(int i=0;i<len;i++)
				{
					printf("%c",pArray[i]);
				}
				
				break;
			case YM_FILE_DATA:
					printf("data");
				break;
			case YM_EXIT:
				printf("Exit\n");
				break;
		}
		if(m_ReceData.ind !=0 && m_ReceData.len != m_ReceData.ind)//刚开始都为0，ind不为0，说明有接收到数据
		{
			if(IS_TIME_OUT(FrameTimeOver,100))
			{
				m_ReceData.len = m_ReceData.ind;
				m_ReceData.ind = 0;
			}
		}
	}

	if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_7) != RESET)
	{
		__disable_irq();
		 vu32 temp = *(vu32*)(FLASH_APP_ADDR+4);
		if((temp&0xFF000000)==0x08000000)//判断是否为0X08XXXXXX.
		{	 
			iap_load_app(FLASH_APP_ADDR);//执行FLASH APP代码
		}
	}else
	{
			while(1)
			{
				Delay_ms(10);
				if(bufcnt)
				{
					if(bufcnt == oldcnt)
					{
						printf("接收完成,%d 字节\n",bufcnt);
						applen = bufcnt;
						update = true;
						bufcnt = 0;
						oldcnt = 0;
					}else
					{
						oldcnt = bufcnt;
					}
				}
				if(update)
				{
					update = false;
					printf("开始更新\n");
					__disable_irq();
					iap_write_appbin(FLASH_APP_ADDR,RxBuf,applen);
					iap_load_app(FLASH_APP_ADDR);
				}
			}
	}
}

void iap_load_app(u32 appxaddr)
{
	if(((*(vu32*)appxaddr)&0x2FFE0000) == 0x20000000)
	{
		jump2app = (iapfun)*(vu32 *)(appxaddr + 4);
		//MSR_MSP(*(vu32*)appxaddr);
		__set_MSP(*(__IO uint32_t*) appxaddr);
		jump2app();
	}
}

u16 iapbuf[1024];
void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 appsize)
{
	u16 t;
	u16 i=0;
	u16 temp;
	u32 fwaddr=appxaddr;//当前写入的地址
	u8 *dfu=appbuf;
	for(t=0;t<appsize;t+=2)
	{						    
		temp=(u16)dfu[1]<<8;
		temp+=(u16)dfu[0];	  
		dfu+=2;//偏移2个字节
		iapbuf[i++]=temp;	    
		if(i==1024)
		{
			i=0;
			STMFLASH_Write(fwaddr,iapbuf,1024);	
			fwaddr+=2048;//偏移2048  16=2*8.所以要乘以2.
		}
	}
	if(i)STMFLASH_Write(fwaddr,iapbuf,i);//将最后的一些内容字节写进去.  
}





void test_flash_bank(void)
{
	u16 ret;
	u8 data[11]={0x12,0x34,0xFF,0xFF,0xFF,4,3,2,1,0,0xAA};
	u8 data2[11]={0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xAA,0xBB};
		
	FLASH_Unlock();
	FLASH_ErasePage(FLASH_APP_ADDR-2);
	FLASH_ErasePage(FLASH_APP_ADDR);
	FLASH_ProgramHalfWord(FLASH_APP_ADDR - 2,0xAABB);
	FLASH_ProgramHalfWord(FLASH_APP_ADDR,0xCCDD);
//	FLASH_ErasePage(FLASH_APP_ADDR-1024);
//	ret = FLASH_WriteBank(data,FLASH_APP_ADDR-11,5);
//	ret = FLASH_WriteBank(data,FLASH_APP_ADDR,11);
	FLASH_Lock();
	if(ret)
	{
			printf("write success\n");
	}else
	{
		  printf("write failed\n");
	}

}


u32 FLASH_WriteBank(u8 *pData, u32 addr, u16 size)
{
    vu16 *pDataTemp = (vu16 *)pData;
    vu32 temp = addr;

    for (; temp < (addr + size); pDataTemp++, temp += 2)
    {
        FLASH_ProgramHalfWord(temp, *pDataTemp);
        if (*pDataTemp != *(vu16 *)temp)
        {
            return false;
        }
    }

    return true;
}

void USART1_UserHandler(u8 data)
{
    if (m_ReceData.ind >= USART1_BUFF_LANGTH)
        return;
        
    if (m_ReceData.len > 0)
        return;
        
    m_ReceData.buf[m_ReceData.ind++] = data;
		CLR_TIME_OUT(FrameTimeOver);
}
void USART2_UserHandler(u8 data)
{
	printf("%c",data);
}
	
	
	
	