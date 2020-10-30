/*******************************************************************************
 * @file    command.c
 * @author  MCD Application Team
 * @version V1.1.2
 * @date    08-September-2017
 * @brief   main command driver dedicated to command AT
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V.
 * All rights reserved.</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted, provided that the following conditions are met:
 *
 * 1. Redistribution of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of STMicroelectronics nor the names of other
 *    contributors to this software may be used to endorse or promote products
 *    derived from this software without specific written permission.
 * 4. This software, including modifications and/or derivative works of this
 *    software, must execute solely and exclusively on microcontroller or
 *    microprocessor devices manufactured by or for STMicroelectronics.
 * 5. Redistribution and use of this software other than as permitted under
 *    this license is void and will automatically terminate your rights under
 *    this license.
 *
 * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
 * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
 * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include "at.h"
//#include "hw.h"
#include "vcom.h"
#include "command.h"


#if (_PEDOMETER_)

/* comment the following to have help message */
/* #define NO_HELP */
/* #define NO_KEY_ADDR_EUI */

/* Private typedef -----------------------------------------------------------*/
/**
 * @brief  Structure defining an AT Command
 */
struct ATCommand_s {
  const char *string;                       /*< command string, after the "AT" */
  const int size_string;                    /*< size of the command string, not including the final \0 */
  ATEerror_t (*get)(const char *param);     /*< =? after the string to get the current value*/
  ATEerror_t (*set)(const char *param);     /*< = (but not =?\0) after the string to set a value */
  ATEerror_t (*run)(const char *param);     /*< \0 after the string - run the command */
#if !defined(NO_HELP)
  const char *help_string;                  /*< to be printed when ? after the string */
#endif
};

/* Private define ------------------------------------------------------------*/
#define CMD_SIZE 128

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

#define NO_HELP

/**
 * @brief  Array corresponding to the description of each possible AT Error
 */
static const char *const ATError_description[] =
{
  "+OK\r",                	/* AT_OK */
  "+ERR\r",               	/* AT_ERROR */
  "+ERR_PARAM\r",         	/* AT_PARAM_ERROR */
  "+ERR_BUSY\r",          	/* AT_BUSY_ERROR */
  "+ERR_PARAM_OVERFLOW\r", 	/* AT_TEST_PARAM_OVERFLOW */
  "+ERR_NO_NETWORK\r",   	/* AT_NO_NET_JOINED */
  "+ERR_RX\r",            	/* AT_RX_ERROR */
  "+ERR_UNKNOWN\r",         /* AT_MAX */
};

/**
 * @brief  Array of all supported AT Commands
 */
static const struct ATCommand_s ATCommand[] =
{
  {
    .string = AT_RESET,
    .size_string = sizeof(AT_RESET) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_RESET ": Trig a reset of the MCU\r\n",
#endif
    .get = at_return_error,
    .set = at_return_error,
    .run = at_reset,
  },

#ifndef NO_KEY_ADDR_EUI
  {
    .string = AT_GSTEP,
    .size_string = sizeof(AT_GSTEP) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_GSTEP ": Get the step count\r\n",
#endif
    .get = at_Pedometer_get,
    .set = at_return_error,
    .run = at_return_error,
  },
#endif
  
};


/* Private function prototypes -----------------------------------------------*/

/**
 * @brief  Print a string corresponding to an ATEerror_t
 * @param  The AT error code
 * @retval None
 */
static void com_error(ATEerror_t error_type);

/**
 * @brief  Parse a command and process it
 * @param  The command
 * @retval None
 */
static void parse_cmd(const char *cmd);

/* Exported functions ---------------------------------------------------------*/

void CMD_Init(void)
{
  vcom_Init();
  vcom_ReceiveInit();
}

char command[CMD_SIZE];

void CMD_Process(void)
{
  //static char command[CMD_SIZE];
  static unsigned i = 0;

  /* Process all commands */
  while (IsNewCharReceived() == SET)
  {
    command[i] = GetNewChar();

#if 0 /* echo On    */
    PRINTF("%c", command[i]);
#endif

    if (command[i] == AT_ERROR_RX_CHAR) {
      i = 0;
      com_error(AT_RX_ERROR);
      break;
    } else {
      if (command[i] == ('\r')) {
        if (i != 0) {
          command[i] = '\0';
          parse_cmd(command);
          i = 0;
        }
      } else {
        if (i == (CMD_SIZE - 1)) {
          i = 0;
          com_error(AT_TEST_PARAM_OVERFLOW);
        } else {
          i++;
        }
      }
    }
  }
}

/* Private functions ---------------------------------------------------------*/

static void com_error(ATEerror_t error_type)
{
  if (error_type > AT_MAX)
  {
    error_type = AT_MAX;
  }
  AT_PRINTF(ATError_description[error_type]);
}


static void parse_cmd(const char *cmd)
{
  ATEerror_t status = AT_OK;
  const struct ATCommand_s *Current_ATCommand;
  int i;
  uint8_t confirm_set = 0;

  if ((cmd[0] != 'A') || (cmd[1] != 'T'))
  {
    status = AT_ERROR;
  }
  else
  if (cmd[2] == '\0')
  {
    /* status = AT_OK; */
    com_error(status);
  }
  else
  {
    /* point to the start of the command, excluding AT */
    status = AT_ERROR;
    cmd += 2;
    for (i = 0; i < (sizeof(ATCommand) / sizeof(struct ATCommand_s)); i++)
    {
      if (strncmp(cmd, ATCommand[i].string, ATCommand[i].size_string) == 0)
      {
        Current_ATCommand = &(ATCommand[i]);
        /* point to the string after the command to parse it */
        cmd += Current_ATCommand->size_string;

        /* parse after the command */
        switch (cmd[0])
        {
          case '\0': /* nothing after the command */
            status = Current_ATCommand->run(cmd);
            break;
          case '?':
            status = Current_ATCommand->get(cmd + 1);
            break;
          case '=':
          case ' ': // special case for CTX and UTX
            status = Current_ATCommand->set(cmd + 1);
            confirm_set = 1;
            break;
          default:
            /* not recognized */
            break;
        }

        /* we end the loop as the command was found */
        break;
      }
    }
  }
  if (status != AT_OK || (confirm_set == 1)) {
    com_error(status);
  }
}


#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
