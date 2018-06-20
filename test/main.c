#include "stm32f10x_gpio.h"
#include "stdio.h"
#include "uart.h"
#include "sys.h"

void Uart_RevHandler(u8 byte);



static u8 uart_revbuf[1028]={0};
u16 rev_cnt=0;
char filename[16]={0};
char filesize[16]={0};
u8 *fileptr;

int main()
{
	int i=0;
	GPIO_InitTypeDef gpio;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Pin = GPIO_Pin_13;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOC, &gpio);
	
	SysTick_Init();
	SysTick_Cmd(ENABLE);
	
	Uart_Init(Uart_RevHandler);
	__enable_irq();
	while(1)
	{
		Delay_ms(5000);
		printf("C");
		GPIO_ResetBits(GPIOC,GPIO_Pin_13);
		Delay_ms(500);
		GPIO_SetBits(GPIOC,GPIO_Pin_13);
		Delay_ms(500);
		if(rev_cnt)
		{
			printf("rev %d bytes\n",rev_cnt);
			fileptr = uart_revbuf + 3;
			for(i=0; (i<16)&&(*fileptr!=0);i++)
			{
				filename[i] = *fileptr;
				fileptr++;
			}
			fileptr[i]='\0';
			printf("file name :%s\n",filename);
			for(i=0,fileptr++;(*fileptr!=' ') && i<16;)
			{
				filesize[i++] = *fileptr++;
			}
			filesize[i++]='\0';
			printf("file size %s\n",filesize);
		}
	}
}



void Uart_RevHandler(u8 byte)
{
	//printf("%c",byte);//printf("%c\n",byte);加上换行符后，接收多字节，程序容易挂掉
	if(rev_cnt<1028)
	{
		uart_revbuf[rev_cnt] = byte;
		rev_cnt++;
	}
}

