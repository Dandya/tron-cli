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
#define UP 'u'
#define DOWN 'd'
#define LEFT 'l'
#define RIGHT 'r'

int work_flag = 1;
void handler(int none)
{
  work_flag = 0;
}

struct args_keys
{
  int* xstep;
  int* ystep;
  pthread_mutex_t* ptr_mtx;
};

void read_keys(struct args_keys* args)
{
  int* ptr_xstep = args->xstep;
  int* ptr_ystep = args->ystep;
  pthread_mutex_t* ptr_mtx = args->ptr_mtx;
  int ch = 0;
  while( ch != 'q' && work_flag )
  {
    ch = getch();
    pthread_mutex_lock(ptr_mtx);
    switch(ch)
    {
      case KEY_LEFT:
        *ptr_xstep = -1;
        *ptr_ystep = 0;
        break;
      case KEY_RIGHT:
        *ptr_xstep = 1;
        *ptr_ystep = 0;
        break;
      case KEY_UP:
        *ptr_xstep = 0;
        *ptr_ystep = -1;
        break;
     case KEY_DOWN:
        *ptr_xstep = 0;
        *ptr_ystep = 1;
        break;
     case 'q':
        *ptr_xstep = -1;
        *ptr_ystep = -1;
        break;
    } 
    pthread_mutex_unlock(ptr_mtx);
  }
}


int main(int argc, char *argv[])
{
  initscr();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  int fb, xstep, ystep;
  struct fb_var_screeninfo info;
  size_t fb_size, map_size, page_size;
  uint32_t *ptr, color;

  signal(SIGINT, handler);
  
  color = 0x30;
  xstep = ystep = 1;
  page_size = sysconf(_SC_PAGESIZE);
  
  if ( 0 > (fb = open("/dev/fb0", O_RDWR))) {
    perror("open");
    return __LINE__;
  }

  if ( (-1) == ioctl(fb, FBIOGET_VSCREENINFO, &info)) {
    perror("ioctl");
    close(fb);
    return __LINE__;
  }

  fb_size = sizeof(uint32_t) * info.xres_virtual * info.yres_virtual;
  map_size = (fb_size + (page_size - 1 )) & (~(page_size-1));

  ptr = mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
  if ( MAP_FAILED == ptr ) {
    perror("mmap");
    close(fb);
    return __LINE__;
  }

  int x_snake[40];
  int y_snake[40];
  int max_index = 39;

  x_snake[max_index] = 0;
  y_snake[max_index] = 5;
  for(int i = 0; i < max_index; i++)
  {
    x_snake[i] = info.xres_virtual-1;
    y_snake[i] = info.yres_virtual-1;
  }

  //RIGHT
  xstep = 1;
  ystep = 0;
  int tmp[] = {xstep, ystep}; 

  pthread_t tid;
  pthread_attr_t attr;
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

  if( pthread_attr_init(&attr) != 0 )
  {
    fprintf(stderr, "Error of init attr\n");
    return 1;
  }

  if( pthread_create(&tid, &attr,(void *)read_keys, &((struct args_keys){&xstep, &ystep, &mutex})) != 0 )
  {
    fprintf(stderr, "Error of create thread\n");
    return 2;
  }

  while(work_flag) 
  {
    ptr[y_snake[max_index] * info.xres_virtual  + x_snake[max_index]] = color;
    ptr[y_snake[0] * info.xres_virtual + x_snake[0]] = 0;

    for(int i = 0; i < max_index; i++)
    {
      x_snake[i] = x_snake[i+1];
      y_snake[i] = y_snake[i+1];
    }

    pthread_mutex_lock(&mutex);

    if( xstep == -1 && ystep == -1)
    {
      work_flag = 0;	    
    } 
   
    if( xstep == -tmp[0] || ystep == -tmp[1] )
    {
      xstep = tmp[0];
      ystep = tmp[1];
    }

    if( info.xres <= (x_snake[max_index] += xstep)) 
    {
      xstep = -xstep;
      x_snake[max_index] += 2 * xstep;
    }

    if( info.yres <= (y_snake[max_index] += ystep)) 
    {
      ystep = -ystep;
      y_snake[max_index] += 2 * ystep;
    }

    tmp[0] = xstep;
    tmp[1] = ystep;

    pthread_mutex_unlock(&mutex);

    color++;
    usleep(10000);
  }
  if( pthread_join(tid, NULL) != 0 )
  {
    fprintf(stderr, "Error of working thread\n");
  }

  munmap(ptr, map_size);
  close(fb);
  endwin();
  return 0;
}
