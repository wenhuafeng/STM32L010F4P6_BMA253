#ifndef COMMAND_H
#define COMMAND_H

#ifdef __cplusplus
extern "C" {
#endif

#define AT_ERROR_RX_CHAR 0x01

void CMD_Init(void);
void CMD_Process(void);

#ifdef __cplusplus
}
#endif

#endif
