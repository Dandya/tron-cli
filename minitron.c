#include "functions.h"

/*
 * Note:
 * info.xres, info.yres - размер видимого экрана; 
 * info.xres_virtual, info.yres_virtual - размер всего экрана 
*/

int work_flag = 1;
int start_flag = 0;
int need_answer = 0;
char number_step = 0;
static struct termios stored_settings;

void handler(int none);
void move_car(uint32_t** ptr_car, char direct, int scr_xres);
char set_opposite_direct(char direct, char direct_prev, char* ptr_opposite_direct);
void invert_four_bytes(char *ptr);
void set_keypress(void);
void reset_keypress(void);
char is_cross(uint32_t * ptr_car_p1, uint32_t* ptr_car_p2, char direct_p1, char direct_p2, int scr_xres);

int main(int argc, char* argv[])
{ 
  int mode_sync = 0; // 0 - not syncing, 1 - syncing
  if(argc < 4)
  {
      printf("Use: ./minitron.exe <xres> <yres> <opponent's ip> <0-nsync, 1-sync>\n");
      return -1;
  }
  else if(argc == 5)
      mode_sync = atoi(argv[4]);

  signal(SIGINT, handler);
  //init screen
  printf("\033c"); //clear stdout
  set_keypress(); // set noecho and cbreak modes stdin

  int fb;
  struct fb_var_screeninfo info;
  size_t fb_size, map_size, page_size;
  uint32_t *ptr;
  
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
  if(xres_area + 2 > info.xres || yres_area + 2 > info.yres 
          || xres_area <= 10 || yres_area <= 10) //considering the boundaries 
#else 
  if(xres_area + 2 > info.xres || yres_area + 2 > info.yres-32
          || xres_area <= 10 || yres_area <= 10) //considering the boundaries and first line
#endif
  {
    munmap(ptr, map_size);
    close(fb);
    reset_keypress();
    printf("Not supported size of area");
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
  pthread_t tid_control, tid_syncing, tid_send;
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
  char direct_p1;
  char direct_p2;
  char direct_prev_p1 = RIGHT;
  char direct_prev_p2 = LEFT;
  char opposite_direct_p1;
  char opposite_direct_p2;
  char is_need_additional_pixel_p1 = 0;
  char is_need_additional_pixel_p2 = 0;
  char who_lose[] = {0,0};  //who_lose[0] - first player 
  char index_player = 0;    //who_lose[1] - second player
  char is_ready_p1 = 0;     //if index_player == 0 then player is master else slave
  char is_ready_p2 = 0;
  char tmp = 0;

  struct args_keys args1 = {sockfd, &direct_p1, &is_ready_p1, &opponent_addr, &mutex};
  struct args_keys args2 = {sockfd, &direct_p2, &is_ready_p2, &opponent_addr, &mutex};  
  void (*control_thread) (struct args_keys* args);
  void (*syncing_thread) (struct args_keys* args);
  if (mode_sync)
  {
      control_thread = control_thread_sync;
      syncing_thread = interaction_thread_sync;
  }
  else
  {
      control_thread = control_thread_nsync;
      syncing_thread = interaction_thread_nsync;

  }

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

  if( pthread_create(&tid_send, &attr,(void *)send_to_opponent, &args1) != 0 )
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

  // wait
#ifndef WITHOUTCURSOR
  printf("For start press key(q - quit)");
#endif
  char game_start = 1;
  while((is_ready_p1 != 1 || is_ready_p2 != 1) && work_flag == 1)
  {
    usleep(1);
  }
  if(work_flag == 0)
  {
    game_start = 0;
  }
  start_flag = 1;

  // start game 
  struct timeb tb;  
  ftime(&tb);
  unsigned start_m = tb.millitm;
  time_t start_s = tb.time; // seconds
  uint32_t background_color = ptr_car_p2[0];
  if(game_start == 1)
  {
    pthread_mutex_lock(&mutex);
    direct_p1 = RIGHT;
    direct_p2 = LEFT;
    draw_area(ptr+info.xres/2 - xres_area/2 + info.xres_virtual*(info.yres/2 - yres_area/2), xres_area, 
            yres_area, info.xres_virtual);
    draw_car(ptr_car_p1, direct_p1, RED, info.xres_virtual);
    draw_car(ptr_car_p2, direct_p2, BLUE, info.xres_virtual);
    pthread_mutex_unlock(&mutex);
  }
 
  while(work_flag)
  {
    if(mode_sync) //with sync
    {
        if(index_player == 0) //player is master
        {
            pthread_mutex_lock(&mutex);
            is_need_additional_pixel_p1 = set_opposite_direct(direct_p1, direct_prev_p1, &opposite_direct_p1);
            need_answer = 1;
            if(direct_prev_p1 != opposite_direct_p1)
                tmp = direct_p1 + number_step % 2;
            else 
                tmp = direct_prev_p1 + number_step % 2;
            while(need_answer)
            {
                sendto(sockfd, &tmp, 1, 0,(struct sockaddr*)(&opponent_addr), sizeof(opponent_addr));
                ftime(&tb);
                if(tb.time - start_s > 5)
                {
                    need_answer = 0;
                    work_flag = 0;
                }
                usleep(50);
            }
            if(work_flag == 0)
            {
                pthread_mutex_unlock(&mutex);
                continue;
            }
            is_need_additional_pixel_p2 = set_opposite_direct(direct_p2, direct_prev_p2, &opposite_direct_p2);
        }
        else //player is slave
        {
            need_answer = 1;
            while(need_answer)
            {
                sendto(sockfd, &tmp, 1, 0, (struct sockaddr*)(&opponent_addr), sizeof(opponent_addr));
                ftime(&tb);
                if(tb.time - start_s > 5)
                {
                    need_answer = 0;
                    work_flag = 0;
                }
                usleep(1); 
            }
            pthread_mutex_lock(&mutex);
            if(work_flag == 0)
            {
                pthread_mutex_unlock(&mutex);
                continue;
            }
            is_need_additional_pixel_p2 = set_opposite_direct(direct_p2, direct_prev_p2, &opposite_direct_p2);
            if(direct_prev_p1 != opposite_direct_p1)
                tmp = direct_p2 + number_step % 2;
            else
                tmp = direct_prev_p2 + number_step % 2;
            for(int i = 0; i<10; i++)
                sendto(sockfd, &tmp, 1, 0, (struct sockaddr*)(&opponent_addr), sizeof(opponent_addr));
            is_need_additional_pixel_p1 = set_opposite_direct(direct_p1, direct_prev_p1, &opposite_direct_p1);
        }
    }
    else //without sync
    {
        pthread_mutex_lock(&mutex);
        is_need_additional_pixel_p1 = set_opposite_direct(direct_p1, direct_prev_p1, &opposite_direct_p1);
        is_need_additional_pixel_p2 = set_opposite_direct(direct_p2, direct_prev_p2, &opposite_direct_p2);
    }
    // move first player's car 
    if(direct_prev_p1 != opposite_direct_p1)
    {
      delete_car(ptr_car_p1, direct_prev_p1, info.xres_virtual, background_color);
      *ptr_car_p1 = RED;
      if(is_need_additional_pixel_p1)
      {
        move_car(&ptr_car_p1, direct_p1, info.xres_virtual);
        *ptr_car_p1 = RED;
        is_need_additional_pixel_p1 = 0;
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
    if(direct_prev_p2 != opposite_direct_p2)
    {
      delete_car(ptr_car_p2, direct_prev_p2, info.xres_virtual, background_color);
      *ptr_car_p2 = BLUE;
      if(is_need_additional_pixel_p2)
      {
        move_car(&ptr_car_p2, direct_p2, info.xres_virtual);
        *ptr_car_p2 = BLUE;
        is_need_additional_pixel_p2 = 0;
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
    if(mode_sync && index_player == 0 || mode_sync == 0)
    {
      if(mode_sync) //player is master
      {
        ftime(&tb);
        usleep(62500 - (((unsigned)(tb.millitm  - start_m) < 10 ) ? (tb.millitm - start_m)*1000 : 5500)); 
        ftime(&tb);
        start_m = tb.millitm;
        start_s = tb.time;
      }
      else // nsync mode
          usleep(62500);
    }
    else //player is slave
    {
        usleep(20000);
        start_s = tb.time;
    }
    number_step++;
  }
  
  if(is_cross(ptr_car_p1,ptr_car_p2, direct_prev_p1, direct_prev_p2, info.xres_virtual))
  {
    work_flag = 0;
    who_lose[0] = 1;
    who_lose[1] = 1;
  }

  //print result of game
  if(who_lose[index_player] == 0 && who_lose[0] != who_lose[1])
  {
      if(index_player == 0)
        draw_area_in_color(ptr+info.xres/2 - xres_area/2 + info.xres_virtual*(info.yres/2 - yres_area/2), xres_area, 
            yres_area, info.xres_virtual, RED);
      else
        draw_area_in_color(ptr+info.xres/2 - xres_area/2 + info.xres_virtual*(info.yres/2 - yres_area/2), xres_area, 
            yres_area, info.xres_virtual, BLUE);
  }
  else if(game_start == 1)
  {
      if(who_lose[index_player] == 1 && who_lose[0] != who_lose[1])
      {
        if(index_player == 0)
          draw_area_in_color(ptr+info.xres/2 - xres_area/2 + info.xres_virtual*(info.yres/2 - yres_area/2), xres_area, 
                yres_area, info.xres_virtual, BLUE);
        else
            draw_area_in_color(ptr+info.xres/2 - xres_area/2 + info.xres_virtual*(info.yres/2 - yres_area/2), xres_area, 
                yres_area, info.xres_virtual, RED);
      }
      else
          draw_area_in_color(ptr+info.xres/2 - xres_area/2 + info.xres_virtual*(info.yres/2 - yres_area/2), xres_area, 
                yres_area, info.xres_virtual, VIOLET);
  }

  //close all
  pthread_join(tid_control, NULL);
  pthread_join(tid_send, NULL);
  pthread_kill(tid_syncing, 17);
  close(sockfd);
  munmap(ptr, map_size);
  close(fb);
  reset_keypress();
  return 0;
}

void handler(int none)
{
    exit(0);
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

char set_opposite_direct(char direct, char direct_prev, char* ptr_opposite_direct)
{
    switch(direct)
    {
      case UP:
      {
        *ptr_opposite_direct = DOWN;
        if( direct_prev == LEFT || direct_prev == RIGHT )
            return 1;
        else
            return 0;
      }
      case DOWN:
      {
        *ptr_opposite_direct = UP;  
        if( direct_prev == LEFT || direct_prev == RIGHT )
            return 1;
        else 
            return 0;
        break;
      }
      case LEFT:
      {
        *ptr_opposite_direct = RIGHT;  
        if( direct_prev == UP || direct_prev == DOWN )
            return 1;
        else 
            return 0;
      }
      case RIGHT:
      {
        *ptr_opposite_direct = LEFT;  
        if( direct_prev == UP || direct_prev == DOWN )
            return 1;
        else 
            return 0;
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
                  for(int indx = 0; indx < 40; indx++)
                  {
                   uint32_t* result = ptr_car_p2 + j + i*scr_xres;
                   if(car_p1[indx] == result)
                   {
                     return 1;
                   }
                  }
                 }
            }
            break;
        case DOWN:
            for(int i = 0; i<8; i++)
            {
                for(int j = -2; j<=2; j++)
                {
                  for(int indx = 0; indx < 40; indx++)
                  {
                   uint32_t* result = ptr_car_p2 + j + i*scr_xres;
                   if(car_p1[indx] == result)
                   {
                     return 1;
                   }
                  }
                }
            }
            break;
        case LEFT:
            for(int i = 0; i>-8; i--)
            {
                for(int j = -2; j<=2; j++)
                {
                  for(int indx = 0; indx < 40; indx++)
                  {
                   uint32_t* result = ptr_car_p2 + i + j*scr_xres;
                   if(car_p1[indx] == result)
                   {
                     return 1;
                   }
                  }
                }
            }
            break;
        case RIGHT:
            for(int i = 0; i<8; i++)
            {
                for(int j = -2; j<=2; j++)
                {
                  for(int indx = 0; indx < 40; indx++)
                  {
                   uint32_t* result = ptr_car_p2 + i + j*scr_xres;
                   if(car_p1[indx] == result)
                   {
                     return 1;
                   }
                  }
                }
            }
            break;
    }
    return 0;
}
