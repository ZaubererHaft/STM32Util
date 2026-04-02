#ifndef HYGRO_DHT_H
#define HYGRO_DHT_H

#include <stdint.h>

typedef enum DHTState_ {
    IDLE,
    COMM_INITIALIZED,
    READY_FOR_BIT,
    COMM_INITIALIZED_WAIT,
    COMM_INITIALIZED_WAIT_FOR_DEVICE,
    COMM_INITIALIZED_WAIT_FOR_DEVICE_HIGH,
    COMM_INITIALIZED_WAIT_FOR_DEVICE_LOW,
    COMM_ERROR,
    WAIT_FOR_BIT,
    READ_BIT,
    PROCESS_DATA,
    RESTART,
    COMM_END
} DHT_STATE;

typedef void (*GPIO_Function)(uint32_t);

typedef void (*Callback_Function)(float, float);

typedef enum DHTType_ {
    DHT11, DHT22
} DHTType;

typedef struct DHT_ {
    uint8_t data_received[5];
    DHT_STATE state;
    uint32_t bits_received;
    uint32_t cycles;
    uint32_t ticks_last_active_irq;
    GPIO_Function gpio_function;
    GPIO_Function wait_function_ms;
    Callback_Function callback;
    DHTType type;
    uint32_t restart_wait_time_ms;
    uint32_t sysclock_mhz;
} DHT;

typedef struct DHTInitParams_ {
    GPIO_Function function;
    GPIO_Function wait_function;
    Callback_Function callback;
    DHTType type;
    uint32_t restart_wait_time_ms;
    uint32_t sysclock_mhz;
} DHTInitParams;

void DHT_Init(DHT *dht, DHTInitParams init_params);

void DHT_Main(DHT *dht);

void DHT_InterruptReceived(DHT *dht);

#endif //HYGRO_DHT_H