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
#include <ncurses.h>
#include <pthread.h>

#include <arpa/inet.h>
#include <sys/socket.h>

#include <netdb.h>
#include <ifaddrs.h>
#include <linux/if_link.h>

#include <sys/timeb.h>

#define UP 'w'
#define DOWN 's'
#define LEFT 'a'
#define RIGHT 'd'

#define RED 0x00FF0000
#define BLUE 0x000000FF
#define WHITE 0x00FFFFFF

#include "interaction.c"
#include "draw.c"

#define DEBUG
