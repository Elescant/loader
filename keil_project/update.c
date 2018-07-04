#include "update.h"
#include "ymodem.h"
#include "sys.h"
#include "stdio.h"
#include "flash.h"
#include "uart.h"

#define FLASH_APP_ADDR (0x08010000)

typedef struct
{
    u32 len;
    u32 ind;
    u8 buf[USART1_BUFF_LANGTH];
} SerialBuffType; //·￠?í?Y′???
#define SerialBuffDefault() \
    {                       \
        {                   \
            0,              \
        },                  \
            0,              \
            0,              \
    }

volatile SerialBuffType m_ReceData = SerialBuffDefault();

int len = 0;
u8 pArray[1028] = {
    0,
};
u32 writeaddr = FLASH_APP_ADDR;

bool pack_rec_is_complete(void);

void file_get_by_ymodem(void)
{
    Uart1_Init(NULL);
	DMA_Configuration(m_ReceData.buf);

    while (1)
    {
        switch (YmodemReceive((char *)m_ReceData.buf, &m_ReceData.len, (char *)pArray, &len))
        {
        case YM_FILE_INFO:
            //printf("\nFile Name: %s\n",pArray);
            CLR_TIME_OUT(eTimYModem);
            break;
        case YM_FILE_DATA:
            printf("#");
			//Delay_ms(100);//让串口接收数据，错开flash操作时间
            //STMFLASH_Write(writeaddr, (u16 *)pArray, len / 2);
            writeaddr += len;
            CLR_TIME_OUT(eTimYModem);
            break;
        case YM_EXIT:
            writeaddr = FLASH_APP_ADDR;
            m_ReceData.ind = 0;
            m_ReceData.len = 0;
            printf("\nDone!\n");
            return;
            //break;
        case YM_PACK_ERR:
            printf("Err Happened\n");
            break;
        default:
            break;
        }
        pack_rec_is_complete();
    }
}


bool pack_rec_is_complete(void)
{
    uint16_t datalen=0;
    uint16_t packsize = PACKET_SIZE;
    static uint16_t lastlen=0;

    datalen = Uart1_Get_RecLen();
    if(lastlen != datalen)
    {
        CLR_TIME_OUT(FrameTimeOver);
        lastlen = datalen;
    }

    if(datalen)
    {
        if(datalen>PACKET_HEADER)
        {
            if (m_ReceData.buf[0] == SOH)
            {
                packsize = PACKET_SIZE;
            }
            else if (m_ReceData.buf[0] == STX)
            {
                packsize = PACKET_1K_SIZE;
            }
            if (packsize + PACKET_OVERHEAD == datalen)
            {
                m_ReceData.len = datalen;
                lastlen = 0;
                return true;
            }
        }else if(m_ReceData.buf[0] == EOT)
        {
            m_ReceData.len = 1;
            lastlen = 0;
            return true;
        }else
        {
            u32 end = TimerCnt[FrameTimeOver];
            if (get_sys_ms() - end > FRAME_TIMEOUT)
            {
                m_ReceData.len = Uart1_Get_RecLen();
                lastlen = 0;
                return true;
            }           
        }
    }
    return false;
}

void pack_get_again(void)
{
    m_ReceData.len = 0;
    //memset(m_ReceData.buf,0,USART1_BUFF_LANGTH);
    DMA_Again(USART1_BUFF_LANGTH);
}

// bool pack_rec_is_complete(void)
// {
//     if (m_ReceData.ind != 0 && m_ReceData.len != m_ReceData.ind) //刚开始都为0，ind不为0，说明有接收到数据
//     {
//         s16 len = 0;
//         if (m_ReceData.ind > PACKET_HEADER)
//         {
//             if (m_ReceData.buf[0] == SOH)
//             {
//                 len = PACKET_SIZE;
//             }
//             else if (m_ReceData.buf[0] == STX)
//             {
//                 len = PACKET_1K_SIZE;
//             }
//         }
//         if (len + PACKET_OVERHEAD == m_ReceData.ind)
//         {
//             m_ReceData.len = m_ReceData.ind;
//             m_ReceData.ind = 0;
//         }
//         else
//         //if (get_sys_ms() - TimerCnt[FrameTimeOver] > 2000)
//         //(IS_TIME_OUT(FrameTimeOver, 1000)) //一包数据接收完成
//         {
//             u32 end = TimerCnt[FrameTimeOver];
//             u32 start = get_sys_ms();
//             u32 interval = start - end; //get_sys_ms() - TimerCnt[FrameTimeOver];
//             if (interval > FRAME_TIMEOUT)
//             {
//                 m_ReceData.len = m_ReceData.ind;
// 				printf("ind %d,interval %d\n",m_ReceData.ind,interval);
//                 m_ReceData.ind = 0;
//                 return true;
//             }
//         }
//     }
//     return false;
// }

// void USART1_UserHandler(u8 data)
// {
//     if (m_ReceData.ind >= USART1_BUFF_LANGTH)
//         return;

//     if (m_ReceData.len > 0)
//         return;
//     if((get_sys_ms() - TimerCnt[FrameTimeOver] < FRAME_TIMEOUT) || m_ReceData.ind ==0)
//     {
//         m_ReceData.buf[m_ReceData.ind++] = data;
//         //CLR_TIME_OUT(FrameTimeOver);
//         TimerCnt[FrameTimeOver] = get_sys_ms();
//     }
//     else
//     {
//         return;
//     }
// }
