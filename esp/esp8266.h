#ifndef ESP8266_H
#define ESP8266_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    const char * text;
    uint16_t timeout; // - 1ms each
} esp_command_t;

typedef void (*reset_watchdog_t)(void);

typedef struct
{
    SerialDriver *sd;
    reset_watchdog_t wdt;
    const char * essid;
    const char * password;
    uint16_t autoReconnectTimeout;     //automatic reconnect timeout when no data arrives [seconds]
} esp_config_t;



void esp_set_sd(const esp_config_t * config);
uint16_t esp_decode_ipd(char * buf, uint8_t * id);
void esp_write_tcp_char(char c, uint8_t tcp_id);
void esp_write_tcp(const char * buf, uint8_t size, uint8_t tcp_id);
uint8_t esp_run_sequence(const esp_command_t * command_list, uint8_t count);
uint8_t esp_run_command(const char * text, uint16_t timeout, char * response, uint16_t buffer_size);
int16_t esp_signal_strength(void);
int16_t esp_ping(const char * address);


uint8_t esp_connect_to_wifi(const char * essid, const char * password);
uint8_t esp_keep_connected_loop(uint16_t open_port, uint8_t force_reconnect);

//helper functions
uint16_t atoiw(const char * in);
char atoi(const char * in);
char atoi10(const char * in);
const char * contains(const char * string, const char * substring);
void itoa(char * out, uint32_t in, uint8_t bytes, uint8_t radix);

//basic commands decoding
// 'r'; read memory word ; 8bytes coded address
// 's'; signal strength ; no args
// 'w'; write memory byte ; 8bytes coded address ; 2bytes code data
// "boot" ; jump to bootloader program

//responses
//uint8_t 85: memory back
//uint8_t 86: memory written
void esp_basic_commands(const char * raw, uint8_t len, uint8_t tcp_id);

//simple modbus
// 'g'; get data ; 2 bytes coded index of array
// 'p'; put data ; 2 bytes coded index of array ; 4 bytes coded data

// response
// 'G' 2bytes coded index ;4 bytes coded data
// 'P' 2bytes code index
void esp_simple_modbus(const char * raw, uint8_t len, uint8_t tcp_id, uint16_t * array, uint8_t array_size);

#ifdef __cplusplus
}
#endif


#endif // ESP8266_H
