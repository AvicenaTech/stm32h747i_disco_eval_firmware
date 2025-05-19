/*
 * cmd_buffer.c
 *
 *  Created on: Mar 11, 2025
 *      Author: ndondlinger
 */
#include <string.h>

#include "cmd_buffer.h"

void Cmd_Buf_InitBuf(CmdBuffer_t *cb)
{
    memset(cb, 0, sizeof *cb);
    cb->maxlen = CMD_BUF_SIZE;
}

uint8_t Cmd_Buf_QueueData(CmdBuffer_t *cb, uint8_t *data_in, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++)
    {
        if (cb->index == (cb->maxlen - 1)){
            return 0;
        } else
        {
            cb->data[cb->index] = data_in[i];
            cb->index++;
        }
    }
    return 1;
}

void Cmd_Buf_DequeueCmd(CmdBuffer_t *cb, char *cmd)
{
    uint32_t i;
    for (i = 0; i <= cb->index; i++)
    {
        cmd[i] = (char)cb->data[i];
    }
    cmd[i] = '\0';
    Cmd_Buf_InitBuf(cb);
}
