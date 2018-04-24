/*
********************************************************************************
*                               ´®¿ÚÇý¶¯³ÌÐòÉè¼Æ
*                              ARM Cortex-M3 Port
*
* File          : DrvUsart1.C
* Version       : V1.0
* By            : ÍõºêÇ¿
*
* For           : Stm32f10x
* Mode          : Thumb2
* Toolchain     : 
*                   RealView Microcontroller Development Kit (MDK)
*                   Keil uVision
* Description   : ´®¿Ú1Çý¶¯º¯Êý	

* Date          : 2013.1.27
*******************************************************************************/

#include "stm32f10x_usart.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_nvic.h"


#include <string.h>
#include "BspUsart1.h"
#include "bsp.h"


void (*receChar)(u8 ch) = NULL;


static SerialBuffType m_SendBuff = SerialBuffDefault();
//static SerialBuffType m_ReceBuff = SerialBuffDefault();

static SerialBuffType *sb = &m_SendBuff;
//static SerialBuffType *rb = &m_ReceBuff;

static bool volatile eTXIdle = TRUE;

/*******************************************************************************
* Function Name :static s32 BspUsartxInit(void)
* Description   :´®¿Ú¼°Òý½Å³õÊ¼»¯
* Input         :
* Output        :
* Other         :
* Date          :2013.01.27
*******************************************************************************/
void BspUsart1Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
  	USART_InitTypeDef USART_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;
 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOG, ENABLE);//Ê¹ÅœGPIOA,GÊ±×“
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//Ê¹ÅœUSART2Ê±×“
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;				 //PG9×‹à šÆ¤×ƒ
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //Î†Î¬Ë¤Ô¶
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOG, &GPIO_InitStructure);
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;	//PA2
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//Ø´ÔƒÎ†Î¬
    GPIO_Init(GPIOA, &GPIO_InitStructure);
   
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//PA3
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //Ø¡à •Ë¤É«
    GPIO_Init(GPIOA, &GPIO_InitStructure);  

	RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART2,ENABLE);//Ø´Î»Ô®à š2
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART2,DISABLE);//Í£Ö¹Ø´Î»
 
	

	USART_InitStructure.USART_BaudRate = 115200;//Ò»Ñ£Ê¨×ƒÎª9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//8Î»Ë½ßÓ¤×ˆ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//Ò»Ù¶Í£Ö¹Î»
	USART_InitStructure.USART_Parity = USART_Parity_No;///Ç¦Å¼Ð£Ò©Î»
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//ÏžÓ²Ý¾Ë½ßÂ·à ˜×†
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//Ë•×¢Ä£Ê½

    USART_Init(USART2, &USART_InitStructure); ; //ÔµÊ¼Û¯Ô®à š
  
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQChannel; //Ê¹ÅœÔ®à š2××
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; //ÐˆÕ¼Ô…ÐˆÜ¶2Ü¶
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3; //Õ“Ô…ÐˆÜ¶2Ü¶
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //Ê¹ÅœÎ¢Ò¿××Í¨Ö€
	NVIC_Init(&NVIC_InitStructure); //Ù¹ßNVIC_InitStruct×Ö¸Ö¨Ö„ÓŽË½ÔµÊ¼Û¯Î¢Ê¨NVICÝ„Õ¦Ç·
 
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//ßªÇ´××
   
    USART_Cmd(USART2, ENABLE);                    //Ê¹ÅœÔ®à š 



    GPIO_ResetBits(GPIOG,GPIO_Pin_9);
}

void BspUsart1Close(void)
{
    while (eTXIdle != TRUE);
    USART_Cmd(USART2, DISABLE);

    USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
    USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
    USART_ITConfig(USART2, USART_IT_TC, DISABLE);

    USART_ClearITPendingBit(USART2, USART_IT_TXE);
    USART_ClearITPendingBit(USART2, USART_IT_TC);
    USART_ClearITPendingBit(USART2, USART_IT_RXNE);	
}


/*******************************************************************************
* Function Name :s32 BspUsartSend(u8* buf, u32 len)
* Description   :·¢ËÍÒ»×é×Ö·û´®
* Input         :*buf:×Ö·û´®Ö¸Õë¡£len:³¤¶È
* Output        :
* Other         :
* Date          :2012.05.11  11:45:38
*******************************************************************************/
u16 BspUsart1Send(u8 *buf, u16 len)
{
    GPIO_SetBits(GPIOG,GPIO_Pin_9);
    Delay(1000);
    if( (0 == sb->len) && (len > 0) && (len <= USART1_BUFF_LANGTH) && (eTXIdle == TRUE))
    {

        USART_SendData(USART2, *buf++);
        while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
        eTXIdle = FALSE;
        sb->len = len-1;
        sb->ind = 0;
        memcpy(sb->buf, buf, len - 1);
        USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
        return TRUE;
    }
    GPIO_ResetBits(GPIOG,GPIO_Pin_9);
    Delay(1000);
    return FALSE;
}

/*******************************************************************************
* Function Name :u16 BspUsart1Receive(u8 *buf)
* Description   :¶ÁÈ¡½ÓÊÕµÄ×Ö·û´®
* Input         :buf: »º³å±äÁ¿
* Output        :
* Other         :
* Date          :2013.02.20
*******************************************************************************/
//u16 BspUsart1Receive(u8 *buf)
//{
//    u16 len = rb->ind;
//    
//    memcpy(buf, rb->buf, len );
//    rb->ind = 0;
//
//    return len;
//}

/*******************************************************************************
* Function Name :u8 BspUsart1IRQCallBack(void *fun)
* Description   :ÉèÖÃ´®¿Ú»Øµ÷º¯Êý
* Input         :
* Output        :
* Other         :
* Date          :2013.02.19
*******************************************************************************/
void BspUsart1IRQCallBack(void *fun)
{
    receChar = (void (*)(u8))fun;
}

/*******************************************************************************
* Function Name :void USART1_IRQHandler(void)
* Description   :USART1 ÖÐ¶Ï³ÌÐò
* Input         :
* Output        :
* Other         :
* Date          :2011.11.16  16:57:39
*******************************************************************************/
void USART2_IRQHandler(void)
{
    if(SET == USART_GetITStatus(USART2, USART_IT_TXE))
    {
        USART_ClearITPendingBit(USART2, USART_IT_TXE);
        
        if (sb->len > 0 )
        {
            GPIO_SetBits(GPIOG,GPIO_Pin_9);	
            Delay(1000);
            USART_SendData(USART2, sb->buf[sb->ind++]);
            
            sb->len--;
        }
        else
        {
            GPIO_ResetBits(GPIOG,GPIO_Pin_9);
            Delay(1000);
            USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
            USART_ITConfig(USART2, USART_IT_TC, ENABLE);
        }
    }
    else if (USART_GetITStatus(USART2, USART_IT_TC) != RESET)
    {
        GPIO_ResetBits(GPIOG,GPIO_Pin_9);
        Delay(1000);
        USART_ClearITPendingBit(USART2, USART_IT_TC);
        USART_ITConfig(USART2, USART_IT_TC, DISABLE);
        sb->len = 0;
        eTXIdle = TRUE;
    }
    else if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        u8 ch;
        
        ch = USART_ReceiveData(USART2);

        USART_ClearITPendingBit(USART2, USART_IT_RXNE);	
        
        if (receChar != NULL)
            (*receChar)(ch);
        
        
//        if (rb->ind <USART1_BUFF_LANGTH)
//            rb->buf[rb->ind++] = ch;
        
    }
}



/********************** END ***************************************************/


