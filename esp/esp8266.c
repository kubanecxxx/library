#include "hal.h"
#include "ch.h"
#include "esp8266.h"
#include "chprintf.h"
#include "string.h"

#ifdef BOOTLOADER_USER_PROGRAM
#include "boot.h"
#endif

static SerialDriver * uart = NULL;
static BaseSequentialStream * stream = NULL;
uint8_t wait_for_data_response(uint16_t to, char * buf, uint16_t buf_size, reset_watchdog_t cb);
uint8_t wait_for_data(uint16_t to,  reset_watchdog_t cb);
static systime_t last_data_rec;

static thread_t * sending_thread;

static const esp_config_t * cfg;
static int16_t signal_strength;

static THD_WORKING_AREA(esp_thread, 200);
static THD_FUNCTION(esp_rec_thread, arg);

#define SIGNAL_OK 1
#define SIGNAL_ERROR 2
#define SIGNAL_CONNECT 4
#define SIGNAL_CLOSED 8
#define EVENT_IPD 1
#define EVENT_AT 2

#ifndef ESP_IPD_BUFFER_SIZE
    #define ESP_IPD_BUFFER_SIZE 256
#endif

#ifndef ESP_AT_BUFFER_SIZE
    #define ESP_AT_BUFFER_SIZE 256
#endif


void esp_basic_commands(const char * raw, uint8_t len, uint8_t id)
{
    char buf[10];
    const char * p;

    if (raw[0] == 'r' )
    {
        if (strlen(raw) < 9)
            return;

        uint32_t a;
        p = raw + 1;
        a = atoiw(p) << 16;
        p+= 4;
        a |= atoiw(p);

        uint32_t v = *(uint32_t *)a;
        itoa(buf+1,v,4,16);
        buf[0] = 0x85;
        esp_write_tcp(buf,strlen(buf),id);
    }
    else if (raw[0] == 'w')
    {
        if (strlen(raw) < 11)
            return;

        uint32_t a;
        p = raw + 1;
        a = atoiw(p) << 16;
        p+= 4;
        a |= atoiw(p);
        p+= 4;

        if (a <= SRAM_BASE)
            return;

        uint8_t d = atoi(p);
        *(uint8_t *) a = d;

        esp_write_tcp_char(0x86, id);
    }
    else if (raw[0] == 's')
    {
        uint16_t s;
        s = signal_strength;
        if (signal_strength < 0)
        {
            s = -signal_strength;
        }
        buf[0] = '-';
        itoa(buf+1,s,2,10);
        esp_write_tcp(buf,strlen(buf),id);
    }
#ifdef BOOTLOADER_USER_PROGRAM
    else if (contains(raw,"boot"))
    {
        bootJumpToBootloader();
    }
#endif
}


void itoa(char *out, uint32_t in, uint8_t bytes, uint8_t radix)
{
    int8_t i;
    uint32_t n;
    char c;
    for (i = 2 * bytes - 1; i>= 0 ; i--)
    {
        n = in % radix;
        in /= radix;

        if ( n <= 9)
        {
            c = n + '0' ;
        }
        else if (n > 9)
        {
            c = n - 10 + 'A';
        }
        out[i] = c;
    }
    out[2 * bytes] = 0;
}

uint32_t atoi10(const char * in, uint8_t bytes)
{
    char z;
    uint8_t i;
    uint8_t b = 0;
    uint32_t ret = 0;
    for (i = 0 ; i < bytes ; i++)
    {
        z = *in;
        in++;
        if (z >= '0' && z <= '9')
        {
            b = z - '0';
        }

        ret = (ret * 10) + b;
    }

    return ret;
}

char atoi(const char * in)
{
    char z;
    uint8_t b,i;
    char ret = 0;
    for (i = 0 ; i < 2 ; i++)
    {
        z = *in;
        in++;
        if (z >= '0' && z <= '9')
        {
            b = z - '0';
        }
        else if (z >= 'A' && z <= 'F')
        {
            b = z - 'A' + 10;
        }
        ret = (ret << 4) + b;
    }

    return ret;

}

uint16_t atoiw(const char * in)
{
    uint16_t r;
    r = atoi(in+2);
    r |= atoi(in) << 8;

    return r;
}

/**
 * @brief esp_set_sd initialize esp driver, must be called from the thread which will call
 * esp methods - because of direct events
 * @param config
 */
void esp_set_sd(const esp_config_t *config)
{
    cfg = config;
    uart = cfg->sd;
    stream = (BaseSequentialStream*)uart;

    chThdCreateStatic(&esp_thread,sizeof(esp_thread),HIGHPRIO-1,esp_rec_thread,NULL);
    sending_thread = chThdGetSelfX();
}

int16_t esp_signal_strength()
{
    int16_t stren = 0;

    char buffer[128];
    char * p ;

    if (esp_run_command("AT+CWJAP?", 100,buffer,sizeof(buffer)))
    {
        p = strrchr(buffer,'-');
        if (p)
        {
            p++;
            while (*p <= '9' && *p >= '0')
            {
                stren = 10 * stren + *p - '0';
                p++;
            }
            signal_strength = -stren;
            return -stren;
        }
    }

    signal_strength = 0xff;
    return 0xff;
}

uint8_t esp_connect_to_wifi(const char *essid, const char *password)
{
    char buffer[128];
    uint8_t ok;

    /*
    ok = esp_run_command("AT+RST", 100,buffer);
    int16_t z = sdGetTimeout(uart,MS2ST(1));
    while (z != -1)
    {
        z = sdGetTimeout(uart,MS2ST(1));
    }
    */

    ok = esp_run_command("AT", 20, buffer,sizeof(buffer));
    if (!ok)
        return 0;

    ok = esp_run_command("AT+CWMODE=1", 500, buffer, sizeof(buffer));
    if (!ok)
        return 0;

    chprintf(stream,"AT+CWJAP=\"%s\",\"%s\"\r\n",essid,password);
    ok = wait_for_data_response(20000, buffer, sizeof(buffer), cfg->wdt);

    return ok;
}

int16_t esp_ping(const char * address)
{
    char buffer[128];
    char * p;
    int16_t ping = 0;

    chprintf(stream,"AT+PING=\"%s\"\r\n",address);
    uint8_t ok = wait_for_data_response(3000,buffer, sizeof(buffer), cfg->wdt);

    p = strchr(buffer, '+');
    p = strchr(p, '+');
    if (!p)
        return -1;
    p++;

    p = strchr(p, '+');
    if (!p)
        return -1;
    p++;

    while(*p >= '0' && *p <= '9')
    {
        ping = 10 * ping + *p - '0';
        p++;
    }

    return ping;
}

uint8_t esp_keep_connected_loop(uint16_t open_port, uint8_t force_reconnect)
{
    static uint8_t ok = 0;
    uint8_t reconnected = 0;
    static systime_t meas = 0;
    static uint16_t old_port;
    int16_t ping;

    char buffer[128];

    //first run or every 5 seconds
    systime_t cur = chVTGetSystemTime();

    if (!ok || (cur - meas > MS2ST(5000)))
    {
        if (cfg->autoReconnectTimeout && (cur - last_data_rec > S2ST(cfg->autoReconnectTimeout)))
        {
            force_reconnect = 1;
            last_data_rec = cur;
        }

        meas = cur;
        //if already connected dont reconnect
        ping = esp_signal_strength() == 0xff ? -1 : 2;

        //not connected to wifi - reconnect
        if (ping == -1 || force_reconnect)
        {
            ok = esp_run_command("AT+CIPSERVER=0",500,NULL,sizeof(buffer));
            //in this function automatic watchdog reset will be called
            reconnected = esp_connect_to_wifi(cfg->essid, cfg->password);
            if (!reconnected)
            {
                ok = 0;
                return 0;
            }
        }

        //esp_run_command("AT+CIFSR",1000,buffer);
        //esp_run_command("AT+CIPSTATUS",500,buffer);

        //if was disconnected and now connected or first run
        //reset all connections and setup new tcp server
        if (reconnected || !ok || old_port != open_port)
        {
            old_port = open_port;
            //already connected
            //if connection doesnt exists it will be error
            ok = esp_run_command("AT+CIPCLOSE",500,NULL,sizeof(buffer));
            ok = esp_run_command("AT+CIPMUX=1",500,NULL,sizeof(buffer));
            ok = esp_run_command("AT+CIPCLOSE=0",500,NULL,sizeof(buffer));
            ok = esp_run_command("AT+CIPCLOSE=1",500,NULL,sizeof(buffer));
            ok = esp_run_command("AT+CIPCLOSE=2",500,NULL,sizeof(buffer));
            ok = esp_run_command("AT+CIPCLOSE=3",500,NULL,sizeof(buffer));
            ok = esp_run_command("AT+CIPCLOSE=4",500,NULL,sizeof(buffer));
            ok = esp_run_command("AT+CIPSERVER=0",500,NULL,sizeof(buffer));

            if (open_port)
            {
                chprintf(stream,"AT+CIPSERVER=1,%d\r\n",open_port);
                ok = wait_for_data_response(500,buffer, sizeof(buffer), NULL);
            }
            else
            {
                ok = 1;
            }
        }
    }


    return ok;
}

uint8_t esp_run_command(const char * text, uint16_t timeout, char * response, uint16_t buffer_size)
{
    chprintf(stream,text);
    chprintf(stream,"\r\n");
    uint8_t ok = 0;
    ok = wait_for_data_response(timeout,response, buffer_size,NULL);
    return ok;
}

uint8_t esp_run_sequence(const esp_command_t *commands, uint8_t count)
{
    char buffer[128];
    const char * text;
    uint8_t status = 0;
    uint16_t to;
    uint8_t j;

    for (j = 0; j < count; j++)
    {
        to = commands[j].timeout;
        text = commands[j].text;
        status = esp_run_command(text,to, buffer,sizeof(buffer));

        if (!status)
            return 0;
    }
    return 1;
}

static ipd_data_t d;

static struct
{
    uint16_t packet_counter;
    uint16_t packet_counter_main;
    uint16_t failed_counter;
} debug;


THD_FUNCTION(esp_rec_thread, arg)
{
    (void) arg;
    static char ipd_data[ESP_IPD_BUFFER_SIZE];
    static char at_data[ESP_AT_BUFFER_SIZE];
    /*
    static  char debug_buf[256];
    static uint8_t debug_buf_idx;
    */

    int16_t c;
    uint16_t idx  = 0;
    systime_t last;
    char help[10];
    uint8_t help_idx = 0;
    uint8_t ipd = 0;
    enum {ID,LEN,DATA} ipd_machine;
    uint8_t id;
    uint16_t len;

    at_data_t at;

    at.data = at_data;
    d.data = ipd_data;


    chRegSetThreadName("esp_rec_thread");

    while (true)
    {
        last = chVTGetSystemTime();
        c = sdGet(uart);

        //debug_buf[debug_buf_idx++] = c;
        //determine start - long time no data received
        //reset pointers
        if (chVTGetSystemTime() - last > MS2ST(10))
        {
            idx = 0;
            help_idx = 0;
            ipd = 0;
        }

        //trimming
        if (idx == 0)
        {
            if (c == '\r' || c == '\n')
                continue;
        }

        //if starts with +IPD decode ipd data
        //if not its some AT packet
        //not start - data continues
        if (!ipd)
        {
            at_data[idx++] = c;
            at_data[idx] = 0;
            const char * p;

            if (contains(at_data,"OK"))
            {
                //call somewhere its OK
                at.status = SIGNAL_OK;
                chMsgSend(sending_thread, (msg_t)&at);
                idx = 0;
            }
            else if((p = contains(at_data,"CONNECT")))
            {
                //asynchronous command - someone has connected
                //to an open TCP server
                at.status = SIGNAL_CONNECT;
                p -= 2;
                id = *p - '0';  //just decode, never used
                idx = 0;        //this is important - someone will connect and
                //want to send data within reset timeout
            }
            else if((p = contains(at_data,"CLOSED")))
            {
                //asynchronous command - someone has connected
                //to an open TCP server
                at.status = SIGNAL_CLOSED;
                p -= 2;
                id = *p - '0';  //just decode, never used
                idx = 0;        //this is important - someone will connect and
                //want to send data within reset timeout
            }
            else if (contains(at_data, "ERROR") || contains(at_data, "FAIL"))
            {
                //call somewehre its NOT OK
                at.status = SIGNAL_ERROR;
                chMsgSend(sending_thread, (msg_t)&at);
                idx = 0;
            }
        }
        else
        {
            if (ipd_machine == ID)
            {
                idx++;
                if(c != ',')
                {
                    id = c - '0';
                    d.valid++;
                    d.id = id;
                }
                else
                {
                    ipd_machine = LEN;
                    help_idx = 0;
                }
            }
            else if (ipd_machine == LEN)
            {
                if (c == ':')
                {
                    len = atoi10(help, help_idx);
                    uint16_t l;
                    d.valid++;
                    l = sdReadTimeout(uart,ipd_data,len, MS2ST(300));
                    help_idx = 0;
                    idx = 0;
                    ipd = 0;
                    if (l == len)
                    {
                        debug.packet_counter++;
                        debug.failed_counter--;
                        ipd_data[l] = 0;
                        d.valid++;
                        d.length = l;
                        chEvtSignal(sending_thread, EVENT_IPD);
                    }

                }
                help[help_idx++] = c;
                help[help_idx] = 0;
            }
            else if (ipd_machine == DATA)
            {

            }
        }

        if (idx == 5)
        {
            if (!strcmp(at_data, "+IPD,"))
            {
                debug.failed_counter++;
                ipd = 1;
                ipd_machine = ID;
                d.valid++;
                //listen data - decode +IPD,ID,length:data structure
                //it is IPD packet
            }
        }
    }
}

uint16_t esp_decode_ipd(char * buf, uint16_t buf_size, uint8_t * iid)
{
    eventmask_t msg;
    uint16_t len;
    uint16_t old;

    msg = chEvtWaitOneTimeout(EVENT_IPD, TIME_IMMEDIATE);

    if (msg != EVENT_IPD)
        return 0;

    debug.packet_counter_main++;
    old = d.valid;
    *iid = d.id;
    len = d.length;
    uint16_t min = buf_size > ESP_IPD_BUFFER_SIZE ? ESP_IPD_BUFFER_SIZE : buf_size;
    min = min < len ? min : len;
    memcpy(buf, d.data, min);

    if (d.valid == old)
    {
        last_data_rec = chVTGetSystemTime();
        return len;
    }
    return 0;
}

uint8_t wait_for_data_response(uint16_t to, char * buf, uint16_t buf_size, reset_watchdog_t cb)
{
    chSysLock();
    uint8_t h = chMsgIsPendingI(chThdGetSelfX());
    chSysUnlock();
    uint16_t cnt = to / 10;
    thread_t * thd;
    msg_t msg;
    at_data_t * d;
    uint8_t status;
    uint16_t min;
    while(!h && cnt--)
    {
        chSysLock();
        h = chMsgIsPendingI(chThdGetSelfX());
        chSysUnlock();
        if (!h)
            chThdSleepMilliseconds(10);
        if (cb)
            cb();
    }

    if (h)
    {
        thd = chMsgWait();
        msg = chMsgGet(thd);
        d = (at_data_t *) msg;

        if (buf)
        {
            min = buf_size > ESP_AT_BUFFER_SIZE ? ESP_AT_BUFFER_SIZE : buf_size;
            if (strlen(d->data) < min)
                strcpy(buf,d->data);
            else
                memcpy(buf,d->data,min);
        }

        status = d->status == SIGNAL_OK;
        chMsgRelease(thd,0);
        return status;
    }
    return 0;
}

void esp_write_tcp_char(char c, uint8_t tcp_id)
{
    char buf[3] ;
    buf[0] = c;
    esp_write_tcp(buf,1, tcp_id);
}

void esp_write_tcp(const char * buf, uint8_t size, uint8_t tcp_id)
{
    uint8_t ok;
    char buffer[200];
    chprintf(stream,"AT+CIPSEND=%d,%04d\r\n",tcp_id, size);
    ok = wait_for_data_response(4000,buffer, sizeof (buffer),cfg->wdt);
    sdWrite(uart,(uint8_t *)buf,size);
    chprintf(stream,"\r");
    ok = wait_for_data_response(4000,buffer, sizeof(buffer),cfg->wdt);
}

const char * contains(const char * string, const char * substring)
{
    const char * sub = substring;
    const char * start;

    while (*string && *substring)
    {
        //find same index
        substring = sub;
        if (*string == *substring )
        {
            start = string;
            while(*string == *substring)
            {
                if (*(substring +1) == 0)
                {
                    return start;
                }
                substring++;
                string++;
            }
            string = start;

        }
        string++;
    }

    return 0;
}

void esp_simple_modbus(const char *raw, uint8_t len, uint8_t tcp_id, uint16_t *array, uint8_t array_size)
{
    const char * p;
    char buf[20];
    if (raw[0] == 'g')
    {
        p = raw + 1;
        if (strlen(p) < 2)
            return;
        uint8_t i = atoi(p);
        uint16_t r = 0xffff;

        if (i < array_size)
        {
            r = array[i];
        }

        buf[0] = 'G';
        itoa(buf+1,i,1,16);
        itoa(buf+3,r,2,16);
        esp_write_tcp(buf,strlen(buf),tcp_id);
    }
    else if (raw[0] == 'p')
    {
        p = raw + 1;
        if (strlen(p) < 6)
            return;

        uint8_t i = atoi(p);
        p += 2;
        uint16_t d = atoiw(p);

        if (i < array_size)
        {
            array[i] = d;
        }

        buf[0] = 'P';
        itoa(buf+1,i,1,16);

        esp_write_tcp(buf,strlen(buf),tcp_id);
    }
}
