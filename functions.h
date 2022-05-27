#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/fb.h>
#include <string.h>
#include <sys/mman.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>

#include <pthread.h>

#include <arpa/inet.h>
#include <sys/socket.h>

#include <netdb.h>
#include <ifaddrs.h>
#include <linux/if_link.h>

#include <sys/timeb.h>
#include <time.h>

#define UP 'w'
#define DOWN 's'
#define LEFT 'a'
#define RIGHT 'd'


#define RED 0x00FF0F3F
#define BLUE 0x003080FF
#define WHITE 0x00FFFFBF
#define VIOLET 0xFF00FF

#include "interaction.c"
#include "draw.c"

