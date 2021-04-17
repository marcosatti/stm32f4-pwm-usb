#include <stddef.h>
#include <stdbool.h>
#include "slip.h"

#define END 0300
#define ESC 0333
#define ESC_END 0334
#define ESC_ESC 0335

static slip_config *config;

void slip_init(slip_config *config_) {
    config = config_;
}

void slip_deinit(void) {
    config = NULL;
}

void send_packet(char *p, int len)
{
    config->send_char(END);

    while (len--)
    {
        switch (*p)
        {
        case END:
            config->send_char(ESC);
            config->send_char(ESC_END);
            break;

        case ESC:
            config->send_char(ESC);
            config->send_char(ESC_ESC);
            break;

        default:
            config->send_char(*p);
        }

        p++;
    }

    config->send_char(END);
}

int recv_packet(char *p, int len)
{
    char c;
    int received = 0;

    while (true)
    {
        c = config->recv_char();

        if (received == 0 && config->check_start) 
        {
            if (c == END)
                continue;
            else
                return -1; /* Invalid start condition */
        }

        switch (c)
        {
        case END:
            break;

        case ESC:
            c = config->recv_char();

            switch (c)
            {
            case ESC_END:
                c = END;
                break;
            case ESC_ESC:
                c = ESC;
                break;
            /* Fallthrough intentional */
            }

        default:
            if (received < len)
                p[received++] = c;
            else
                return -2; /* Out of memory */
        }
    }
}
