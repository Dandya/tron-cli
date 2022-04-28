#include "functions.h"

/*
 * Problems:
 * 1) Как узнать собственный адрес?
 * TODO: 
 * 1) Проверка на размеры экрана (+2 пикселя для каждой координаты) +
 * 2) Сравние IP
 * 3) Ввод размеров поля +
 * Notes:
 * 1) info.xres, info.yres - размер видимого экрана; 
 *              info.xres_virtual, info.yres_virtual - размер всего экрана 
*/

int work_flag= 1;

void handler(int none)
{
  work_flag = 0;
}

int move_car(int* ptr_car, char* ptr_direct, char* ptr_direct_prev, uint32_t color)
{
    switch(direct_p1)
    {
        case UP:
        {
            if(direct_prev_p1 == DOWN)
            {
                delete_car(ptr_car_p1, direct_prev_p1, info.xres_virtual);
                *ptr_car_p1 = RED;
          ptr_car_p1 += info.xres_virtual;
          if(draw_car(ptr_car_p1, DOWN, RED, info.xres_virtual))
          {
            work_flag = 0;
          }
          break;
        }
        delete_car(ptr_car_p1, direct_prev_p1, info.xres_virtual);
        *ptr_car_p1 = RED;
        ptr_car_p1 -= info.xres_virtual;
        if(draw_car(ptr_car_p1, UP, RED, info.xres_virtual))
        {
          work_flag= 0;
        }
        direct_prev_p1 = UP;
        break;
      }
      case DOWN:
      {
        if(direct_prev_p1 == UP)
        {
          delete_car(ptr_car_p1, direct_prev_p1, info.xres_virtual);
          *ptr_car_p1 = RED;
          ptr_car_p1 -= info.xres_virtual;
          if(draw_car(ptr_car_p1, UP, RED, info.xres_virtual))
          {
            work_flag = 0;
          }
          break;
        }
        delete_car(ptr_car_p1, direct_prev_p1, info.xres_virtual);
        *ptr_car_p1 = RED;
        ptr_car_p1 += info.xres_virtual;
        if(draw_car(ptr_car_p1, DOWN, RED, info.xres_virtual))
        {
          work_flag = 0;
        }
        direct_prev_p1 = DOWN;
        break;
      }
      case LEFT:
      {
        if(direct_prev_p1 == RIGHT)
        {
          delete_car(ptr_car_p1, direct_prev_p1, info.xres_virtual);
          *ptr_car_p1 = RED;
          ptr_car_p1 += 1;
          if(draw_car(ptr_car_p1, RIGHT, RED, info.xres_virtual))
          {
            work_flag = 0;
          }
          break;
        }
        delete_car(ptr_car_p1, direct_prev_p1, info.xres_virtual);
        *ptr_car_p1 = RED;
        ptr_car_p1 -= 1;
        if(draw_car(ptr_car_p1, LEFT, RED, info.xres_virtual))
        {
          work_flag = 0;
        }
        direct_prev_p1 = LEFT;
        break;
      }
      case RIGHT:
      {
        if(direct_prev_p1 == LEFT)
        {
          delete_car(ptr_car_p1, direct_prev_p1, info.xres_virtual);
          *ptr_car_p1 = RED;
          ptr_car_p1 -= 1;
          if(draw_car(ptr_car_p1, LEFT, RED, info.xres_virtual))
          {
            work_flag = 0;
          }
          break;
        }
        delete_car(ptr_car_p1, direct_prev_p1, info.xres_virtual);
        *ptr_car_p1 = RED;
        ptr_car_p1 += 1;
        if(draw_car(ptr_car_p1, RIGHT, RED, info.xres_virtual))
        {
          work_flag = 0;
        }
        direct_prev_p1 = RIGHT;
        break;
      }
    }
}

int main(int argc, char* argv[])
{ 
  printf("\033c"); //clear stdout
  initscr();
  noecho();
  cbreak();
  curs_set(0);
 // keypad(stdscr, TRUE);
  int fb, xstep, ystep;
  struct fb_var_screeninfo info;
  size_t fb_size, map_size, page_size;
  uint32_t *ptr, color;

  signal(SIGINT, handler);
  
  color = 0x00FFFFFF;
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
  
  int xres_area = atoi(argv[1]);
  int yres_area = atoi(argv[2]);
  if(xres_area + 2 > info.xres || yres_area + 2 > info.yres) //considering the boundaries
  {
    munmap(ptr, map_size);
    close(fb);
    endwin();
    printf("Big size of area\n");
    return 0;
  }

  int sockfd;
  struct sockaddr_in opponent_addr, server_addr;
  
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  
  opponent_addr.sin_family = AF_INET;
  opponent_addr.sin_port = htons(12345);
  opponent_addr.sin_addr.s_addr = inet_addr(argv[3]);
 
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(12345);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
  
  pthread_t tid_control, tid_syncing;
  pthread_attr_t attr;
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
 
  // init players
  uint32_t* ptr_car_p1 = ptr + info.xres/2 - xres_area/2 + 3 + info.xres_virtual*(info.yres/2 - yres_area/2 + 1);
  uint32_t* ptr_car_p2 = ptr + info.xres/2 - xres_area/2 + xres_area-2 + 
      info.xres_virtual*(info.yres/2 -yres_area/2 + yres_area);
  char direct_p1 = DOWN;
  char direct_p2= UP;
  char direct_prev_p1 = DOWN;
  char direct_prev_p2 = UP;
 
  if( pthread_attr_init(&attr) != 0 )
  {
    fprintf(stderr, "Error of init attr\n");
    return 1;
  }
//  printf("All wait\n");
  struct args_keys args1 = {sockfd, &direct_p1 ,&opponent_addr, &mutex};
  struct args_keys args2 = {sockfd, &direct_p2 ,&opponent_addr, &mutex};  
  
  if( pthread_create(&tid_control, &attr,(void *)control_thread, &args1) != 0 )
  {
    fprintf(stderr, "Error of create thread\n");
    return 2;
  }

  if( pthread_create(&tid_syncing, &attr,(void *)syncing_thread, &args2) != 0 )
  {
    fprintf(stderr, "Error of create thread\n");
    return 2;
  }
//  printf("Threads created\n"); 
//  refresh();
  draw_area(ptr+info.xres/2 - xres_area/2 + info.xres_virtual*(info.yres/2 - yres_area/2), xres_area, 
          yres_area, info.xres_virtual);
  draw_car(ptr_car_p1, direct_p1, RED, info.xres_virtual);
  draw_car(ptr_car_p2, direct_p2, BLUE, info.xres_virtual);
  while(work_flag)
  { 
    pthread_mutex_lock(&mutex);
    switch(direct_p1)
    {
      case UP:
      {
        if(direct_prev_p1 == DOWN)
        {
          delete_car(ptr_car_p1, direct_prev_p1, info.xres_virtual);
          *ptr_car_p1 = RED;
          ptr_car_p1 += info.xres_virtual;
          if(draw_car(ptr_car_p1, DOWN, RED, info.xres_virtual))
          {
            work_flag = 0;
          }
          break;
        }
        delete_car(ptr_car_p1, direct_prev_p1, info.xres_virtual);
        *ptr_car_p1 = RED;
        ptr_car_p1 -= info.xres_virtual;
        if(draw_car(ptr_car_p1, UP, RED, info.xres_virtual))
        {
          work_flag= 0;
        }
        direct_prev_p1 = UP;
        break;
      }
      case DOWN:
      {
        if(direct_prev_p1 == UP)
        {
          delete_car(ptr_car_p1, direct_prev_p1, info.xres_virtual);
          *ptr_car_p1 = RED;
          ptr_car_p1 -= info.xres_virtual;
          if(draw_car(ptr_car_p1, UP, RED, info.xres_virtual))
          {
            work_flag = 0;
          }
          break;
        }
        delete_car(ptr_car_p1, direct_prev_p1, info.xres_virtual);
        *ptr_car_p1 = RED;
        ptr_car_p1 += info.xres_virtual;
        if(draw_car(ptr_car_p1, DOWN, RED, info.xres_virtual))
        {
          work_flag = 0;
        }
        direct_prev_p1 = DOWN;
        break;
      }
      case LEFT:
      {
        if(direct_prev_p1 == RIGHT)
        {
          delete_car(ptr_car_p1, direct_prev_p1, info.xres_virtual);
          *ptr_car_p1 = RED;
          ptr_car_p1 += 1;
          if(draw_car(ptr_car_p1, RIGHT, RED, info.xres_virtual))
          {
            work_flag = 0;
          }
          break;
        }
        delete_car(ptr_car_p1, direct_prev_p1, info.xres_virtual);
        *ptr_car_p1 = RED;
        ptr_car_p1 -= 1;
        if(draw_car(ptr_car_p1, LEFT, RED, info.xres_virtual))
        {
          work_flag = 0;
        }
        direct_prev_p1 = LEFT;
        break;
      }
      case RIGHT:
      {
        if(direct_prev_p1 == LEFT)
        {
          delete_car(ptr_car_p1, direct_prev_p1, info.xres_virtual);
          *ptr_car_p1 = RED;
          ptr_car_p1 -= 1;
          if(draw_car(ptr_car_p1, LEFT, RED, info.xres_virtual))
          {
            work_flag = 0;
          }
          break;
        }
        delete_car(ptr_car_p1, direct_prev_p1, info.xres_virtual);
        *ptr_car_p1 = RED;
        ptr_car_p1 += 1;
        if(draw_car(ptr_car_p1, RIGHT, RED, info.xres_virtual))
        {
          work_flag = 0;
        }
        direct_prev_p1 = RIGHT;
        break;
      }
    }
    switch(direct_p2)
    {
      case UP:
      {
        if(direct_prev_p2 == DOWN)
        {
          delete_car(ptr_car_p2, direct_prev_p2, info.xres_virtual);
          *ptr_car_p2 = BLUE;
          ptr_car_p2 += info.xres_virtual;
          if(draw_car(ptr_car_p2, DOWN, BLUE, info.xres_virtual))
          {
            work_flag = 0;
          }
          break;
        }
        delete_car(ptr_car_p2, direct_prev_p2, info.xres_virtual);
        *ptr_car_p2 = BLUE;
        ptr_car_p2 -= info.xres_virtual;
        if(draw_car(ptr_car_p2, UP, BLUE, info.xres_virtual))
        {
          work_flag= 0;
        }
        direct_prev_p2 = UP;
        break;
      }
      case DOWN:
      {
        if(direct_prev_p2 == UP)
        {
          delete_car(ptr_car_p2, direct_prev_p2, info.xres_virtual);
          *ptr_car_p2 = RED;
          ptr_car_p2 -= info.xres_virtual;
          if(draw_car(ptr_car_p2, UP, RED, info.xres_virtual))
          {
            work_flag = 0;
          }
          break;
        }
        delete_car(ptr_car_p2, direct_prev_p2, info.xres_virtual);
        *ptr_car_p2 = RED;
        ptr_car_p2 += info.xres_virtual;
        if(draw_car(ptr_car_p2, DOWN, RED, info.xres_virtual))
        {
          work_flag = 0;
        }
        direct_prev_p2 = DOWN;
        break;
      }
      case LEFT:
      {
        if(direct_prev_p2 == RIGHT)
        {
          delete_car(ptr_car_p2, direct_prev_p2, info.xres_virtual);
          *ptr_car_p2 = RED;
          ptr_car_p2 += 1;
          if(draw_car(ptr_car_p2, RIGHT, RED, info.xres_virtual))
          {
            work_flag = 0;
          }
          break;
        }
        delete_car(ptr_car_p2, direct_prev_p2, info.xres_virtual);
        *ptr_car_p2 = RED;
        ptr_car_p2 -= 1;
        if(draw_car(ptr_car_p2, LEFT, RED, info.xres_virtual))
        {
          work_flag = 0;
        }
        direct_prev_p2 = LEFT;
        break;
      }
      case RIGHT:
      {
        if(direct_prev_p2 == LEFT)
        {
          delete_car(ptr_car_p2, direct_prev_p2, info.xres_virtual);
          *ptr_car_p2 = RED;
          ptr_car_p2 -= 1;
          if(draw_car(ptr_car_p2, LEFT, RED, info.xres_virtual))
          {
            work_flag = 0;
          }
          break;
        }
        delete_car(ptr_car_p2, direct_prev_p2, info.xres_virtual);
        *ptr_car_p2 = RED;
        ptr_car_p2 += 1;
        if(draw_car(ptr_car_p2, RIGHT, RED, info.xres_virtual))
        {
          work_flag = 0;
        }
        direct_prev_p2 = RIGHT;
        break;
      }
    }
    pthread_mutex_unlock(&mutex);
    usleep(62500);
    //usleep(5000);
  }
  if( pthread_join(tid_control, NULL) != 0 || pthread_kill(tid_syncing, 17) != 0 )
  {
    fprintf(stderr, "Error of working thread\n");
  }

  close(sockfd);
  munmap(ptr, map_size);
  close(fb);
  //getchar();
  endwin();
  printf("\033c\n\t*\t\t\t\t\t\t\t\t\t*\t\t*\n*\t\t\t\t*\t\t\t\t*\n\n\t*\t\t\t\t*\t\t\t\t\t\t*\n");
  printf("\t\t\t\t\t\t\tGood!\n");
  return 0;
}
