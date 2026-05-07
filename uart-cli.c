#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "led_strip.h"
#include "esp_log.h"
#include <stdlib.h> //needed due to atoi() - Sofia
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
 Part 1: Diego
 */
static void led_set_raw(int on)
{
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
 * YOUR PART — COMPLETED (John)
 */
static int read_line(char *buf, int len)
{
    int pos = 0;
    uint8_t ch;

    while (pos < len - 1) {

        // Read ONE byte from UART, block forever
        int n = uart_read_bytes(UART_PORT, &ch, 1, portMAX_DELAY);

        // If nothing was read, continue
        if (n <= 0) {
            continue;
        }

        // Echo the character back to the terminal
        uart_write_bytes(UART_PORT, (const char *)&ch, 1);

        // Stop on newline or carriage return
        if (ch == '\n' || ch == '\r') {
            break;
        }

        // Store the character
        buf[pos++] = ch;
    }

    // Null terminate the string
    buf[pos] = '\0';

    return pos;
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
  /* Part 3 Complete */
    printf("Available commands:\n");
    printf("  help         - Show this help menu\n");
    printf("  led on       - Turn the LED on\n");
    printf("  led off      - Turn the LED off\n");
    printf("  blink <ms>   - Blink LED with period in ms (50-5000)\n");
    printf("  status       - Show current LED state\n");
    printf("  about        - Show project/team info\n");
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
    /* part 3  complete */
    if (strcmp(line, "help") == 0) {
        cli_print_help();
        return;
    }

    if (strcmp(line, "status") == 0) {
        if (st->mode == LED_MODE_OFF) {
            printf("LED is OFF\n");
        } else if (st->mode == LED_MODE_ON) {
            printf("LED is ON\n");
        } else if (st->mode == LED_MODE_BLINK) {
            printf("LED is BLINKING at %d ms\n", st->blink_ms);
        }
        return;
    }

    if (strcmp(line, "about") == 0) {
        printf("UART-CLI Final Project\n");
        printf("Team: Diego de Leon, Arturo Valles, Brandon Hernandez, Christian Pintor, John Zermeno, Sofia Garcia, Ziggy Zimmerman \n");
        printf("UTEP - Computer Organization 2026\n");
        return;
    }

    /* Ziggy Z - Part 6 Complete */

    if(strncmp(line, "blink ", 6) == 0){
        int ms;
        ms = atoi(line + 6);

        if(ms < 50 || ms > 5000){
            printf("Please use a number from 50 to 5000\n");
            return;
        }

        st->mode = LED_MODE_BLINK;
        st->blink_ms = ms;
        printf("Blink is set to %d ms\n", ms);
        
        return;
    }



 /*  part 5 complete */ 
		
    if (strcmp(line, "led on") == 0) {
        st->mode = LED_MODE_ON;
        st->led_level = 1;
        printf("LED turned on\n");
        return;
    }
    
    if (strcmp(line, "led off") == 0) {
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
