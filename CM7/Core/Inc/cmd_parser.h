/*
 * cmd_parser.h
 *
 *  Created on: Mar 11, 2025
 *      Author: ndondlinger
 */

#ifndef INC_CMD_PARSER_H_
#define INC_CMD_PARSER_H_

#include "stm32h7xx.h"
#include "cmd_buffer.h"

#define BYTES_IN_ASIC 2
#define REG_READ_RESP_LEN ((2+6) + 1 + (2+6) + 2)
#define REG_WRITE_RESP_LEN (REG_READ_RESP_LEN + 2)
#define ADC_RESP_LEN (3 + 3 + 1 + 6 + 1 + 1)
#define REG_MULTI_WRITE_RESP_LEN CMD_BUF_SIZE

#define CMD_DELIM " "
// based on multi-write (cmd + sub-cmd + addr + 256 data)
#define MAX_NUM_CMD_TOKS (1 + 1 + 1 + 256)

typedef struct
{
    char *name;
    void (*func)(char**, uint16_t);
} func_map_t;

void Cmd_Parser_ParseCmd(char*);

#endif /* INC_CMD_PARSER_H_ */
