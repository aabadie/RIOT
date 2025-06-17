#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "compiler_hints.h"
#include "cpu.h"
#include "periph/uart.h"


static uart_isr_ctx_t _uart_isr_ctx = { 0 };

int uart_init(uart_t uart, uint32_t baudrate, uart_rx_cb_t rx_cb, void *arg)
{
    (void)uart;
    (void)baudrate;

    _uart_isr_ctx.rx_cb = rx_cb;
    _uart_isr_ctx.arg = arg;

    NVIC_EnableIRQ(UART_IRQn);
    return 0;
}

void uart_poweron(uart_t uart)
{
    (void)uart;
}

void uart_poweroff(uart_t uart)
{
    (void)uart;
}

void uart_write(uart_t uart, const uint8_t *data, size_t len)
{
    (void)len;
    (void)uart;
    for (size_t i = 0; i < len; i++) {
        SCUM_UART->DATA = (uint32_t)data[i];
    }
}

void UART_Handler(void)
{
    if (_uart_isr_ctx.rx_cb) {
        _uart_isr_ctx.rx_cb(_uart_isr_ctx.arg, (uint8_t)SCUM_UART->DATA);
    }
}
