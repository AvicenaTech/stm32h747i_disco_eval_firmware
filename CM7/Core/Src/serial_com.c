/*
 * serial_com.c
 *
 *  Created on: Mar 11, 2025
 *      Author: ndondlinger
 */


#include "usbd_cdc_if.h"
#include "serial_com.h"
#include "cmd_buffer.h"
#include "cmd_parser.h"

char NEW_LINE[] = "\r\n";

CmdBuffer_t *cmd_buf;

void USB_Serial_Init(void)
{
    Cmd_Buf_InitBuf(cmd_buf);
}

void USB_Serial_TransmitStr(char *msg)
{
	char Buf[strlen(msg)+5];
	strcpy(Buf, NEW_LINE);
	if (strlen(msg) > 0)
	{
		strcat(Buf, msg);
		strcat(Buf, NEW_LINE);
	}
	CDC_Transmit_HS((uint8_t*)Buf, strlen(Buf));
}

void USB_Serial_TransmitErrorStr(char *msg)
{
	char err_msg[strlen(msg)+2];
	strcpy(err_msg, "?");
	strcat(err_msg, msg);
	USB_Serial_TransmitStr(err_msg);
}

void USB_Serial_RxHandler(uint8_t* Buf, uint32_t Len)
{
	if (Buf[Len-1] == '\r')
	{
		Cmd_Buf_QueueData(cmd_buf, Buf, Len-1);
		char cmd[(cmd_buf->index)+1];
        Cmd_Buf_DequeueCmd(cmd_buf, cmd);
		Cmd_Parser_ParseCmd(cmd);
	} else if (Len == 1)
	{
        Cmd_Buf_QueueData(cmd_buf, Buf, Len);
		CDC_Transmit_HS(Buf, Len);
	} else
	{
		Cmd_Buf_QueueData(cmd_buf, Buf, Len);
	}
}
