#include "motor.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"

motor::motor(const uint _ain1_pin, const uint _ain2_pin, const uint _bin1_pin, const uint _bin2_pin) : ain1_pin(_ain1_pin), ain2_pin(_ain2_pin), bin1_pin(_bin1_pin), bin2_pin(_bin2_pin) {
    gpio_init(ain1_pin);
    gpio_init(ain2_pin);
    gpio_init(bin1_pin);
    gpio_init(bin2_pin);
    gpio_set_dir(ain1_pin, GPIO_OUT);
    gpio_set_dir(ain2_pin, GPIO_OUT);
    gpio_set_dir(bin1_pin, GPIO_OUT);
    gpio_set_dir(bin2_pin, GPIO_OUT);

    // Tell GPIO 16 and 21 they are allocated to the PWM
    gpio_set_function(16, GPIO_FUNC_PWM);
    gpio_set_function(21, GPIO_FUNC_PWM);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 4.f);

    // Find out which PWM slice is connected to GPIO 16 and GPIO 21
    _slice_num_a = pwm_gpio_to_slice_num(16);
    pwm_init(_slice_num_a, &config, true);
    _slice_num_b = pwm_gpio_to_slice_num(21);
    pwm_init(_slice_num_b, &config, true);

    // disable PWM at startup
    pwm_set_enabled(_slice_num_a, true);
    pwm_set_enabled(_slice_num_b, true);
}

motor::~motor() {
}

void motor::forward(int speed) {
    if ((speed >= 0) and (speed <= 100)) {
        pwm_set_chan_level(_slice_num_a, PWM_CHAN_A, int(speed*0xFFFF/100));
        pwm_set_chan_level(_slice_num_b, PWM_CHAN_B, int(speed*0xFFFF/100));
        gpio_put(ain1_pin, 1);
        gpio_put(ain2_pin, 0);
        gpio_put(bin1_pin, 0);
        gpio_put(bin2_pin, 1);
    }
}

void motor::backward(int speed) {
    if ((speed >= 0) and (speed <= 100)) {
        pwm_set_chan_level(_slice_num_a, PWM_CHAN_A, int(speed*0xFFFF/100));
        pwm_set_chan_level(_slice_num_b, PWM_CHAN_B, int(speed*0xFFFF/100));
        gpio_put(ain1_pin, 0);
        gpio_put(ain2_pin, 1);
        gpio_put(bin1_pin, 1);
        gpio_put(bin2_pin, 0);
    }
}

void motor::left(int speed) {
    if ((speed >= 0) and (speed <= 100)) {
        pwm_set_chan_level(_slice_num_a, PWM_CHAN_A, int(speed*0xFFFF/100));
        pwm_set_chan_level(_slice_num_b, PWM_CHAN_B, int(speed*0xFFFF/100));
        gpio_put(ain1_pin, 0);
        gpio_put(ain2_pin, 1);
        gpio_put(bin1_pin, 0);
        gpio_put(bin2_pin, 1);
    }
}

void motor::right(int speed) {
    if ((speed >= 0) and (speed <= 100)) {
        pwm_set_chan_level(_slice_num_a, PWM_CHAN_A, int(speed*0xFFFF/100));
        pwm_set_chan_level(_slice_num_b, PWM_CHAN_B, int(speed*0xFFFF/100));
        gpio_put(ain1_pin, 1);
        gpio_put(ain2_pin, 0);
        gpio_put(bin1_pin, 1);
        gpio_put(bin2_pin, 0);
    }
}

void motor::stop() {
    pwm_set_chan_level(_slice_num_a, PWM_CHAN_A, 0);
    pwm_set_chan_level(_slice_num_b, PWM_CHAN_B, 0);
    gpio_put(ain1_pin, 0);
    gpio_put(ain2_pin, 0);
    gpio_put(bin1_pin, 0);
    gpio_put(bin2_pin, 0);
}

void motor::set_motor(int left, int right) {
    if ((left >= 0) and (left <= 100)) {
        gpio_put(ain1_pin, 1);
        gpio_put(ain2_pin, 0);
        pwm_set_chan_level(_slice_num_a, PWM_CHAN_A, int(left*0xFFFF/100));
    }
    else if ((left < 0) and (left >= -100)) {
        gpio_put(ain1_pin, 0);
        gpio_put(ain2_pin, 1);
        pwm_set_chan_level(_slice_num_a, PWM_CHAN_A, -int(left*0xFFFF/100));
    }

    if ((right >= 0) and (right <= 100)) {
        gpio_put(bin1_pin, 0);
        gpio_put(bin2_pin, 1);
        pwm_set_chan_level(_slice_num_b, PWM_CHAN_B, int(right*0xFFFF/100));
    }
    else if ((right < 0) and (right >= -100)) {
        gpio_put(bin1_pin, 1);
        gpio_put(bin2_pin, 2);
        pwm_set_chan_level(_slice_num_b, PWM_CHAN_B, -int(right*0xFFFF/100));
    }
}