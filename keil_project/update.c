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
    \
}

static volatile SerialBuffType m_ReceData = SerialBuffDefault();

int len = 0;
u8 pArray[1028] = {
    0,
};
u32 writeaddr = FLASH_APP_ADDR;

void file_get_by_ymodem(void)
{
    while (1)
    {
        switch (YmodemReceive((char *)m_ReceData.buf, &m_ReceData.len, (char *)pArray, &len))
        {
        case YM_FILE_INFO:
            //printf("\nFile Name: %s\n",pArray);
        break;
        case YM_FILE_DATA:
            printf("#");
            STMFLASH_Write(writeaddr, (u16 *)pArray, len / 2);
            writeaddr += len;
            break;
        case YM_EXIT:
            writeaddr = FLASH_APP_ADDR;
            printf("\nDone!\n");
            return;
            //break;
				default:
					break;
        }
        if (m_ReceData.ind != 0 && m_ReceData.len != m_ReceData.ind) //刚开始都为0，ind不为0，说明有接收到数据
        {
            if (IS_TIME_OUT(FrameTimeOver, 100)) //一包数据接收完成
            {
                m_ReceData.len = m_ReceData.ind;
                m_ReceData.ind = 0;
            }
        }
    }
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
