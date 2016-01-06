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
uint8_t wait_for_data(uint16_t to, char * buf, reset_watchdog_t cb);
static systime_t last_data_rec;

static const esp_config_t * cfg;

static int16_t signal_strength;

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


void esp_set_sd(const esp_config_t *config)
{
    cfg = config;
    uart = cfg->sd;
    stream = (BaseSequentialStream*)uart;
}

int16_t esp_signal_strength()
{
    int16_t stren = 0;

    char buffer[128];
    char * p ;

    if (esp_run_command("AT+CWJAP?", 100,buffer))
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


    ok = esp_run_command("AT", 20, buffer);
    if (!ok)
        return 0;

    ok = esp_run_command("AT+CWMODE=1", 500, buffer);
    if (!ok)
        return 0;

    chprintf(stream,"AT+CWJAP=\"%s\",\"%s\"\r\n",essid,password);
    ok = wait_for_data(20000, buffer, cfg->wdt);

    return ok;
}

int16_t esp_ping(const char * address)
{
    char buffer[128];
    char * p;
    int16_t ping = 0;

    chprintf(stream,"AT+PING=\"%s\"\r\n",address);
    uint8_t ok = wait_for_data(1500,buffer, cfg->wdt);


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
            ok = esp_run_command("AT+CIPCLOSE",500,buffer);
            ok = esp_run_command("AT+CIPMUX=1",500,buffer);
            ok = esp_run_command("AT+CIPCLOSE=0",500,buffer);
            ok = esp_run_command("AT+CIPCLOSE=1",500,buffer);
            ok = esp_run_command("AT+CIPCLOSE=2",500,buffer);
            ok = esp_run_command("AT+CIPCLOSE=3",500,buffer);
            ok = esp_run_command("AT+CIPCLOSE=4",500,buffer);
            ok = esp_run_command("AT+CIPSERVER=0",500,buffer);

            if (open_port)
            {
                chprintf(stream,"AT+CIPSERVER=1,%d\r\n",open_port);
                ok = wait_for_data(500,buffer, NULL);
            }
            else
            {
                ok = 1;
            }
        }
    }


    return ok;
}


uint8_t esp_run_command(const char * text, uint16_t timeout, char * response)
{
    chprintf(stream,text);
    chprintf(stream,"\r\n");
    return wait_for_data(timeout,response,NULL);
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
        status = esp_run_command(text,to, buffer);

        if (!status)
            return 0;
    }
    return 1;
}


uint16_t esp_decode_ipd(char * buf, uint8_t * iid)
{
    uint8_t id;
    char * p;
    uint16_t len;
    const char * po;
    char * help;
    char buff[100];
    msg_t t;

    //wait for connection
    //remember connection ID - always 0
    //listen data - decode +IPD,ID,length:data

    len = sdReadTimeout(uart,(uint8_t *)buff,9, MS2ST(500));

    if (!len)
        return 0;

    *(buff + len) = 0;

    p = strchr(buff,':');
    po = contains(buff,"+IPD");
    if (!po)
        return 0;
    while (!p)
    {
        t = sdGetTimeout(uart, MS2ST(10));
        if (t == -1 )
            return 0;

        *(buff + len++) = t;
        *(buff + len) = 0;
        p = strchr(buff,':');
    }

    if (p)
    {
        help = strchr(po,',');
        help++;
        id = *(help) - '0';
        if (iid)
            *iid = id;
        help = strchr(help,',') + 1;
        len = 0;

        for (;help < p; help++)
        {
            len = 10 * len + (*help) - '0';
        }
    }

    //read data
    uint16_t l;
    l = sdReadTimeout(uart,(uint8_t *)buf,len, MS2ST(len));
    if (len != l)
        return 0;
    *(buf + len) = 0;

    last_data_rec = chVTGetSystemTime();
    return len;
}


uint8_t wait_for_data(uint16_t to, char * buf, reset_watchdog_t cb)
{
    chDbgAssert(buf, "cannot be zero");
    char * p;
    uint16_t i;
    msg_t z;

    p = buf;
    for (i = 0 ; i < to; i++)
    {
        z = sdGetTimeout(uart,MS2ST(1));
        if (z == -1)
        {
            //timeout
        }
        else
        {
            *p = z;
            p++;
            *p = 0;
        }

        if (cb)
            cb();

        if(contains(buf,"OK\r") )
        {
            return 1;
        }
        if (contains(buf,"ERROR") || contains(buf,"FAIL"))
        {
            //error occured
            return 0;
        }
        //no break means timeout
    }
    return 0;
}

void esp_write_tcp_char(char c, uint8_t tcp_id)
{
    char buf[3] ;
    buf[0] = c;
    esp_write_tcp(buf,1, tcp_id);
}

void esp_write_tcp(char * buf, uint8_t size, uint8_t tcp_id)
{
    uint8_t ok;
    char buffer[200];
    chprintf(stream,"AT+CIPSEND=%d,%04d\r\n",tcp_id, size);
    ok = wait_for_data(4000,buffer,cfg->wdt);
    sdWrite(uart,(uint8_t *)buf,size);
    chprintf(stream,"\r");
    ok = wait_for_data(4000,buffer,cfg->wdt);
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
