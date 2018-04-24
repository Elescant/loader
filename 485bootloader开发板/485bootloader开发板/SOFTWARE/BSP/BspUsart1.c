/*
********************************************************************************
*                               ���������������
*                              ARM Cortex-M3 Port
*
* File          : DrvUsart1.C
* Version       : V1.0
* By            : ����ǿ
*
* For           : Stm32f10x
* Mode          : Thumb2
* Toolchain     : 
*                   RealView Microcontroller Development Kit (MDK)
*                   Keil uVision
* Description   : ����1��������	

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
* Description   :���ڼ����ų�ʼ��
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
 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOG, ENABLE);//ʹŜGPIOA,Gʱד
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//ʹŜUSART2ʱד
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;				 //PG9׋ࠚƤ׃
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //ΆάˤԶ
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOG, &GPIO_InitStructure);
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;	//PA2
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//شԃΆά
    GPIO_Init(GPIOA, &GPIO_InitStructure);
   
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//PA3
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //ءࠕˤɫ
    GPIO_Init(GPIOA, &GPIO_InitStructure);  

	RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART2,ENABLE);//شλԮࠚ2
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART2,DISABLE);//ֹͣشλ
 
	

	USART_InitStructure.USART_BaudRate = 115200;//һѣʨ׃Ϊ9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//8λ˽ߝӤ׈
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һٶֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;///ǦżУҩλ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//ϞӲݾ˽ߝ·࠘׆
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//˕עģʽ

    USART_Init(USART2, &USART_InitStructure); ; //ԵʼۯԮࠚ
  
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQChannel; //ʹŜԮࠚ2א׏
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; //ЈռԅЈܶ2ܶ
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3; //ՓԅЈܶ2ܶ
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //ʹŜ΢ҿא׏ͨր
	NVIC_Init(&NVIC_InitStructure); //ٹߝNVIC_InitStructאָ֨քӎ˽Եʼۯ΢ʨNVIC݄զǷ
 
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//ߪǴא׏
   
    USART_Cmd(USART2, ENABLE);                    //ʹŜԮࠚ 



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
* Description   :����һ���ַ���
* Input         :*buf:�ַ���ָ�롣len:����
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
* Description   :��ȡ���յ��ַ���
* Input         :buf: �������
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
* Description   :���ô��ڻص�����
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
* Description   :USART1 �жϳ���
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


