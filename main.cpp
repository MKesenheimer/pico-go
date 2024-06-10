#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <numeric>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "tr-sensor.h"
#include "helper.h"
#include "motor.h"

int main() {
    stdio_init_all();
    std::cout << "PicoGo!\n" << std::endl;

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
    int16_t maximum = 40;
    float p = 0.05;
    float i = 0.0;
    float d = 0.0;

    while (1) {
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
            if (integral >= 1000)
                integral = 1000;
            else if (integral <= -1000)
                integral = -1000;

            // Remember the last position.
            last_proportional = proportional;

            // apply values
            power_difference = proportional * p  + derivative * d + integral * i;
            std::cout << "power_difference = " << power_difference << std::endl;

            if (power_difference > maximum)
                power_difference = maximum;
            if (power_difference < - maximum)
                power_difference = - maximum;
        
            if (power_difference > 0)
                motors.set_motor(maximum - power_difference, maximum);
            else
                motors.set_motor(maximum, maximum + power_difference);
        }

        //std::cout << "position = " << position << std::endl; 
        //helper::std_print_sensor_values<NUM_SENSORS>(sensor_values);
        //busy_wait_ms(10);
    }

    return 0;
}
