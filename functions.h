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

#include <arpa/inet.h>
#include <sys/socket.h>
//#include <ncurses.h>
#include <pthread.h>
#define UP 'w'
#define DOWN 's'
#define LEFT 'a'
#define RIGHT 'd'

#include "interaction.c"
