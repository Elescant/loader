/*
********************************************************************************
*                               串口驱动程序设计
*                              ARM Cortex-M3 Port
*
* File          : DrvUsart1.C
* Version       : V1.0
* By            : 王宏强
*
* For           : Stm32f10x
* Mode          : Thumb2
* Toolchain     : 
*                   RealView Microcontroller Development Kit (MDK)
*                   Keil uVision
* Description   : 串口1驱动函数	

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
* Description   :串口及引脚初始化
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
 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOG, ENABLE);//使艤GPIOA,G时讚
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//使艤USART2时讚
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;				 //PG9讒酄毱ぷ�
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //螁维摔远
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOG, &GPIO_InitStructure);
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;	//PA2
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//卮詢螁维
    GPIO_Init(GPIOA, &GPIO_InitStructure);
   
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//PA3
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //亍酄曀ど�
    GPIO_Init(GPIOA, &GPIO_InitStructure);  

	RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART2,ENABLE);//卮位援酄�2
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART2,DISABLE);//停止卮位
 
	

	USART_InitStructure.USART_BaudRate = 115200;//一眩狮變为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//8位私邼婴讏
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一俣停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;///铅偶校药位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//蠟硬菥私邼路酄樧�
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//藭注模式

    USART_Init(USART2, &USART_InitStructure); ; //缘始郫援酄�
  
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQChannel; //使艤援酄�2讗讖
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; //袌占詤袌芏2芏
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3; //論詤袌芏2芏
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //使艤微铱讗讖通謤
	NVIC_Init(&NVIC_InitStructure); //俟邼NVIC_InitStruct讗指吱謩訋私缘始郫微狮NVIC輨咋欠
 
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//擢谴讗讖
   
    USART_Cmd(USART2, ENABLE);                    //使艤援酄� 



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
* Description   :发送一组字符串
* Input         :*buf:字符串指针。len:长度
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
* Description   :读取接收的字符串
* Input         :buf: 缓冲变量
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
* Description   :设置串口回调函数
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
* Description   :USART1 中断程序
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


