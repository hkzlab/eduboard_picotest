#include <stdio.h>
#include "pico/stdlib.h"

#include "hardware/i2c.h"
#include "mcp23017.h"

static const int MCP_ALL_PINS_OFF = 0x0000;

#define I2C_GPIO_PIN_SDA 20
#define I2C_GPIO_PIN_SLC 21

Mcp23017 mcp1(i2c0, 0x20); // MCP with A0, A1 and A2 to GND

void setup_output(Mcp23017 mcp) {
	int result;

	result = mcp.setup(true, false);
	result = mcp.set_io_direction(0x0000); // Set all pins to output
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
	mcp1.set_all_output_bits(MCP_ALL_PINS_OFF);

	printf("Setting MCP(0x21) pin 4\n");
	mcp1.set_output_bit_for_pin(8, true);
	mcp1.flush_output();

    return 0;
}