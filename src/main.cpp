#include <stdio.h>
#include "pico/stdlib.h"
#include <pico/platform.h>

#include "hardware/i2c.h"
#include "mcp23017.h"

#define I2C_GPIO_PIN_SDA 20
#define I2C_GPIO_PIN_SLC 21

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

static volatile uint8_t cur_row = 0;
static struct repeating_timer timer;

void setup_output(Mcp23017 mcp) {
	int result;

	result = mcp.setup(true, false);
	result = mcp.set_io_direction(0x0000); // Set all pins to output
}

bool timer_isr(struct repeating_timer *t) {
	// Turn off the columns (leave the row selected, otherwise we risk shortly blinking the wrong led!!!)
	mcp1.set_all_output_bits((row_address_translation[cur_row & 0x7]) << 5);
	mcp1.flush_output();

	// Increment the row
	cur_row += 1;
	uint8_t cur_col = 1 << (cur_row & 0x7);

	// Turn on the column leds on the new row
	mcp1.set_all_output_bits((((uint16_t)cur_col) << 8) | ((row_address_translation[cur_row & 0x7]) << 5));
	mcp1.flush_output();

    return true;
}

int main() {
	stdio_init_all();
	busy_wait_ms(2000);
	printf("Starting up\n");

	i2c_init(i2c0, 400000);
	gpio_set_function(I2C_GPIO_PIN_SDA, GPIO_FUNC_I2C);
	gpio_set_function(I2C_GPIO_PIN_SLC, GPIO_FUNC_I2C);
	//gpio_pull_up(I2C_GPIO_PIN_SDA);
	//gpio_pull_up(I2C_GPIO_PIN_SLC);

	setup_output(mcp1);
	mcp1.set_all_output_bits(0x0000); // Set all pins to off
	mcp1.flush_output();

	cur_row = 0;

	add_repeating_timer_us(-1000, timer_isr, NULL, &timer);

	while(true);

    return 0;
}