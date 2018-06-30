#include "update.h"
#include "ymodem.h"
#include "sys.h"
#include "stdio.h"
#include "flash.h"

#define USART1_BUFF_LANGTH 1048
#define FLASH_APP_ADDR (0x08010000)

typedef struct
{
    u32 len;
    u16 ind;
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

static volatile SerialBuffType m_ReceData = SerialBuffDefault();

int len = 0;
u8 pArray[1028] = {
    0,
};
u32 writeaddr = FLASH_APP_ADDR;

bool pack_rec_is_complete(void);

void file_get_by_ymodem(void)
{
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
            STMFLASH_Write(writeaddr, (u16 *)pArray, len / 2);
						//Delay_ms(1000);
            writeaddr += len;
            CLR_TIME_OUT(eTimYModem);
            break;
        case YM_EXIT:
            writeaddr = FLASH_APP_ADDR;
            printf("\nDone!\n");
            return;
            //break;
        default:
            break;
        }
        pack_rec_is_complete();
    }
}

bool pack_rec_is_complete(void)
{
    if (m_ReceData.ind != 0 && m_ReceData.len != m_ReceData.ind) //刚开始都为0，ind不为0，说明有接收到数据
    {
        s16 len = 0;
        if (m_ReceData.ind > PACKET_HEADER)
        {
            if (m_ReceData.buf[0] == SOH)
            {
                len = PACKET_SIZE;
            }
            else if (m_ReceData.buf[0] == STX)
            {
                len = PACKET_1K_SIZE;
            }
        }
        if (len + PACKET_OVERHEAD == m_ReceData.ind)
        {
            m_ReceData.len = m_ReceData.ind;
            m_ReceData.ind = 0;
        }
        else
        //if (get_sys_ms() - TimerCnt[FrameTimeOver] > 2000)
        //(IS_TIME_OUT(FrameTimeOver, 1000)) //一包数据接收完成
        {
            u32 end = TimerCnt[FrameTimeOver];
            u32 start = get_sys_ms();
            u32 interval = start - end; //get_sys_ms() - TimerCnt[FrameTimeOver];
            if (interval > 100)
            {
                m_ReceData.len = m_ReceData.ind;
                m_ReceData.ind = 0;
                return true;
            }
        }
    }
    return false;
}

void USART1_UserHandler(u8 data)
{
    if (m_ReceData.ind >= USART1_BUFF_LANGTH)
        return;

    if (m_ReceData.len > 0)
        return;

    m_ReceData.buf[m_ReceData.ind++] = data;
    //CLR_TIME_OUT(FrameTimeOver);
    TimerCnt[FrameTimeOver] = get_sys_ms();
}
