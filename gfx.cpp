#include "gfx.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"

gfx::gfx(bool mirror_x, bool mirror_y, bool swap_xy, bool upside_down) : _mirror_x(mirror_x), _mirror_y(mirror_y), _swap_xy(swap_xy), _upside_down(upside_down) {
    _pio = ST7789_PIO;
    _sm = 0;
    uint offset = pio_add_program(_pio, &st7789_lcd_program);
    st7789_lcd_program_init(_pio, _sm, offset, PIN_DIN, PIN_CLK, SERIAL_CLK_DIV);

    gpio_init(PIN_CS);
    gpio_init(PIN_DC);
    gpio_init(PIN_RESET);
    gpio_init(PIN_BL);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_set_dir(PIN_DC, GPIO_OUT);
    gpio_set_dir(PIN_RESET, GPIO_OUT);
    gpio_set_dir(PIN_BL, GPIO_OUT);

    gpio_put(PIN_CS, 1);
    gpio_put(PIN_RESET, 1);
    lcd_init(_pio, _sm, st7789_init_seq);
    gpio_put(PIN_BL, 1);

    for (int y = 0; y < SCREEN_HEIGHT; ++y) {
        for (int x = 0; x < SCREEN_WIDTH; ++x) {
            _framebuffer[x][y] = 0;
        }
    }
}

void gfx::draw_pixel(int x, int y, colors color) {
    transform(x, y);
    _framebuffer[x][y] = color;
}

void gfx::show() {
    st7789_start_pixels(_pio, _sm);
    for (int y = 0; y < SCREEN_HEIGHT; ++y) {
        for (int x = 0; x < SCREEN_WIDTH; ++x) {
            st7789_lcd_put(_pio, _sm, _framebuffer[x][y] >> 8);
            st7789_lcd_put(_pio, _sm, _framebuffer[x][y] & 0xff);
        }
    }
}

void gfx::clear() {
    for (int y = 0; y < SCREEN_HEIGHT; ++y) {
        for (int x = 0; x < SCREEN_WIDTH; ++x) {
            _framebuffer[x][y] = 0;
        }
    }
}

void gfx::draw_char(int x, int y, char chr, colors color) {
	if(chr > 0x7E) return; // chr > '~'
	for(uint8_t i=0; i < this->_font[1]; i++ ) {
        uint8_t line = (uint8_t)(this->_font)[(chr-0x20) * (this->_font)[1] + i + 2];
        for(int8_t j=0; j<this->_font[0]; j++, line >>= 1) {
            if(line & 1) {
            	this->draw_pixel(x+i, y+j, color);
            }
        }
    }
}

void gfx::draw_string(int x, int y, std::string str, colors color){
	int x_tmp = x;
	while(str.length()) {
		this->draw_char(x_tmp, y, str.front(), color);
		x_tmp += ((uint8_t)_font[1]) + 1;
		str.erase(str.begin());
	}
}

void gfx::draw_rectangle(int x, int y, uint16_t w, uint16_t h, colors color) {
    this->draw_horizontal_line(x, y, w, color);
    this->draw_horizontal_line(x, y+h-1, w, color);
    this->draw_vertical_line(x, y, h, color);
    this->draw_vertical_line(x+w-1, y, h, color);
}

void gfx::draw_fill_rectangle(int x, int y, uint16_t w, uint16_t h, colors color) {
    for (int i=x; i<x+w; i++) {
    	this->draw_vertical_line(i, y, h, color);
    }
}

void gfx::draw_progress_bar(int x, int y, uint16_t w, uint16_t h, uint8_t progress, colors color) {
    this->draw_rectangle(x, y, w, h, color);
    this->draw_fill_rectangle(x, y, (uint8_t)((w*progress)/100), h, color);
}

void gfx::draw_vertical_line(int x, int y, int h, colors color) {
	this->draw_line(x, y, x, y+h-1, color);
}

void gfx::draw_horizontal_line(int x, int y, int w, colors color) {
	this->draw_line(x, y, x+w-1, y, color);
}

void gfx::draw_line(int x_start, int y_start, int x_end, int y_end, colors color) {
	int16_t steep = abs(y_end - y_start) > abs(x_end - x_start);
	if (steep) {
		this->swap(x_start, y_start);
		this->swap(x_end, y_end);
	}

	if (x_start > x_end) {
		this->swap(x_start, x_end);
		this->swap(y_start, y_end);
	}

	int16_t dx = x_end - x_start;
	int16_t dy = abs(y_end - y_start);

	int16_t err = dx / 2;
	int16_t ystep;

	if (y_start < y_end) ystep = 1;
	else ystep = -1;

	while(x_start <= x_end) {
		if (steep) this->draw_pixel(y_start, x_start, color);
		else this->draw_pixel(x_start, y_start, color);
		x_start++;
		err -= dy;
		if (err < 0) {
			y_start += ystep;
			err += dx;
		}
	}
}

inline void gfx::swap(int &a, int &b) {
    int tmp = a;
    a = b;
    b = tmp;
}

inline void gfx::mirror_x(int &x) {
    x = SCREEN_WIDTH - x;
}

inline void gfx::mirror_y(int &y) {
    y = SCREEN_WIDTH - y;
}

inline void gfx::transform(int &x, int &y) {
    if (_upside_down) {
        if (_swap_xy) swap(x, y);
        if (_mirror_x) mirror_x(x);
        if (_mirror_y) mirror_x(y);
    } else {
        if (_mirror_x) mirror_x(x);
        if (_mirror_y) mirror_x(y);
        if (_swap_xy) swap(x, y);
    }
}