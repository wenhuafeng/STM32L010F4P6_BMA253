#include "vcom.h"
#include <stdarg.h>
#include <stdint.h>
#include "main.h"
#include "tiny_vsnprintf.h"

static struct {
    char buffTx[256];
    char buffRx[256];
    int rx_idx_free;
    int rx_idx_toread;
    HW_LockTypeDef Lock;
} uart_context;

static int buffer_transmit(int start, int len)
{
    int i;

    for (i = start; i < len; i++) {
        LL_LPUART_ClearFlag_TC(LPUART1);
        LL_LPUART_TransmitData8(LPUART1, uart_context.buffTx[i]);

        while (LL_LPUART_IsActiveFlag_TC(LPUART1) != SET) {
            ;
        }
    }
    LL_LPUART_ClearFlag_TC(LPUART1);

    return len;
}

static void receive(char rx)
{
    int next_free;

    uart_context.buffRx[uart_context.rx_idx_free] = rx;
    next_free = (uart_context.rx_idx_free + 1) % sizeof(uart_context.buffRx);
    if (next_free != uart_context.rx_idx_toread) {
        uart_context.rx_idx_free = next_free;
    } else {
        uart_context.buffRx[uart_context.rx_idx_free] = '\r';
        PRINTF("uart_context.buffRx buffer overflow %d\r\n");
    }
}

void vcom_Init(void)
{
    LL_LPUART_EnableInStopMode(LPUART1);
    LL_LPUART_SetWKUPType(LPUART1, LL_LPUART_WAKEUP_ON_STARTBIT);
    LL_LPUART_EnableIT_WKUP(LPUART1);
    LL_LPUART_Enable(LPUART1);
    while (LL_LPUART_IsActiveFlag_TEACK(LPUART1) == RESET) {
        ;
    }
    while (LL_LPUART_IsActiveFlag_REACK(LPUART1) == RESET) {
        ;
    }
}

void vcom_ReceiveInit(void)
{
    HW_LOCK(&uart_context);

    LL_LPUART_EnableIT_PE(LPUART1);
    LL_LPUART_EnableIT_ERROR(LPUART1);

    HW_UNLOCK(&uart_context);
}

void vcom_Send(const char *format, ...)
{
    va_list args;
    static __IO uint16_t len = 0;
    uint16_t current_len;
    int start;
    int stop;

    va_start(args, format);

    //BACKUP_PRIMASK();
    DISABLE_IRQ();
    if (len != 0) {
        if (len != sizeof(uart_context.buffTx)) {
            current_len = len;
            len = current_len + tiny_vsnprintf_like(uart_context.buffTx + current_len,
                                                    sizeof(uart_context.buffTx) - current_len, format, args);
        }
        ENABLE_IRQ();
        //RESTORE_PRIMASK();
        va_end(args);
        return;
    } else {
        len = tiny_vsnprintf_like(uart_context.buffTx, sizeof(uart_context.buffTx), format, args);
    }

    current_len = len;
    ENABLE_IRQ();
    //RESTORE_PRIMASK();

    start = 0;

    do {
        stop = buffer_transmit(start, current_len);
        //BACKUP_PRIMASK();
        DISABLE_IRQ();
        if (len == stop) {
            len = 0;
        } else {
            start = stop;
            current_len = len;
        }
        ENABLE_IRQ();
        //RESTORE_PRIMASK();
    } while (current_len != stop);

    va_end(args);
}

FlagStatus IsNewCharReceived(void)
{
    FlagStatus status;

    //BACKUP_PRIMASK();
    DISABLE_IRQ();

    status = ((uart_context.rx_idx_toread == uart_context.rx_idx_free) ? RESET : SET);

    ENABLE_IRQ();
    //RESTORE_PRIMASK();

    return status;
}

uint8_t GetNewChar(void)
{
    uint8_t NewChar;

    //BACKUP_PRIMASK();
    DISABLE_IRQ();

    NewChar = uart_context.buffRx[uart_context.rx_idx_toread];
    uart_context.rx_idx_toread = (uart_context.rx_idx_toread + 1) % sizeof(uart_context.buffRx);

    ENABLE_IRQ();
    //RESTORE_PRIMASK();

    return NewChar;
}

void vcom_IRQHandler(void)
{
    int rx_ready = 0;
    char rx;

    if (LL_LPUART_IsActiveFlag_WKUP(LPUART1) && (LL_LPUART_IsEnabledIT_WKUP(LPUART1) != RESET)) {
        LL_LPUART_ClearFlag_WKUP(LPUART1);
        LL_LPUART_EnableIT_RXNE(LPUART1);
    }

    if (LL_LPUART_IsActiveFlag_RXNE(LPUART1) && (LL_LPUART_IsEnabledIT_RXNE(LPUART1) != RESET)) {
        rx = LL_LPUART_ReceiveData8(LPUART1);
        rx_ready = 1;
    }

    if (LL_LPUART_IsActiveFlag_PE(LPUART1) || LL_LPUART_IsActiveFlag_FE(LPUART1) ||
        LL_LPUART_IsActiveFlag_ORE(LPUART1) || LL_LPUART_IsActiveFlag_NE(LPUART1)) {
        PRINTF("Error when receiving\n");
        LL_LPUART_ClearFlag_PE(LPUART1);
        LL_LPUART_ClearFlag_FE(LPUART1);
        LL_LPUART_ClearFlag_ORE(LPUART1);
        LL_LPUART_ClearFlag_NE(LPUART1);

        rx = AT_ERROR_RX_CHAR;
        rx_ready = 1;
    }

    if (rx_ready == 1) {
        receive(rx);
    }
}
