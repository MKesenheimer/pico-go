#pragma once
#include <stdlib.h>
#include <string>
#include "font.h"
#include "st7789_lcd.h"

using colors = uint16_t;

class gfx {
    bool _mirror_x;
    bool _mirror_y;
    bool _swap_xy;
    bool _upside_down;

    const uint8_t* _font = font_8x5;
    uint16_t _framebuffer[SCREEN_WIDTH][SCREEN_HEIGHT];

    PIO _pio;
    uint _sm;

    static void swap(int &a, int &b);
    static void mirror_x(int &x);
    static void mirror_y(int &y);
    void transform(int &x, int &y);

    public:
        gfx(bool mirror_x, bool mirror_y, bool swap_xy, bool upside_down);
        void draw_pixel(int x, int y, colors color);
        void draw_char(int x, int y, char chr, colors color);
        void draw_string(int x, int y, std::string str, colors color);
        void draw_progress_bar(int x, int y, uint16_t w, uint16_t h, uint8_t progress, colors color);
        void draw_fill_rectangle(int x, int y, uint16_t w, uint16_t h, colors color);
        void draw_rectangle(int x, int y, uint16_t w, uint16_t h, colors color);
        void draw_horizontal_line(int x, int y, int w, colors color);
        void draw_vertical_line(int x, int y, int w, colors color);
        void draw_line(int x_start, int y_start, int x_end, int y_end, colors color);
        void show();
        void clear();
};