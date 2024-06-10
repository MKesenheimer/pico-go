#include <iostream>
#include "pico/stdlib.h"
#include "tr-sensor.h"
#include "helper.h"
#include "spi.pio.h"
#include "pico/binary_info.h"

tr_sensor::tr_sensor(const uint _buffer_size, const uint _clk_pin, const uint _mosi_pin, const uint _miso_pin, const uint _cs_pin) : buffer_size(_buffer_size), clk_pin(_clk_pin), mosi_pin(_mosi_pin), miso_pin(_miso_pin), cs_pin(_cs_pin) {
    _pio = pio0;
    uint offset = pio_add_program(_pio, &spi_cpha0_program);
    _sm = pio_claim_unused_sm(_pio, true);

    pio_spi_init(_pio, _sm, offset,
                 12,       // 12 bits per SPI frame
                 156.25f / 2.f,  // 800 kHz @ 125 clk_sys
                 false,     // CPHA = 0
                 false,     // CPOL = 0
                 clk_pin,
                 mosi_pin,
                 miso_pin
    );

    // Make the 'SPI' pins available to picotool
    bi_decl(bi_4pins_with_names(miso_pin, "SPI RX", mosi_pin, "SPI TX", clk_pin, "SPI SCK", cs_pin, "SPI CS"));
    
    gpio_init(cs_pin);
    gpio_put(cs_pin, 1);
    gpio_set_dir(cs_pin, GPIO_OUT);
}

tr_sensor::~tr_sensor() {
}

std::array<uint16_t, NUM_SENSORS> tr_sensor::analog_read() {
    std::array<uint16_t, NUM_SENSORS+1> value;
    value.fill(0);
    uint offset = 0;
    for (uint32_t i = offset; i < offset + NUM_SENSORS+1; ++i) {
        gpio_put(cs_pin, 0);
        // set channel
        pio_sm_put_blocking(_pio, _sm, i << 28);
        // get last channe value
        value[i - offset] = pio_sm_get_blocking(_pio, _sm) & 0xfff;
        gpio_put(cs_pin, 1);
        value[i - offset] >>= 2;
        busy_wait_us(50);
    }
    std::array<uint16_t, NUM_SENSORS> ret;
    std::copy(value.begin()+1, value.end(), ret.begin());
    return ret;
}

void tr_sensor::calibrate() {
    _calibrated_min.fill(1023);
    _calibrated_max.fill(0);
    for (size_t j = 0; j < 10; ++j) {
        auto sensor_values = analog_read();
        for (size_t i = 0; i < NUM_SENSORS; ++i) {
            if (_calibrated_max[i] < sensor_values[i] && sensor_values[i] != 0)
                _calibrated_max[i] = sensor_values[i];
            if (_calibrated_min[i] > sensor_values[i] && sensor_values[i] != 0)
                _calibrated_min[i] = sensor_values[i];
        }
    }
    std::cout << "calibration:\nmin values: ";
    helper::std_print_sensor_values<NUM_SENSORS>(_calibrated_min);
    std::cout << "max values: ";
    helper::std_print_sensor_values<NUM_SENSORS>(_calibrated_max);
}

const std::array<uint16_t, NUM_SENSORS>& tr_sensor::get_calibrated_min() {
    return _calibrated_min;
}

const std::array<uint16_t, NUM_SENSORS>& tr_sensor::get_calibrated_max() {
    return _calibrated_max;
}

void tr_sensor::fixed_calibration() { 
    _calibrated_min = {117, 129, 124, 127, 101};
    _calibrated_max = {841, 899, 925, 945, 823};
}

const std::array<uint16_t, NUM_SENSORS> tr_sensor::read_calibrated() {
    uint16_t value = 0;
    std::array<uint16_t, NUM_SENSORS> sensor_values = analog_read();
        
    for (int i = 0; i < NUM_SENSORS; ++i) {
        uint16_t denominator = _calibrated_max[i] - _calibrated_min[i];
        if (denominator != 0)
            value = (sensor_values[i] - _calibrated_min[i]) * 1000 / denominator;
        if (value < 0)
            value = 0;
        else if (value > 1000)
            value = 1000;
        sensor_values[i] = int(value);
    }
    return sensor_values;
}

std::tuple<uint16_t, std::array<uint16_t, NUM_SENSORS>> tr_sensor::read_line(bool white_line) {
    auto sensor_values = read_calibrated();
    double avg = 0;
    double sum1 = 0;
    bool on_line = false;
    for (int i = 0; i < NUM_SENSORS; ++i) {
        uint16_t value = sensor_values[i];
        if (white_line)
            value = 1000 - value;
        
        // keep track of whether we see the line at all
        if (value < 800)
            on_line = true;
            
        // only average in values that are above a noise threshold
        if (value > 50)
            avg += value * ((i + 1) * 1000);  // this is for the weighted total,
            sum1 += value;                    // this is for the denominator 
    }

    if (on_line)
        _successive_not_on_line = 0;
    else
        _successive_not_on_line++;
    if (_successive_not_on_line >= _max_fails)
        _successive_not_on_line = _max_fails;

    if (_successive_not_on_line >= _max_fails) {
        //std::cout << "not on line" << std::endl;
        // If last read to the left of center, return min.
        if (_last_value < 3050) {
            //std::cout << "left" << std::endl;
            _last_value = 2500;
        }
        // If last read to the right of center, return the max.
        else {
            //std::cout << "right" << std::endl;
            _last_value = 3500;
        }
    }

    if (on_line) {
        //std::cout << "on line" << std::endl;
        if (sum1 != 0)
            _last_value = static_cast<uint16_t>(avg / sum1);
    }
    return std::make_tuple(uint16_t(_last_value), sensor_values);
}