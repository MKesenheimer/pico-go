#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "bluetooth.h"
#include "pico/stdlib.h"
#include "hardware/uart.h"

bluetooth* bluetooth::_singleton= nullptr;
std::string bluetooth::s_buffer;
bool bluetooth::s_data_available;

bluetooth *bluetooth::get_instance() {
    if(_singleton== nullptr) {
        _singleton = new bluetooth();
    }
    return _singleton;
}

bluetooth::bluetooth() {
    // Set up UART with
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_fifo_enabled(UART_ID, false);
    int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;
    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ, true);
    uart_set_irq_enables(UART_ID, true, false);
}

void bluetooth::on_uart_rx() {
    while (uart_is_readable(UART_ID)) {
        char ch = uart_getc(UART_ID);
        s_buffer += ch;
        if (ch == ';' or ch == '\n')
            s_data_available = true;
        //std::cout << ch << std::endl;
    }
}

std::string bluetooth::get_buffer() {
    s_data_available = false;
    std::string ret = s_buffer;
    s_buffer = std::string();
    return std::move(ret);
}

bool bluetooth::data_available() {
    return s_data_available;
}