/*
 * cmd_parser.c
 *
 *  Created on: Mar 11, 2025
 *      Author: ndondlinger
 */

#include "string.h"
#include "stdio.h"

#include "cmd_parser.h"
#include "cmd_buffer.h"
#include "serial_com.h"
#include "usbd_cdc_if.h"
#include "fake_function_calls.h"


char *UNKOWN_CMD_str = "Unknown Cmd: ";
char *ID_str = "NUCLEO-F767Z1";


void Cmd_Parser_BadSubCmdMsg(char **cmd_arr)
{
    char resp[CMD_BUF_SIZE];
    strcpy(resp, "Unknown sub-command for ");
    strcat(resp, cmd_arr[0]);
    strcat(resp, ": ");
    strcat(resp, cmd_arr[1]);
    USB_Serial_TransmitErrorStr(resp);
}

void Cmd_Parser_SendID(char **cmd_arr, uint16_t len)
{
    USB_Serial_TransmitStr(ID_str);
}

void Cmd_Parser_ParseAsicBatchOem(char **cmd_arr, uint16_t len)
{
    if (len < 5)
    {
        USB_Serial_TransmitErrorStr("Please provide word and channel(s)!");
        return;
    }

    char *pStrEnd;
    char temp_str[5];

    strcpy(temp_str, cmd_arr[3]);
    uint8_t word = (uint8_t)strtol(temp_str, &pStrEnd, 0);
    if (((pStrEnd-temp_str) < strlen(temp_str)) | (word > 8))
    {
        USB_Serial_TransmitErrorStr("Bad word!");
        return;
    }

    uint16_t i;
    uint16_t num_channels = len - 4;
    uint8_t ch;
    uint8_t oem_caps[4096 * num_channels];

    for  (i=0; i<num_channels; i++)
    {
        strcpy(temp_str, cmd_arr[4+i]);
        ch = (uint8_t)strtol(temp_str, &pStrEnd, 0);
        if (((pStrEnd-temp_str) < strlen(temp_str)) | (ch > 41))
        {
            USB_Serial_TransmitErrorStr("Bad channel!");
            return;
        }
        // do OEM capture
        Fake_ASIC_OemCapture(word, ch, &oem_caps[i*4096]);
    }

    CDC_Transmit_HS(oem_caps, sizeof oem_caps);
}

void Cmd_Parser_ParseAsicOem(char **cmd_arr, uint16_t len)
{
    if (len < 3)
    {
        USB_Serial_TransmitErrorStr("Please provide word and channel or type!");
        return;
    }
    if (strcmp(cmd_arr[2], "batch") == 0)
    {
        Cmd_Parser_ParseAsicBatchOem(cmd_arr, len);
        return;
    }
    if (len < 4)
    {
        USB_Serial_TransmitErrorStr("Please provide word and channel!");
        return;
    }

    uint8_t oem_cap[4096];
    char *pStrEnd;
    char temp_str[5];

    strcpy(temp_str, cmd_arr[2]);
    uint8_t word = (uint8_t)strtol(temp_str, &pStrEnd, 0);
    if (((pStrEnd-temp_str) < strlen(temp_str)) | (word > 8))
    {
        USB_Serial_TransmitErrorStr("Bad word!");
        return;
    }

    strcpy(temp_str, cmd_arr[3]);
    uint8_t ch = (uint8_t)strtol(temp_str, &pStrEnd, 0);
    if (((pStrEnd-temp_str) < strlen(temp_str)) | (ch > 41))
    {
        USB_Serial_TransmitErrorStr("Bad channel!");
        return;
    }

    // do OEM capture
    Fake_ASIC_OemCapture(word, ch, oem_cap);

    CDC_Transmit_HS(oem_cap, sizeof oem_cap);
}

void Cmd_Parser_ReadRegCmd(char **cmd_arr, uint16_t len)
{
    uint16_t addr, data;
    char temp_hex_str[2+(BYTES_IN_ASIC*2)+1];
    // format "<addr_hex_str> <data_hex_str>"
    char resp[REG_READ_RESP_LEN];
    char *pStrEnd;

    if (len < 3)
    {
        USB_Serial_TransmitErrorStr("Provide address!");
        return;
    }

    // third token is the addr
    strcpy(temp_hex_str, cmd_arr[2]);
    if (strlen(temp_hex_str) == 0)
    {
        USB_Serial_TransmitErrorStr("Provide address!");
        return;
    }
    addr = (uint16_t)strtol(temp_hex_str, &pStrEnd, 0);
    if ((pStrEnd-temp_hex_str) < strlen(temp_hex_str))
    {
        USB_Serial_TransmitErrorStr("Bad address!");
        return;
    }

    // start building the response string with the reg addr
    sprintf(temp_hex_str, "0x%04X", (unsigned int)addr);
    strcpy(resp, temp_hex_str);
    strcat(resp, " ");

    // do the read
    Fake_ASIC_Read(addr, &data, 0);

    // add the data to the response string
    sprintf(temp_hex_str, "0x%04X", (unsigned int)data);
    strcat(resp, temp_hex_str);

    // send the data packet
    USB_Serial_TransmitStr(resp);
}

void Cmd_Parser_ReadMultiRegCmd(char **cmd_arr, uint16_t len)
{
    uint8_t num;
    uint16_t addr;
    char temp_str[2+(BYTES_IN_ASIC*2)+1+1];
    // format "<addr_hex_str> <data_hex_str>"
    char *pStrEnd;

    if (len < 4)
    {
        USB_Serial_TransmitErrorStr("Provide address and number of registers!");
        return;
    }

    // third token is the addr
    strcpy(temp_str, cmd_arr[2]);
    if (strlen(temp_str) == 0)
    {
        USB_Serial_TransmitErrorStr("Provide address!");
        return;
    }
    addr = (uint16_t)strtol(temp_str, &pStrEnd, 0);
    if ((pStrEnd-temp_str) < strlen(temp_str))
    {
        USB_Serial_TransmitErrorStr("Bad address!");
        return;
    }

    // fourth token is the num
    strcpy(temp_str, cmd_arr[3]);
    if (strlen(temp_str) == 0)
    {
        USB_Serial_TransmitErrorStr("Provide number of registers!");
        return;
    }
    num = (uint8_t)strtol(temp_str, &pStrEnd, 0);
    if ((pStrEnd-temp_str) < strlen(temp_str))
    {
        USB_Serial_TransmitErrorStr("Bad number of registers!");
        return;
    }

    // do the read
    uint16_t data[num+1];
    // Fake_ASIC_SpiMultiRegRead(addr, num, data);
    Fake_ASIC_Read(addr, data, num);

    // start building the response string with the reg addr
    char resp[REG_READ_RESP_LEN + 7*num];
    sprintf(temp_str, "0x%04X", (unsigned int)addr);
    strcpy(resp, temp_str);
    uint8_t str_offset = strlen(resp);

    uint8_t i;
    for (i=0; i<num; i++)
    {
        sprintf(temp_str, " 0x%04X", (unsigned int)data[i]);
        strcat(resp, temp_str);
    }
    sprintf(temp_str, " 0x%04X", (unsigned int)data[i]);
    strcat(resp, temp_str);

    // send the data packet
    USB_Serial_TransmitStr(resp);
}

void Cmd_Parser_WriteRegCmd(char **cmd_arr, uint16_t len)
{
    if (len < 4)
    {
        USB_Serial_TransmitErrorStr("Provide addr and data!");
        return;
    }
    char *addr_str = cmd_arr[2];
    char *data_str = cmd_arr[3];
    char *pStrEnd;
    char temp_hex_str[11];
    uint8_t status = 0;
    // format "<addr_hex_str> <data_hex_str> <status>"
    char resp[REG_WRITE_RESP_LEN];

    // get the address to write to the ASIC
    uint16_t addr = (uint16_t)strtol(addr_str, &pStrEnd, 0);
    if ((pStrEnd-addr_str) < strlen(addr_str))
    {
        USB_Serial_TransmitErrorStr("Bad Addr!");
        return;
    }

    // get the data to write to the ASIC address
    uint16_t data = (uint16_t)strtol(data_str, &pStrEnd, 0);
    if ((pStrEnd-data_str) < strlen(data_str))
    {
        USB_Serial_TransmitErrorStr("Bad Data!");
        return;
    }

    // do the write
    Fake_ASIC_Write(addr, &data, 0);

    // start building the response string with the address
    sprintf(temp_hex_str, "0x%04X", (unsigned int)addr);
    strcpy(resp, temp_hex_str);
    // then data
    strcat(resp, " ");
    sprintf(temp_hex_str, "0x%04X", (unsigned int)data);
    strcat(resp, temp_hex_str);
    // then status
    strcat(resp, " ");
    sprintf(temp_hex_str, "%d", (unsigned int)status);
    strcat(resp, temp_hex_str);

    // send the data packet
    USB_Serial_TransmitStr(resp);
}

void Cmd_Parser_WriteMultiRegCmd(char **cmd_arr, uint16_t len)
{
    uint8_t i;
    uint16_t addr;
    char temp_str[2+(BYTES_IN_ASIC*2)+1+1];
    char *pStrEnd;

    if (len < 4)
    {
        USB_Serial_TransmitErrorStr("Provide address and data!");
        return;
    }
    uint8_t num = len-3;
    uint16_t data[num];

    // third token is the addr
    strcpy(temp_str, cmd_arr[2]);
    if (strlen(temp_str) == 0)
    {
        USB_Serial_TransmitErrorStr("Provide address!");
        return;
    }
    addr = (uint16_t)strtol(temp_str, &pStrEnd, 0);
    if ((pStrEnd-temp_str) < strlen(temp_str))
    {
        USB_Serial_TransmitErrorStr("Bad address!");
        return;
    }

    // fourth token and beyond is data
    for (i=0; i<num; i++)
    {
        strcpy(temp_str, cmd_arr[i+3]);
        data[i] = (uint16_t)strtol(temp_str, &pStrEnd, 0);
        if ((pStrEnd-temp_str) < strlen(temp_str))
        {
            USB_Serial_TransmitErrorStr(strcat("Bad data ", temp_str));
            return;
        }
    }

    // do the write
    // Fake_ASIC_SpiMultiRegWrite(addr, num, data);
    Fake_ASIC_Write(addr, data, num-1);

    // // start building the response string with the reg addr
    // char resp[REG_MULTI_WRITE_RESP_LEN];
    // sprintf(temp_str, "0x%04X", (unsigned int)addr);
    // strcpy(resp, temp_str);

    // // add the data to the response string
    // for (i=0; i<num; i++)
    // {
    //     sprintf(temp_str, " 0x%04X", (unsigned int)data[i]);
    //     strcat(resp, temp_str);
    // }

    // send the data packet
    USB_Serial_TransmitStr("Success");
}

func_map_t asic_cmd_map[] =
{
    {"oem", Cmd_Parser_ParseAsicOem},
    {"mw", Cmd_Parser_WriteMultiRegCmd},
    {"w", Cmd_Parser_WriteRegCmd},
    {"mr", Cmd_Parser_ReadMultiRegCmd},
    {"r", Cmd_Parser_ReadRegCmd},
    {NULL, NULL}
};

void Cmd_Parser_ParseAsicCmd(char **cmd_arr, uint16_t len)
{
    // second token is sub-command
    if (len < 2)
    {
        USB_Serial_TransmitStr("");
        return;
    }
    for (int i = 0; asic_cmd_map[i].name != NULL; i++)
    {
        if (strcmp(cmd_arr[1], asic_cmd_map[i].name) == 0)
        {
            asic_cmd_map[i].func(cmd_arr, len);
            return;
        }
    }
    Cmd_Parser_BadSubCmdMsg(cmd_arr);
}

void Cmd_Parser_ParseAdcCmd(char **cmd_arr, uint16_t len)
{
    if (len < 2)
    {
        USB_Serial_TransmitErrorStr("Provide ADC channel!");
        return;
    }
    uint8_t status;
    char *pStrEnd;
    // get the ADC channel number
    uint8_t ch = (uint8_t)strtol(cmd_arr[1], &pStrEnd, 0);
    if ((pStrEnd-cmd_arr[1]) < strlen(cmd_arr[1]))
    {
        USB_Serial_TransmitErrorStr("Bad ADC channel number!");
        return;
    }
    float voltage;
    status = Fake_ADC_Read(ch, &voltage);
    char resp[ADC_RESP_LEN];
    sprintf(resp, "ADC%d %.3fV", ch, voltage);
    USB_Serial_TransmitStr(resp);
}

void Cmd_Parser_ParseClkCmd(char **cmd_arr, uint16_t len)
{
    if (len < 4)
    {
        USB_Serial_TransmitErrorStr("Provide clock reg address and data!");
        return;
    }
    uint8_t status;
    char *pStrEnd;
    // get the reg addr
    uint8_t addr = (uint8_t)strtol(cmd_arr[2], &pStrEnd, 0);
    if ((pStrEnd-cmd_arr[1]) < strlen(cmd_arr[1]))
    {
        USB_Serial_TransmitErrorStr("Bad address!");
        return;
    }
    uint32_t data = (uint32_t)strtol(cmd_arr[3], &pStrEnd, 0);
    if ((pStrEnd-cmd_arr[2]) < strlen(cmd_arr[2]))
    {
        USB_Serial_TransmitErrorStr("Bad data!");
        return;
    }
    // ADF4355_SPI_Write(addr, data);
    USB_Serial_TransmitStr("Success!");
}

func_map_t cmd_map[] =
{
    {"asic", Cmd_Parser_ParseAsicCmd},
    {"idn", Cmd_Parser_SendID},
    {"adc", Cmd_Parser_ParseAdcCmd},
    {"clk", Cmd_Parser_ParseClkCmd},
    {NULL, NULL}
};

void Cmd_Parser_ParseCmd(char *cmd)
{
    if (strlen(cmd) == 0)
    {
        USB_Serial_TransmitStr("");
        return;
    };

    char *cmd_arr[MAX_NUM_CMD_TOKS];
    char *token = strtok(cmd, CMD_DELIM);
    uint16_t j;
    for (uint16_t i = 0; cmd_map[i].name != NULL; i++)
    {
        if (strcmp(token, cmd_map[i].name) == 0)
        {
            for (j = 0; token != NULL; j++)
            {
                cmd_arr[j] = token;
                token = strtok(NULL, CMD_DELIM);
                if (j == (MAX_NUM_CMD_TOKS-1))
                {
                    j++; // need to increment by one to get the correct len
                    break;
                }
            }
            cmd_map[i].func(cmd_arr, j);
            return;
        }
    }
    char resp[CMD_BUF_SIZE];
    strcpy(resp, UNKOWN_CMD_str);
    strcat(resp, token);
    USB_Serial_TransmitErrorStr(resp);
}
