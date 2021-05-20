/*
 * cmd_handler.h
 *
 *  Created on: Apr 9, 2021
 *      Author: karol
 */

#include <stdio.h>
#include <string.h>

#include "cmd_handler.h"
#include "uart.h"
#include "main.h"
#include "usart.h"
#include "tim.h"

//--------------------------------------------------------------------------------

#define MAX_SIZE_CMD_BUFFER 100
#define MAX_SIZE_TX_BUFFER 400
#define MY_UART  huart2

//--------------------------------------------------------------------------------

enum commends
{
    GET_TIME,
    SET_TIME,
    GET_VALVE_STATE,
    SET_VALVE_STATE,
    GET_TEMP,
    SET_TEMP,
    HELP,
    TOGGLE_MONITOR,

    EMPTY_BUFFER,

    commends_enum_end,
};

//--------------------------------------------------------------------------------

struct cmd_struct
{
    char *cmd;
    uint8_t size;
};

struct beer_machine
{
    bool monitor_first_write;
    uint32_t tmp_var;

    char time[9];
    bool valve_state;
    char temp[4];
    bool monitor;
};

//--------------------------------------------------------------------------------

const static struct cmd_struct cmd_tab[commends_enum_end] =
{
    [GET_TIME]          = {.cmd = "get time", .size = 8},
    [SET_TIME]          = {.cmd = "set time", .size = 8},
    [GET_VALVE_STATE]   = {.cmd = "get valve state", .size = 15},
    [SET_VALVE_STATE]   = {.cmd = "set valve state", .size = 15},
    [GET_TEMP]          = {.cmd = "get temp", .size = 8},
    [SET_TEMP]          = {.cmd = "set temp", .size = 8},
    [TOGGLE_MONITOR]    = {.cmd = "toggle monitor", .size = 14},
    [HELP]              = {.cmd = "help", .size = 4},

    [EMPTY_BUFFER]      = {.cmd = "\r", .size = 1},
};

struct beer_machine beer_machine_ctx = {.time = "00:00:00", .valve_state = true, .temp = "24C"};

//--------------------------------------------------------------------------------

static char tmp_cmd_buffer[MAX_SIZE_CMD_BUFFER];
static char prev_tmp_cmd_buffer[MAX_SIZE_CMD_BUFFER];
static char transmit_buffer[MAX_SIZE_TX_BUFFER];
static const char feedback_cmd[] = "<beer machine>";

//--------------------------------------------------------------------------------

static void cmd_handler(enum commends cmd);
static bool backspace_handler(void);
static enum commends cmd_search(void);
static void wrong_argument(void);
static bool check_if_number(char c);
static void monitor_print(void);

static void cmd_get_time_handler(void);
static void cmd_set_time_handler(void);
static void cmd_get_valve_state_handler(void);
static void cmd_set_valve_state_handler(void);
static void cmd_get_temp_handler(void);
static void cmd_set_temp_handler(void);
static void cmd_toggle_monitor_handler(void);
static void cmd_help_handler(void);
static void cmd_unknown_handler(void);

//--------------------------------------------------------------------------------

void cmd_handler_init(void)
{
    USART_WriteString(feedback_cmd);
}

//--------------------------------------------------------------------------------

void cmd_handler_process(void)
{
    if(uart_check_new_line())
    {
        USART_ReadData(tmp_cmd_buffer, MAX_SIZE_CMD_BUFFER);
        memcpy(prev_tmp_cmd_buffer, tmp_cmd_buffer, MAX_SIZE_CMD_BUFFER);
        while(backspace_handler());
        cmd_handler(cmd_search());
        USART_WriteString(feedback_cmd);
    }

    if(uart_check_esc())
    {
//        uint32_t i = 0;
//        USART_ReadData(tmp_cmd_buffer, MAX_SIZE_CMD_BUFFER);
//        while(tmp_cmd_buffer[i++] != '\e')
//        {
//            USART_WriteString("\b");
//        }
//
//        for(uint8_t j = 0; j < i; j++)
//        {
//            ring_buffer_put_val(&USART_RingBuffer_Rx, prev_tmp_cmd_buffer[j]);
//        }
//
//        USART_WriteString(prev_tmp_cmd_buffer);
    }
}

//--------------------------------------------------------------------------------

static void cmd_handler(enum commends cmd)
{
    switch(cmd)
    {
    case GET_TIME:
        cmd_get_time_handler();
        break;
    case SET_TIME:
        cmd_set_time_handler();
        break;
    case GET_VALVE_STATE:
        cmd_get_valve_state_handler();
        break;
    case SET_VALVE_STATE:
        cmd_set_valve_state_handler();
        break;
    case GET_TEMP:
        cmd_get_temp_handler();
        break;
    case SET_TEMP:
        cmd_set_temp_handler();
        break;
    case TOGGLE_MONITOR:
        cmd_toggle_monitor_handler();
        break;
    case HELP:
        cmd_help_handler();
        break;
    case commends_enum_end:
        cmd_unknown_handler();
        break;
    case EMPTY_BUFFER:
        break;
    default:
        break;
    }
}

//--------------------------------------------------------------------------------

static void cmd_get_time_handler(void)
{
    snprintf(transmit_buffer, MAX_SIZE_TX_BUFFER, "Current time: %s\r\n", beer_machine_ctx.time);
    USART_WriteString(transmit_buffer);
}

//--------------------------------------------------------------------------------

static void cmd_set_time_handler(void)
{
    if(check_if_number(tmp_cmd_buffer[9]) && check_if_number(tmp_cmd_buffer[10]) &&
       check_if_number(tmp_cmd_buffer[12]) && check_if_number(tmp_cmd_buffer[13]) &&
       check_if_number(tmp_cmd_buffer[15]) && check_if_number(tmp_cmd_buffer[16]) &&
       tmp_cmd_buffer[11] == ':' && tmp_cmd_buffer[14] == ':')
    {
        for(uint8_t i = 0; i < 8; i++)
        {
            beer_machine_ctx.time[i] = tmp_cmd_buffer[9 + i];
        }
        snprintf(transmit_buffer, MAX_SIZE_TX_BUFFER, "Time set correctly.\r\n");
        USART_WriteString(transmit_buffer);
    }
    else
    {
        wrong_argument();
    }
}

//--------------------------------------------------------------------------------

static void cmd_get_valve_state_handler(void)
{
    snprintf(transmit_buffer, MAX_SIZE_TX_BUFFER, "Valve state: %s\r\n", (beer_machine_ctx.valve_state) ? "open" : "close");
    USART_WriteString(transmit_buffer);
}

//--------------------------------------------------------------------------------

static void cmd_set_valve_state_handler(void)
{
    struct cmd_struct compare_tab[2] = {[0] = {.cmd = "close", .size = 5},
                                        [1] = {.cmd = "open", .size = 4},};

    for (uint8_t arg = 0; arg < (sizeof(compare_tab) / sizeof(compare_tab[0])); arg++ )
    {
        for (uint8_t sign = 0; sign < compare_tab[arg].size; sign++)
        {
            if (!(tmp_cmd_buffer[16 + sign] == compare_tab[arg].cmd[sign]))
            {
                break;
            }

            if (sign == compare_tab[arg].size - 1)
            {
                if (arg == 0)
                {
                    beer_machine_ctx.valve_state = false;
                    snprintf(transmit_buffer, MAX_SIZE_TX_BUFFER, "Valve state set correctly.\r\n");
                    USART_WriteString(transmit_buffer);
                    return;
                }
                else if (arg == 1)
                {
                    beer_machine_ctx.valve_state = true;
                    snprintf(transmit_buffer, MAX_SIZE_TX_BUFFER, "Valve state set correctly.\r\n");
                    USART_WriteString(transmit_buffer);
                    return;
                }
            }
        }
    }
    wrong_argument();
}

//--------------------------------------------------------------------------------

static void cmd_get_temp_handler(void)
{
    snprintf(transmit_buffer, MAX_SIZE_TX_BUFFER, "Current temperature: %s\r\n", beer_machine_ctx.temp);
    USART_WriteString(transmit_buffer);
}

//--------------------------------------------------------------------------------

static void cmd_set_temp_handler(void)
{
    if(check_if_number(tmp_cmd_buffer[9]) && check_if_number(tmp_cmd_buffer[10]) && tmp_cmd_buffer[11] == 'C')
    {
        for(uint8_t i = 0; i < 3; i++)
        {
            beer_machine_ctx.temp[i] = tmp_cmd_buffer[9 + i];
        }
        snprintf(transmit_buffer, MAX_SIZE_TX_BUFFER, "Temperature set correctly.\r\n");
        USART_WriteString(transmit_buffer);
    }
    else
    {
        wrong_argument();
    }
}

//--------------------------------------------------------------------------------

static void cmd_toggle_monitor_handler(void)
{
    beer_machine_ctx.monitor ^= true;

    if(beer_machine_ctx.monitor)
    {
        HAL_TIM_Base_Start_IT(&htim4);
    }
    else
    {
        beer_machine_ctx.monitor_first_write = true;
        beer_machine_ctx.tmp_var = 0;
        HAL_TIM_Base_Stop_IT(&htim4);
    }

    snprintf(transmit_buffer, MAX_SIZE_TX_BUFFER, "Monitor toggle.\r\n");
    USART_WriteString(transmit_buffer);
}

//--------------------------------------------------------------------------------

static void cmd_help_handler(void)
{
    snprintf(transmit_buffer, MAX_SIZE_TX_BUFFER,   "get time           This command return current time.\r\n"
                                                    "set time           With this command user can set current time.\r\n"
                                                    "get valve state    This command return valve state.\r\n"
                                                    "set valve state    With this command user can set valve state.\r\n"
                                                    "get temp           This command return current temperature.\r\n"
                                                    "set temp           With this command user can set current temperature.\r\n"
                                                    "toggle monitor     Toggle monitor.\r\n"
                                                    );

    USART_WriteString(transmit_buffer);
}

//--------------------------------------------------------------------------------

static void cmd_unknown_handler(void)
{
    snprintf(transmit_buffer, MAX_SIZE_TX_BUFFER, "Unknown command! Write \"help\" to get list of all commands.\r\n");
    USART_WriteString(transmit_buffer);
}

//--------------------------------------------------------------------------------

static void wrong_argument(void)
{
    snprintf(transmit_buffer, MAX_SIZE_TX_BUFFER, "Incorrect command argument!\r\n");
    USART_WriteString(transmit_buffer);
}

//--------------------------------------------------------------------------------

static void monitor_print(void)
{
    if(beer_machine_ctx.monitor_first_write)
    {
        beer_machine_ctx.monitor_first_write = false;
        snprintf(transmit_buffer, MAX_SIZE_TX_BUFFER, "\r\nCurrent time: %s\r\nCurrent temperature: %s\r\nValve state: %s\r\nTmp: %u\r\n", beer_machine_ctx.time, beer_machine_ctx.temp, (beer_machine_ctx.valve_state) ? "open" : "close", beer_machine_ctx.tmp_var);
    }
    else
    {
        snprintf(transmit_buffer, MAX_SIZE_TX_BUFFER, "\r\nCurrent time: %s\r\nCurrent temperature: %s\r\nValve state: %s\r\nTmp: %u\r\n", beer_machine_ctx.time, beer_machine_ctx.temp, (beer_machine_ctx.valve_state) ? "open" : "close", beer_machine_ctx.tmp_var);
        for(uint32_t i = 0; i < (strlen(transmit_buffer) + strlen(feedback_cmd) - 5); i++)
        {
            USART_WriteString("\b");
        }
    }

    USART_WriteString(transmit_buffer);
    USART_WriteString(feedback_cmd);
    beer_machine_ctx.tmp_var++;
}

//--------------------------------------------------------------------------------

static bool check_if_number(char c)
{
    if(c >= 48 && c <= 57)
    {
        return true;
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------------------------------------

static enum commends cmd_search(void)
{
    for(uint8_t cmd = 0; cmd < commends_enum_end; cmd++)
    {
        for(uint8_t sign = 0; sign < cmd_tab[cmd].size; sign++)
        {
            if(!(tmp_cmd_buffer[sign] == cmd_tab[cmd].cmd[sign]))
            {
                break;
            }

            if(sign == cmd_tab[cmd].size - 1)
            {
                return cmd;
            }
        }
    }
    return commends_enum_end;
}

//--------------------------------------------------------------------------------

static bool backspace_handler(void)
{
    for(size_t i = 0; i < MAX_SIZE_CMD_BUFFER; i++)
    {
        if(tmp_cmd_buffer[i] == 8)
        {
            if(!i)
            {
                for(size_t j = 0; j < MAX_SIZE_CMD_BUFFER - 1; j++)
                {
                    tmp_cmd_buffer[j] = tmp_cmd_buffer[j + 1];
                }
            }
            else
            {
                for(size_t j = i; j < MAX_SIZE_CMD_BUFFER - i; j++)
                {
                    tmp_cmd_buffer[j - 1] = tmp_cmd_buffer[j + 1];
                }
            }
            return true;
        }
    }
    return false;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim3)
    {
        cmd_handler_process();
    }
    else if (htim == &htim4)
    {
        monitor_print();
    }
}
