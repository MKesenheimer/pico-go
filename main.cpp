#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <numeric>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "trsensor.h"
#include "helper.h"
#include "motor.h"
#include "bluetooth.h"
#include "parser.h"
#include "gfx.h"

// bluetooth data is in the format:
// P%0.99,;
// or
// P%0.99,I+0.5,D+3,;
void parse_bluetooth_data(const std::string& data, const std::string& match, float& pid) {
    //std::cout << "data = " << data << std::endl;
    std::string parsed = parser::parse_string(data, match, ",");
    //std::cout << "parsed(" << match << ") = " << parsed << std::endl;
    if (parsed != std::string() && parser::is_percent(parsed)) {
        parsed = parsed.substr(1, parsed.length()); // remove the percent symbol
        //std::cout << "parsed(" << match << ") = " << parsed << std::endl;
        float fact = parser::to_number<float>(parsed);
        //std::cout << "fact(" << match << ") = " << fact << std::endl;
        if (pid*fact > 0)
            pid *= fact;
    }
    else if (parsed != std::string() && parser::is_delta(parsed)) {
        float delta = parser::to_number<float>(parsed);
        if (pid + delta > 0)
            pid += delta;
    }
    else if (parsed != std::string()) {
        float n = parser::to_number<float>(parsed);
        if (n > 0)
            pid = n;
    }
}

void apply_bluetooth_data(const std::string& data, float& p, float& i, float& d) {
    //std::cout << data << std::endl;
    if (parser::contains(data, "P"))
        parse_bluetooth_data(data, "P", p);
    if (parser::contains(data, "I"))
        parse_bluetooth_data(data, "I", i);
    if (parser::contains(data, "D"))
        parse_bluetooth_data(data, "D", d);
}

int main() {
    set_sys_clock_khz(133000, true);

    stdio_init_all();
    std::cout << "PicoGo!\n" << std::endl;

    // init display
    gfx display(false, true, true, false);
    display.clear();
    display.draw_string(70, 70, "Pico-Go!", 0xffff);
    display.show();

    // bluetooth handling
    //bluetooth bt;
    bluetooth* bt = bluetooth::get_instance();

    // IR sensors
    tr_sensor sensors;
    //sensors.calibrate();
    sensors.fixed_calibration();

    // motor controller
    motor motors;

    // pid controller
    int16_t integral = 0;
    int16_t proportional = 0;
    int16_t derivative = 0;
    int16_t last_proportional = 0;
    int16_t power_difference = 0;
    int16_t maximum = 45;
    float p = 0.06;
    float i = 0.00003;
    float d = 0.0001;

    busy_wait_ms(1000);
    while (1) {
        // handle bluetooth data
        if (bt->data_available()) {
            std::string data = bt->get_buffer();
            apply_bluetooth_data(data, p, i, d);
            std::cout << "p = " << p << ", i = " << i << ", d = " << d << std::endl;
            display.clear();
            display.draw_string(70, 70, "p = " + std::to_string(p), 0xffff);
            display.draw_string(70, 90, "i = " + std::to_string(i), 0xffff);
            display.draw_string(70, 110, "d = " + std::to_string(d), 0xffff);
            display.show();
        }


        // read position
        auto position_sensor = sensors.read_line();
        uint16_t position = std::get<0>(position_sensor);
        auto sensor_values = std::get<1>(position_sensor);
        uint16_t sensor_sum = std::accumulate(std::begin(sensor_values), std::end(sensor_values), 0);

        if (sensor_sum < 6000) {
            // The "proportional" term should be 0 when we are on the line.
            proportional = position - 3000;

            // Compute the derivative (change) and integral (sum) of the position.
            derivative = proportional - last_proportional;
            integral += proportional;
            int16_t max_int = 2000;
            if (integral >= max_int)
                integral = max_int;
            else if (integral <= -max_int)
                integral = -max_int;

            // Remember the last position.
            last_proportional = proportional;

            // apply values
            power_difference = proportional * p  + derivative * d + integral * i;
            //std::cout << "power_difference = " << power_difference << std::endl;

            if (power_difference > maximum)
                power_difference = maximum;
            if (power_difference < - maximum)
                power_difference = - maximum;

#if 1
            if (power_difference > 0)
                motors.set_motor(maximum - power_difference, maximum);
            else
                motors.set_motor(maximum, maximum + power_difference);
#endif
        } 
        else {
            motors.stop();
        }

        //std::cout << "position = " << position << std::endl; 
        //helper::std_print_sensor_values<NUM_SENSORS>(sensor_values);
        //busy_wait_ms(10);
    }

    return 0;
}
