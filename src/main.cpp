#include <stdio.h>
#include <string.h>

#include <pico/stdlib.h>
#include <pico/platform.h>
#include <pico/rand.h>

#include "hardware/i2c.h"
#include "mcp23017.h"

#define I2C_GPIO_PIN_SDA 20
#define I2C_GPIO_PIN_SLC 21

// Given that A0 and A2 are swapped when connected between the MCP expander and the '238 multiplexer.
// we precalculate the correct bit pattern we want to set on the MCP depending on the row we wish to light up.
const uint8_t __in_flash() row_address_translation[8] = { 
	0x00,
	0x04,
	0x02,
	0x06,
	0x01,
	0x05,
	0x03,
	0x07
 };

Mcp23017 mcp1(i2c0, 0x20); // MCP with A0, A1 and A2 to GND

static volatile uint8_t cur_row = 0; // Current row that will be drawn. Note that this will always increase, we are just using the first 3 bit by masking the rest
static volatile bool invert = false; // If true, the display on the eduboard will be inverted
static struct repeating_timer led_update_timer, snow_update_timer;

// This is our buffer that we'll draw on screen
static volatile uint8_t snow[8] = {
	0, 0, 0, 0,
	0, 0, 0, 0
};

void setup_output(Mcp23017 mcp) {
	int result;

	result = mcp.setup(true, false);
	result = mcp.set_io_direction(0x0000); // Set all pins to output
}

// This takes care of continuously drawing the image on the lines of the matrix
bool led_update_timer_isr(struct repeating_timer *t) {
	// Turn off the columns (leave the row selected, otherwise we risk shortly blinking the wrong led!!!)
	mcp1.set_all_output_bits((row_address_translation[cur_row & 0x7]) << 5);
	mcp1.flush_output();

	// Increment the row
	cur_row += 1;

	// Turn on the column leds on the new row
	uint8_t col_data = invert ? ~snow[cur_row & 0x07] : snow[cur_row & 0x07];
	mcp1.set_all_output_bits((((uint16_t)col_data) << 8) | ((row_address_translation[cur_row & 0x07]) << 5));
	mcp1.flush_output();

    return true;
}

bool check_full_matrix() {
	for(uint8_t idx = 0; idx < sizeof(snow); idx++) {
		if(snow[idx] != 0xFF) return false;
	}

	return true;
}

// This will take care of updating the "screen buffer", or the snow array
bool snow_update_timer_isr(struct repeating_timer *t) {
	const uint8_t snow_len = sizeof(snow);
	uint8_t snow_diff;

	if(check_full_matrix()) {
		memset((void*)snow, 0, sizeof(snow));
		invert = !invert;
	}

	// Start from bottom, go upward, skip the first line
	for(uint8_t idx = sizeof(snow) - 1; idx >= 1; idx--) {
		snow_diff = (snow[idx] ^ snow[idx - 1]) & snow[idx - 1]; // Find which flakes move from above row to current row
		snow[idx] |= snow_diff;
		snow[idx - 1] &= ~snow_diff; // Clear the flakes on the row above that moved to the current row
	}

	// Then generate new snowflakes 
	uint32_t rand = get_rand_32();
	snow[0] |= (uint8_t)(rand & (rand >> 8) & (rand >> 16));
	
	return true;
}

int main() {
	stdio_init_all();
	busy_wait_ms(2000);
	printf("Starting up\n");

	i2c_init(i2c0, 400000);
	gpio_set_function(I2C_GPIO_PIN_SDA, GPIO_FUNC_I2C);
	gpio_set_function(I2C_GPIO_PIN_SLC, GPIO_FUNC_I2C);
	//gpio_pull_up(I2C_GPIO_PIN_SDA); // Enable these if the eduboard doesn't have the I2C pullups installed
	//gpio_pull_up(I2C_GPIO_PIN_SLC);

	setup_output(mcp1);
	mcp1.set_all_output_bits(0x0000); // Set all pins to off
	mcp1.flush_output();

	cur_row = 0;

	add_repeating_timer_us(-500, led_update_timer_isr, NULL, &led_update_timer);
	add_repeating_timer_us(-40000, snow_update_timer_isr, NULL, &snow_update_timer);

	while(true);

	return 0;
}