/*
 * ring_buffer.h
 *
 *  Created on: Mar 11, 2025
 *      Author: ndondlinger
 */

#ifndef INC_CMD_BUFFER_H_
#define INC_CMD_BUFFER_H_

#include "stm32h7xx.h"

// max cmd size based on 256 reg multi write
// "reg mr <addr> <data>*256\r"
# define CMD_BUF_SIZE (3 + 1 + 2 + 1 + 6 + (1 + 6)*256 + 1 + 1)

typedef struct CmdBuffer
{
    uint8_t data[CMD_BUF_SIZE];
    uint16_t index;
    uint16_t maxlen;
} CmdBuffer_t;

void Cmd_Buf_InitBuf(CmdBuffer_t*);
uint8_t Cmd_Buf_QueueData(CmdBuffer_t*, uint8_t*, uint32_t);
void Cmd_Buf_DequeueCmd(CmdBuffer_t*, char*);

#endif /* INC_CMD_BUFFER_H_ */
