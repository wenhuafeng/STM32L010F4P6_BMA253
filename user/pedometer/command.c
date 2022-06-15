#include <stdlib.h>
#include <string.h>
#include "at.h"
#include "vcom.h"
#include "command.h"

#if defined(PEDOMETER) && PEDOMETER

struct ATCommand_s {
    const char *string;                   /*< command string, after the "AT" */
    const int size_string;                /*< size of the command string, not including the final \0 */
    ATEerror_t (*get)(const char *param); /*< =? after the string to get the current value*/
    ATEerror_t (*set)(const char *param); /*< = (but not =?\0) after the string to set a value */
    ATEerror_t (*run)(const char *param); /*< \0 after the string - run the command */
#if !defined(NO_HELP)
    const char *help_string; /*< to be printed when ? after the string */
#endif
};

#define CMD_SIZE 128

#define NO_HELP

static const char *const ATError_description[] = {
    "+OK\r",                 /* AT_OK */
    "+ERR\r",                /* AT_ERROR */
    "+ERR_PARAM\r",          /* AT_PARAM_ERROR */
    "+ERR_BUSY\r",           /* AT_BUSY_ERROR */
    "+ERR_PARAM_OVERFLOW\r", /* AT_TEST_PARAM_OVERFLOW */
    "+ERR_NO_NETWORK\r",     /* AT_NO_NET_JOINED */
    "+ERR_RX\r",             /* AT_RX_ERROR */
    "+ERR_UNKNOWN\r",        /* AT_MAX */
};

static const struct ATCommand_s ATCommand[] = {
    {
            .string = AT_RESET,
            .size_string = sizeof(AT_RESET) - 1,
#ifndef NO_HELP
            .help_string = "AT" AT_RESET ": Trig a reset of the MCU\r\n",
#endif
            .get = AT_ReturnError,
            .set = AT_ReturnError,
            .run = AT_Reset,
    },

    {
            .string = AT_GSTEP,
            .size_string = sizeof(AT_GSTEP) - 1,
#ifndef NO_HELP
            .help_string = "AT" AT_GSTEP ": Get the step count\r\n",
#endif
            .get = AT_GetPedometer,
            .set = AT_ReturnError,
            .run = AT_ReturnError,
    },

};

static void com_error(ATEerror_t err)
{
    if (err > AT_MAX) {
        err = AT_MAX;
    }
    PRINTF(ATError_description[err]);
}

static void parse_cmd(const char *cmd)
{
    ATEerror_t status = AT_OK;
    const struct ATCommand_s *Current_ATCommand;
    int i;
    uint8_t confirm_set = 0;

    if ((cmd[0] != 'A') || (cmd[1] != 'T')) {
        status = AT_ERROR;
    } else if (cmd[2] == '\0') {
        /* status = AT_OK; */
        com_error(status);
    } else {
        /* point to the start of the command, excluding AT */
        status = AT_ERROR;
        cmd += 2;
        for (i = 0; i < (sizeof(ATCommand) / sizeof(struct ATCommand_s)); i++) {
            if (strncmp(cmd, ATCommand[i].string, ATCommand[i].size_string) == 0) {
                Current_ATCommand = &(ATCommand[i]);
                /* point to the string after the command to parse it */
                cmd += Current_ATCommand->size_string;

                /* parse after the command */
                switch (cmd[0]) {
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

void CMD_Init(void)
{
    vcom_Init();
    vcom_ReceiveInit();
}

void CMD_Process(void)
{
    static char command[CMD_SIZE];
    static unsigned i = 0;

    /* Process all commands */
    while (vcom_IsNewCharReceived() == SET) {
        command[i] = vcom_GetNewChar();

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

#endif
