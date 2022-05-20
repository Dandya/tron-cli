#include "functions.h"

/*
 * Note:
 * info.xres, info.yres - размер видимого экрана; 
 * info.xres_virtual, info.yres_virtual - размер всего экрана 
*/

int work_flag = 1;
int start_flag = 0;
static struct termios stored_settings;

void handler(int none);
void move_car(uint32_t** ptr_car, char direct, int scr_xres);
void invert_four_bytes(char *ptr);
void set_keypress(void);
void reset_keypress(void);
char is_cross(uint32_t * ptr_car_p1, uint32_t* ptr_car_p2, char direct_p1, char direct_p2, int scr_xres);

int main(int argc, char* argv[])
{ 
  if(argc < 4)
  {
      printf("Use: ./minitron.exe <xres> <yres> <opponent's ip>\n");
      return -1;
  }

  //init screen
  printf("\033c"); //clear stdout
  set_keypress(); // set noecho and cbreak modes stdin

  int fb;
  struct fb_var_screeninfo info;
  size_t fb_size, map_size, page_size;
  uint32_t *ptr;

  signal(SIGINT, handler);
  
  page_size = sysconf(_SC_PAGESIZE);
  
  if ( 0 > (fb = open("/dev/fb0", O_RDWR))) 
  {
    printf("open");
    reset_keypress();
    return __LINE__;
  }

  if ( (-1) == ioctl(fb, FBIOGET_VSCREENINFO, &info)) 
  {
    printf("ioctl");
    close(fb);
    reset_keypress();
    return __LINE__;
  }

  fb_size = sizeof(uint32_t) * info.xres_virtual * info.yres_virtual;
  map_size = (fb_size + (page_size - 1 )) & (~(page_size-1));

  ptr = mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
  if ( MAP_FAILED == ptr ) 
  {
    printf("mmap");
    close(fb);
    reset_keypress();
    return __LINE__;
  }
  
  int xres_area = atoi(argv[1]);
  int yres_area = atoi(argv[2]);
#ifdef WITHOUTCURSOR
  if(xres_area + 2 > info.xres || yres_area + 2 > info.yres) //considering the boundaries 
#else 
  if(xres_area + 2 > info.xres || yres_area + 2 > info.yres-32) //considering the boundaries and first line
#endif
  {
    munmap(ptr, map_size);
    close(fb);
    reset_keypress();
    printf("Big size of area");
    return __LINE__;
  }

  //init socket
  int sockfd;
  struct sockaddr_in opponent_addr, player_addr;
  
  if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    munmap(ptr, map_size);
    close(fb);
    reset_keypress();
    printf("Socket creation failed");
    return __LINE__;
  }
  
  opponent_addr.sin_family = AF_INET;
  opponent_addr.sin_port = htons(12345);
  opponent_addr.sin_addr.s_addr = inet_addr(argv[3]);
 
  player_addr.sin_family = AF_INET;
  player_addr.sin_port = htons(12345);
  player_addr.sin_addr.s_addr = get_local_ip(opponent_addr.sin_addr.s_addr);

  if(player_addr.sin_addr.s_addr == 0 || 
    player_addr.sin_addr.s_addr == opponent_addr.sin_addr.s_addr)
  {
    close(sockfd);
    munmap(ptr, map_size);
    close(fb);
    reset_keypress();
    printf("Incorrect opponent's ip\n");
    printf("Your ip:%s", inet_ntoa(player_addr.sin_addr));
    return __LINE__;
  }

  if(bind(sockfd, (struct sockaddr*)&player_addr, sizeof(player_addr)) < 0)
  {
    close(sockfd);
    munmap(ptr, map_size);
    close(fb);
    reset_keypress();
    printf("Bind error");
    return __LINE__;
  }

  //init threads
  pthread_t tid_control, tid_syncing;
  pthread_attr_t attr;
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  
  if( pthread_attr_init(&attr) != 0 )
  {
    close(sockfd);
    munmap(ptr, map_size);
    close(fb);
    reset_keypress();
    fprintf(stderr, "Error of init attr\n");
    return 1;
  }

  // init players
  uint32_t* ptr_car_p1 = ptr + info.xres/2 - xres_area/2 + 1 + info.xres_virtual*(info.yres/2 - yres_area/2 + 3);
  uint32_t* ptr_car_p2 = ptr + info.xres/2 - xres_area/2 + xres_area + 
      info.xres_virtual*(info.yres/2 -yres_area/2 + yres_area-2);
  char direct_p1 = RIGHT;
  char direct_p2= LEFT;
  char direct_prev_p1 = RIGHT;
  char direct_prev_p2 = LEFT;
  char is_need_additional_pixel = 0;
  char who_lose[] = {0,0};  //who_lose[0] - first player 
  char index_player = 0;    //who_lose[1] - second player
  char is_ready_p1 = 0;
  char is_ready_p2 = 0;

  struct args_keys args1 = {sockfd, &direct_p1, &is_ready_p1, &opponent_addr, &mutex};
  struct args_keys args2 = {sockfd, &direct_p2, &is_ready_p2, &opponent_addr, &mutex};  
  
  //invert bytes for compliance ips
  char opponent_ip[sizeof(unsigned long)];
  char player_ip[sizeof(unsigned long)];
  
  *((unsigned long*)opponent_ip) = opponent_addr.sin_addr.s_addr;
  *((unsigned long*)player_ip) = player_addr.sin_addr.s_addr;
  
  invert_four_bytes(opponent_ip);
  invert_four_bytes(player_ip);
  
  if(*((int*)player_ip) < *((int*)opponent_ip))
  {
      args1.ptr_direct = &direct_p2; args1.ptr_is_ready_player = &is_ready_p2;
      args2.ptr_direct = &direct_p1; args2.ptr_is_ready_player = &is_ready_p1;
      index_player = 1;
  }
  
  if( pthread_create(&tid_control, &attr,(void *)control_thread, &args1) != 0 )
  {
    close(sockfd);
    munmap(ptr, map_size);
    close(fb);
    reset_keypress();
    fprintf(stderr, "Error of create thread\n");
    return 2;
  }

  if( pthread_create(&tid_syncing, &attr,(void *)syncing_thread, &args2) != 0 )
  {
    close(sockfd);
    munmap(ptr, map_size);
    close(fb);
    reset_keypress();
    fprintf(stderr, "Error of create thread\n");
    return 2;
  }
  
  struct timeb tb; 
  time_t start_t; 
 #if CLOCK_PER_SEC == 1000000
  clock_t start_c;
 #endif

  ftime(&tb);
#ifndef WITHOUTCURSOR
  printf("For start press any key");
#endif
  start_t = tb.time;
  char game_start = 1;
  while(is_ready_p1 != 1 || is_ready_p2 != 1)
  {
      if(tb.time - start_t >= 10)
      {
        work_flag = 0;
        game_start = 0;
        is_ready_p1 = 1;
        is_ready_p2 = 1;
      }
      else
      {
        usleep(1);
        ftime(&tb);
      }
  }
  start_flag = 1;

  // start game 
 #if CLOCK_PER_SEC == 1000000
  start_c = clock();
 #endif
  uint32_t background_color = ptr_car_p2[0];
  draw_car(ptr_car_p1, direct_p1, RED, info.xres_virtual);
  draw_car(ptr_car_p2, direct_p2, BLUE, info.xres_virtual);
  char opposite_direct;
  draw_area(ptr+info.xres/2 - xres_area/2 + info.xres_virtual*(info.yres/2 - yres_area/2), xres_area, 
          yres_area, info.xres_virtual);
 
  while(work_flag)
  {
    pthread_mutex_lock(&mutex);
    // move first player's car 
    switch(direct_p1)
    {
      case UP:
      {
        opposite_direct = DOWN;
        if( direct_prev_p1 == LEFT || direct_prev_p1 == RIGHT )
            is_need_additional_pixel = 1;
        break;
      }
      case DOWN:
      {
        opposite_direct = UP;  
        if( direct_prev_p1 == LEFT || direct_prev_p1 == RIGHT )
            is_need_additional_pixel = 1;
        break;
      }
      case LEFT:
      {
        opposite_direct = RIGHT;  
        if( direct_prev_p1 == UP || direct_prev_p1 == DOWN )
            is_need_additional_pixel = 1;
        break;
      }
      case RIGHT:
      {
        opposite_direct = LEFT;  
        if( direct_prev_p1 == DOWN || direct_prev_p1 == DOWN )
            is_need_additional_pixel = 1;
        break;
      }
    }
    if(direct_prev_p1 != opposite_direct)
    {
      delete_car(ptr_car_p1, direct_prev_p1, info.xres_virtual, background_color);
      *ptr_car_p1 = RED;
      if(is_need_additional_pixel)
      {
        move_car(&ptr_car_p1, direct_p1, info.xres_virtual);
        *ptr_car_p1 = RED;
        is_need_additional_pixel = 0;
      }
      move_car(&ptr_car_p1, direct_p1, info.xres_virtual);
      if(draw_car(ptr_car_p1, direct_p1, RED, info.xres_virtual))
      {
        work_flag= 0;
        who_lose[0] = 1;
      }
      direct_prev_p1 = direct_p1;  
    }
    else
    {
      delete_car(ptr_car_p1, direct_prev_p1, info.xres_virtual, background_color);
      *ptr_car_p1 = RED;
      move_car(&ptr_car_p1, direct_prev_p1, info.xres_virtual);
      if(draw_car(ptr_car_p1, direct_prev_p1, RED, info.xres_virtual))
      {
        work_flag = 0;
        who_lose[0] = 1;
      }
    }
    // move second player's car
    switch(direct_p2)
    {
      case UP:
      {
        opposite_direct = DOWN;
        if( direct_prev_p2 == LEFT || direct_prev_p2 == RIGHT )
            is_need_additional_pixel = 1;
        break;
      }
      case DOWN:
      {
        opposite_direct = UP;
        if( direct_prev_p2 == LEFT || direct_prev_p2 == RIGHT )
            is_need_additional_pixel = 1;
        break;
      }
      case LEFT:
      {
        opposite_direct = RIGHT;
        if( direct_prev_p2 == UP || direct_prev_p2 == DOWN )
            is_need_additional_pixel = 1;
        break;
      }
      case RIGHT:
      {
        opposite_direct = LEFT;
        if( direct_prev_p2 == UP || direct_prev_p2 == DOWN )
            is_need_additional_pixel = 1;
        break;
      }
    }
    if(direct_prev_p2 != opposite_direct)
    {
      delete_car(ptr_car_p2, direct_prev_p2, info.xres_virtual, background_color);
      *ptr_car_p2 = BLUE;
      if(is_need_additional_pixel)
      {
        move_car(&ptr_car_p2, direct_p2, info.xres_virtual);
        *ptr_car_p2 = BLUE;
        is_need_additional_pixel = 0;
      }
      move_car(&ptr_car_p2, direct_p2, info.xres_virtual);
      if(draw_car(ptr_car_p2, direct_p2, BLUE, info.xres_virtual))
      {
        work_flag= 0;
        who_lose[1] = 1;
      }
      direct_prev_p2 = direct_p2;
    }
    else
    {
      delete_car(ptr_car_p2, direct_prev_p2, info.xres_virtual, background_color);
      *ptr_car_p2 = BLUE;
      move_car(&ptr_car_p2, direct_prev_p2, info.xres_virtual);
      if(draw_car(ptr_car_p2, direct_prev_p2, BLUE, info.xres_virtual))
      {
        work_flag = 0;
        who_lose[1] = 1;
      }
    }
    pthread_mutex_unlock(&mutex);    
   #if CLOCK_PER_SEC == 1000000
    usleep(62500 - clock() + start_c); // clock() returns microsecs
    start_c = clock();
   #else
    usleep(62500);
   #endif
  }
  if(is_cross(ptr_car_p1,ptr_car_p2, direct_prev_p1, direct_prev_p2, info.xres_virtual))
  {
    work_flag = 0;
    who_lose[0] = 1;
    who_lose[1] = 1;
  }
  //close all
  pthread_kill(tid_control, 17); 
  pthread_kill(tid_syncing, 17);

  close(sockfd);
  munmap(ptr, map_size);
  close(fb);
  reset_keypress();
  //print result of game
  printf("\033c\n\t*\t\t\t\t\t\t\t\t\t*\t\t*\n*\t\t\t\t*\t\t\t\t*\n\n\t*\t\t\t\t*\t\t\t\t\t\t*\n");
  if(who_lose[index_player] == 0 && who_lose[0] != who_lose[1])
    printf("\t\t\t\t\t\t\tYou win!\n");
  else if(game_start == 1)
    printf("\t\t\t\t\t\t\tYou lose:(\n");
  return 0;
}

void handler(int none)
{
  work_flag = 0;
}

void move_car(uint32_t** ptr_car, char direct, int scr_xres)
{
  switch(direct)
  {
    case UP:
      {
        *ptr_car -= scr_xres;
        break;
      }
      case DOWN:
      {
        *ptr_car += scr_xres;
        break;
      }
      case LEFT:
      {
        *ptr_car -= 1;
        break;
      }
      case RIGHT:
      {
        *ptr_car += 1;
        break;
      }
  }
}

void invert_four_bytes(char *ptr)
{
    char tmp=ptr[0];
    ptr[0]=ptr[3];
    ptr[3]=tmp;
    
    tmp=ptr[1];
    ptr[1]=ptr[2];
    ptr[2]=tmp;
}

void set_keypress(void)
{
#ifdef WITHOUTCURSOR
    printf("\e[?25l"); // poweroff print cursor  
#endif
    struct termios new_settings;

	tcgetattr(0,&stored_settings);

	new_settings = stored_settings;

	new_settings.c_lflag &= (~ICANON & ~ECHO);
	new_settings.c_cc[VTIME] = 0;
	new_settings.c_cc[VMIN] = 1;

	tcsetattr(0,TCSANOW,&new_settings);
	return;
}

void reset_keypress(void)
{
#ifdef WITHOUTCURSOR
    printf("\e[?25h"); // poweron cursor
#endif
    tcsetattr(0,TCSANOW,&stored_settings);
	return;
}

char is_cross(uint32_t * ptr_car_p1, uint32_t* ptr_car_p2, char direct_p1, char direct_p2, int scr_xres)
{
    uint32_t* car_p1[40];
    int index = 0;
    switch (direct_p1)
    {
        case UP:
            for(int i = 0; i>-8; i--)
            {
                for(int j = -2; j<=2; j++)
                {
                   car_p1[index] = ptr_car_p1 + j + i*scr_xres;
                   index ++;
                }
            }
            break;
        case DOWN:
            for(int i = 0; i<8; i++)
            {
                for(int j = -2; j<=2; j++)
                {
                   car_p1[index] = ptr_car_p1 + j + i*scr_xres;
                   index ++;
                }
            }
            break;
        case LEFT:
            for(int i = 0; i>-8; i--)
            {
                for(int j = -2; j<=2; j++)
                {
                   car_p1[index] = ptr_car_p1 + i + j*scr_xres;
                   index ++;
                }
            }
            break;
        case RIGHT:
            for(int i = 0; i<8; i++)
            {
                for(int j = -2; j<=2; j++)
                {
                   car_p1[index] = ptr_car_p1 + i + j*scr_xres;
                   index ++;
                }
            }
            break;
    }  
    index = 0;
    switch (direct_p2)
    {
        case UP:
            for(int i = 0; i>-8; i--)
            {
                for(int j = -2; j<=2; j++)
                {
                   if(car_p1[index] == ptr_car_p2 + j + i*scr_xres)
                   {
                     return 1;
                   }
                   index++;
                }
            }
            break;
        case DOWN:
            for(int i = 0; i<8; i++)
            {
                for(int j = -2; j<=2; j++)
                {
                   if(car_p1[index] == ptr_car_p2 + j + i*scr_xres)
                   {
                     return 1;
                   }
                   index++;
                }
            }
            break;
        case LEFT:
            for(int i = 0; i>-8; i--)
            {
                for(int j = -2; j<=2; j++)
                {
                   if(car_p1[index] == ptr_car_p2 + i + j*scr_xres)
                   {
                     return 1;
                   }
                   index++;
                }
            }
            break;
        case RIGHT:
            for(int i = 0; i<8; i++)
            {
                for(int j = -2; j<=2; j++)
                {
                   if(car_p1[index] == ptr_car_p2 + i + j*scr_xres)
                   {
                     return 1;
                   }
                   index++;
                }
            }
            break;
    }
    return 0;
}
