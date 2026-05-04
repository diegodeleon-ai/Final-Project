#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "led_strip.h"
#include "esp_log.h"

// =========================
//   Hardware definitions
// =========================

#define LED_GPIO   GPIO_NUM_8      // ESP32-C3 addressable LED, DO NOT TOUCh
#define UART_PORT  UART_NUM_0
#define UART_BAUD  115200
#define UART_RX_BUF_SIZE 1024


typedef enum {
    LED_MODE_OFF = 0,
    LED_MODE_ON,
    LED_MODE_BLINK
} led_mode_t;

typedef struct {
    led_mode_t mode;       // OFF / ON / BLINK
    int        blink_ms;   // blink period in milliseconds
    int        led_level;  // to keep track of the state
} cli_state_t;

static led_strip_handle_t led_strip;

// Already implemented, DO NOT TOUCH

static void led_init(void)
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_GPIO,
        .max_leds = 1,
    };
    
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    led_strip_clear(led_strip);  // Turn off at startup
}

/*
 * Set LED to on/off.
 * M1: TODO
 */
static void led_set_raw(int on)
{
	// This method needs to check if the led is on every time it is called
	// (i.e., make a comparison) and then turn on/off depending on its state.

	// Methods you need to use: led_strip_set_pixel(), led_strip_refresh and led_strip_clear().
	// Variables:  a refrence to the led strip (i.e., the led) and the pointer to the led on/off step.
	if (on) {
        led_strip_set_pixel(led_strip, 0, 255, 255, 255);
        led_strip_refresh(led_strip);
    } else {
        led_strip_clear(led_strip);
    }

}


/*
 * Initialize UART with the specified configuration.
 * This function is already implemented for you.
 */
static void uart_init(void)
{
    const uart_config_t cfg = {
        .baud_rate = UART_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    uart_param_config(UART_PORT, &cfg);
    uart_set_pin(UART_PORT,
                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE,
                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_PORT, UART_RX_BUF_SIZE, 0, 0, NULL, 0);
}

/*
 * M2: Read a line from UART (blocking), storing at most len-1 chars.
 * The line should be terminated by '\n' and the buffer by '\0'.
 *
 * This function is already implemented for you.
 */

static int read_line(char *buf, int len)
{

/*


M2: Convert this pseudocode to C:

    position = 0

    WHILE position < max_length - 1 DO
		NOTE: read_one_byte_from_uart is a esp driver, it goes exactly in that place

        character = read_one_byte_from_uart(wait_indefinitely)
    
        IF nothing_was_read THEN
            continue_to_next_iteration
        END IF
    
        // Echo back the character
        write_character_to_uart(character)
    
        IF character == '\r' OR character == '\n' THEN
            break_from_loop
        END IF
    
        buffer[position] = character
        position = position + 1
    END WHILE

buffer[position] = '\0'  // Null terminate the string
RETURN position
*/
}

// =========================
//   CLI logic (M3/M4)
// =========================

/*
 * M3: TODO - Print the help text showing all available commands.
 * 
 * Commands to document:
 *   - help: show the help menu
 *   - led on: turn LED on
 *   - led off: turn LED off
 *   - blink <ms>: blink LED with period <ms>
 *   - status: show current LED state
 *   - about: show information about your team/project
 * 
 */
static void cli_print_help(void)
{
    // TODO: Implement this function
    printf("TODO: Print help menu here\n");
}

/*
 * M3/M4: TODO - Parse a command line and update the CLI state accordingly.
 *
 * This is the main command parser. You should implement:
 *
 *   - "help": call cli_print_help()
 *   - "led on": set st->mode to LED_MODE_ON
 *   - "led off": set st->mode to LED_MODE_OFF
 *   - "status": print current LED mode (OFF/ON/BLINK) and blink period if applicable
 *   - "about": print your name and student ID
 *
 * M4 - Commands with parameters:
 *    - "blink <ms>": parse the number after "blink", validate it (50-5000 ms),
 *     and set st->mode to LED_MODE_BLINK with st->blink_ms = parsed value
 *
 */

static void cli_handle_line(cli_state_t *st, const char *line)
{
    // Trim leading spaces (already done for you)
    while (*line == ' ') {
        line++;
    }

    // Handle empty input
    if (*line == '\0') {
        printf("(empty command)\n");
        return;
    }

    else if (strcmp(line, "led on") == 0) {
		st->mode = LED_MODE_ON;
		st->led_level = 1;
		printf("LED turned on\n");
		return;
	}
    
    else if (strcmp(line, "led off") == 0) {
		st->mode = LED_MODE_OFF;
		st->led_level = 0;
		printf("LED turned off\n");
		return;
	}
    
    // If no command matched, print error message
    printf("Unknown command: '%s'\n", line);
    printf("Type 'help' for a list of commands.\n");
}

// =========================
//   Main loop 
// =========================

void app_main(void)
{
    // Initialize hardware (already done for you)
    led_init();
    uart_init();

    // Initialize CLI state
    cli_state_t st = {
        .mode = LED_MODE_OFF,
        .blink_ms = 500,
        .led_level = 0,
    };

    led_set_raw(0);  // Start with LED off

    printf("\n=================================\n");
    printf("ESP32-C3 UART CLI ready!\n");
    printf("Type 'help' to see available commands.\n");
    printf("=================================\n\n");

    char line[64];
    TickType_t last_blink = xTaskGetTickCount();

    while (1) {
        // M2/M3: Get a line from UART
        printf("> ");
        fflush(stdout);

        int n = read_line(line, sizeof(line));
        printf("\n");
        
        if (n > 0) {
            cli_handle_line(&st, line);
        }

        // M4: Handle LED blinking (already implemented for you)
        // This code runs continuously to update the LED state
        if (st.mode == LED_MODE_BLINK) {
            TickType_t now = xTaskGetTickCount();
            if ((now - last_blink) >= pdMS_TO_TICKS(st.blink_ms)) {
                st.led_level = !st.led_level;
                led_set_raw(st.led_level);
                last_blink = now;
            }
        } else if (st.mode == LED_MODE_ON) {
            st.led_level = 1;
            led_set_raw(1);
        } else {
            st.led_level = 0;
            led_set_raw(0);
        }
    }
}
