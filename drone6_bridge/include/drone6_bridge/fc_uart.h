#pragma once
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>

#include <stdint.h>
#include <string.h>

#define PI_UART "/dev/serial0"

class FC_UART
{
private:
    int serial_port = -1;

public:
    bool works;
    FC_UART(const FC_UART &) = delete;
    FC_UART &operator=(const FC_UART &) = delete;
    FC_UART(const char *port = PI_UART)
    {
        serial_port = open(port, O_RDWR);
        if (serial_port == -1)
        {
            printf("Error %i from open: %s\n", errno, strerror(errno));
            works = false;
            return;
        }
        struct termios tty;
        if (tcgetattr(serial_port, &tty) != 0)
        {
            printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
            works = false;
            return;
        }
        tty.c_cflag &= ~PARENB;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CSIZE;
        tty.c_cflag |= CS8;
        tty.c_cflag &= ~CRTSCTS;
        tty.c_cflag |= CREAD | CLOCAL;
        tty.c_lflag &= ~ICANON;
        tty.c_lflag &= ~ECHO;
        tty.c_lflag &= ~ECHOE;
        tty.c_lflag &= ~ECHONL;
        tty.c_lflag &= ~ISIG;
        tty.c_iflag &= ~(IXON | IXOFF | IXANY);
        tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);
        tty.c_oflag &= ~OPOST;
        tty.c_oflag &= ~ONLCR;
        tty.c_cc[VTIME] = 10;
        tty.c_cc[VMIN] = 0;
        cfsetispeed(&tty, B460800);
        cfsetospeed(&tty, B460800);
        if (tcsetattr(serial_port, TCSANOW, &tty) != 0)
        {
            printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
            works = false;
        }
        else
        {
            works = true;
        }
    };

    ~FC_UART(){
        if(serial_port != -1){
            close(serial_port);
        }
    }

    bool read_byte(uint8_t &out)
    {
        int num_bytes = read(serial_port, &out, 1);
        if (num_bytes < 0)
        {
            printf("Error reading: %s", strerror(errno));
            return false;
        }
        if (num_bytes == 0)
        {
            return false;
        }
        return true;
    };
};
