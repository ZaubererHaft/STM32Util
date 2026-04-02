#include "dht.h"
#include "dwt_delay.h"
#include "main.h"

#include <string.h>

#define INIT_MAX_US_LOW  100
#define INIT_MAX_US_HIGH  100
#define MAX_US_WAIT_BIT  100
#define HIGH_BIT_MIN_US  60
#define HIGH_BIT_MAX_US  95
#define BITS_TO_RECEIVE  40
#define TIMEOUT_LAST_INTERRUPT  3000

void DHT_Init(DHT *dht, const DHTInitParams init_params) {
    dht->type = init_params.type;
    dht->state = RESTART;
    dht->bits_received = 0;
    dht->cycles = 0;
    dht->ticks_last_active_irq = 0;
    dht->gpio_function = init_params.gpio_function;
    dht->wait_function_ms = init_params.wait_function;
    dht->callback = init_params.callback;
    dht->restart_wait_time_ms = init_params.restart_wait_time_ms;
    dht->sysclock_mhz = init_params.sysclock_mhz;
}


void DHT_InterruptReceived(DHT *dht) {
    const uint32_t curr_cycles = DWT_GetTick();
    const uint32_t diff_time = (curr_cycles - dht->cycles) / dht->sysclock_mhz;
    DHT_STATE new_state = dht->state;

    if (dht->state == COMM_INITIALIZED) {
        new_state = COMM_INITIALIZED_WAIT;
    } else if (dht->state == COMM_INITIALIZED_WAIT) {
        new_state = COMM_INITIALIZED_WAIT_FOR_DEVICE;
    } else if (dht->state == COMM_INITIALIZED_WAIT_FOR_DEVICE) {
        new_state = COMM_INITIALIZED_WAIT_FOR_DEVICE_LOW;
    } else if (dht->state == COMM_INITIALIZED_WAIT_FOR_DEVICE_LOW) {
        if (diff_time <= INIT_MAX_US_LOW) {
            new_state = COMM_INITIALIZED_WAIT_FOR_DEVICE_HIGH;
        } else {
            new_state = COMM_ERROR;
        }
    } else if (dht->state == COMM_INITIALIZED_WAIT_FOR_DEVICE_HIGH) {
        if (diff_time <= INIT_MAX_US_HIGH) {
            new_state = WAIT_FOR_BIT;
        } else {
            new_state = COMM_ERROR;
        }
    } else if (dht->state == WAIT_FOR_BIT) {
        if (diff_time <= MAX_US_WAIT_BIT) {
            new_state = READY_FOR_BIT;
        } else {
            new_state = COMM_ERROR;
        }
    } else if (dht->state == READY_FOR_BIT) {
        uint32_t index = dht->bits_received / 8;
        uint32_t shift = dht->bits_received % 8;

        if (diff_time > HIGH_BIT_MIN_US && diff_time <= HIGH_BIT_MAX_US) {
            dht->data_received[index] |= (1 << (7 - shift));
        }

        new_state = WAIT_FOR_BIT;
        dht->bits_received++;

        if (dht->bits_received >= BITS_TO_RECEIVE) {
            new_state = COMM_END;
        }
    } else if (dht->state == COMM_END) {
        new_state = PROCESS_DATA;
    }

    if (new_state != dht->state) {
        dht->ticks_last_active_irq = HAL_GetTick();
        dht->state = new_state;
    }
    dht->cycles = curr_cycles;
}

void idle_state(DHT *dht) {
    dht->state = COMM_INITIALIZED;
    dht->gpio_function(GPIO_PIN_RESET);
    dht->wait_function_ms(18);
    dht->gpio_function(GPIO_PIN_SET);
}

void restart_state(DHT *dht) {
    dht->gpio_function(GPIO_PIN_SET);
    memset(&dht->data_received[0], 0, 5);
    dht->bits_received = 0;
    dht->wait_function_ms(dht->restart_wait_time_ms);
    dht->state = IDLE;
}

void comm_error_state(DHT *dht) {
    dht->state = RESTART;
}

void process_data_state(DHT *dht) {
    uint32_t hum1 = dht->data_received[0];
    uint32_t hum2 = dht->data_received[1];
    uint32_t temp1 = dht->data_received[2];
    uint32_t temp2 = dht->data_received[3];
    uint32_t check = dht->data_received[4];

    uint32_t crc_expected = (hum1 + hum2 + temp1 + temp2) % 256;

    if (check == crc_expected) {
        float temp = (float) temp1;
        float hum = (float) hum1;

        if (dht->type == DHT22) {
            temp = (float) (temp1 << 8 | temp2) / 10.0f;
            hum = (float) (hum1 << 8 | hum2) / 10.0f;
        }

        dht->callback(temp, hum);
        dht->state = RESTART;
    } else {
        dht->state = COMM_ERROR;
    }
}


void DHT_Main(DHT *dht) {
    // check interrupt for timeout (if there was none since a defined time -> try again but display error)
    uint32_t tick = HAL_GetTick();
    if (dht->state != RESTART && dht->ticks_last_active_irq < tick &&
        tick - dht->ticks_last_active_irq > TIMEOUT_LAST_INTERRUPT) {
        dht->state = COMM_ERROR;
        dht->ticks_last_active_irq = tick;
    }
    if (dht->state == RESTART) {
        restart_state(dht);
    }
    if (dht->state == IDLE) {
        idle_state(dht);
    }
    if (dht->state == PROCESS_DATA) {
        process_data_state(dht);
    }
    if (dht->state == COMM_ERROR) {
        comm_error_state(dht);
    }
}
