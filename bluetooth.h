#pragma once
#include <string>

#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1

// note: this singleton class is not thread safe!
class bluetooth {
private:
    static std::string s_buffer;
    static bool s_data_available;
protected:
    bluetooth();
    static bluetooth* _singleton;
public:
    bluetooth(bluetooth &other) = delete;
    void operator=(const bluetooth&) = delete;
    static bluetooth *get_instance();

    // RX interrupt handler
    static void on_uart_rx();

    std::string get_buffer();
    bool data_available();
};