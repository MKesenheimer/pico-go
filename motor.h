#pragma once
#include "hardware/pio.h"

class motor {
private:
    uint _slice_num_a;
    uint _slice_num_b;
public:
    const uint ain2_pin;
    const uint ain1_pin;
    const uint bin1_pin;
    const uint bin2_pin;
    motor(const uint _ain1_pin = 17, const uint _ain2_pin = 18, const uint _bin1_pin = 19, const uint _bin2_pin = 20);
    ~motor();
    void forward(int speed);
    void backward(int speed);
    void left(int speed);
    void right(int speed);
    void stop();
    void set_motor(int left, int right);
};
