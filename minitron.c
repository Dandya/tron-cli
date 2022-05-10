#include "functions.h"

/*
 * Note:
 * info.xres, info.yres - размер видимого экрана; 
 * info.xres_virtual, info.yres_virtual - размер всего экрана 
*/

int work_flag= 1;

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

int main(int argc, char* argv[])
{ 

  // time of one move car
  
  if(argc < 4)
  {
      printf("Use: ./minitron.exe <xres> <yres> <opponent's ip>\n");
      return -1;
  }

  //init screen
  printf("\033c"); //clear stdout
  initscr();
  noecho();
  cbreak();
  halfdelay(0);
  curs_set(0);
  int fb, xstep, ystep;
  struct fb_var_screeninfo info;
  size_t fb_size, map_size, page_size;
  uint32_t *ptr, color;

  signal(SIGINT, handler);
  
  color = 0x00FFFFFF;
  xstep = ystep = 1;
  page_size = sysconf(_SC_PAGESIZE);
  
  if ( 0 > (fb = open("/dev/fb0", O_RDWR))) 
  {
    perror("open");
    return __LINE__;
  }

  if ( (-1) == ioctl(fb, FBIOGET_VSCREENINFO, &info)) 
  {
    perror("ioctl");
    close(fb);
    return __LINE__;
  }

  fb_size = sizeof(uint32_t) * info.xres_virtual * info.yres_virtual;
  map_size = (fb_size + (page_size - 1 )) & (~(page_size-1));

  ptr = mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
  if ( MAP_FAILED == ptr ) 
  {
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
    perror("Big size of area");
    return __LINE__;
  }

  //init socket
  int sockfd;
  struct sockaddr_in opponent_addr, player_addr;
  
  if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    munmap(ptr, map_size);
    close(fb);
    endwin();
    perror("Socket creation failed");
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
    endwin();
    perror("Incorrect opponent's ip");
    printf("Your ip:%s", inet_ntoa(player_addr.sin_addr));
    return __LINE__;
  }


  if(bind(sockfd, (struct sockaddr*)&player_addr, sizeof(player_addr)) < 0)
  {
    close(sockfd);
    munmap(ptr, map_size);
    close(fb);
    endwin();
    perror("Bind error");
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
    endwin();
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
      args1.ptr_direct = &direct_p2;
      args1.ptr_is_ready_player = &is_ready_p2;
      args2.ptr_direct = &direct_p1;
      args2.ptr_is_ready_player = &is_ready_p1;
      index_player = 1;
  }
  
  pthread_mutex_lock(&mutex); // for wait players

  if( pthread_create(&tid_control, &attr,(void *)control_thread, &args1) != 0 )
  {
    close(sockfd);
    munmap(ptr, map_size);
    close(fb);
    endwin();
    fprintf(stderr, "Error of create thread\n");
    return 2;
  }

  if( pthread_create(&tid_syncing, &attr,(void *)syncing_thread, &args2) != 0 )
  {
    close(sockfd);
    munmap(ptr, map_size);
    close(fb);
    endwin();
    fprintf(stderr, "Error of create thread\n");
    return 2;
  }
  
  // start game
  uint32_t background_color = ptr_car_p2[0];
  draw_area(ptr+info.xres/2 - xres_area/2 + info.xres_virtual*(info.yres/2 - yres_area/2), xres_area, 
          yres_area, info.xres_virtual);
  draw_car(ptr_car_p1, direct_p1, RED, info.xres_virtual);
  draw_car(ptr_car_p2, direct_p2, BLUE, info.xres_virtual);
  char opposite_direct;

  //for wait
  struct timeb tb; 
  time_t start;

  #ifdef DEBUG
  FILE* log = fopen("log", "w");
  unsigned short start_millisec, end_millisec;  
  #endif

  ftime(&tb);
  start = tb.time;  
  while(is_ready_p1 != 1 || is_ready_p2 != 1)
  {
      ftime(&tb);
      if(tb.time - start >= 5)
      {
        if( pthread_kill(tid_control, 17) != 0 || pthread_kill(tid_syncing, 17) != 0 )
        {
          #ifdef DEBUG
          fclose(log);
          #endif
          close(sockfd);
          munmap(ptr, map_size);
          close(fb);
          endwin();
          fprintf(stderr, "Error of working thread\n");
          return -1;
        }

        #ifdef DEBUG
        fclose(log);
        #endif
        close(sockfd);
        munmap(ptr, map_size);
        close(fb);
        endwin();
        return 3;
      }
      else
      {
        continue;
      }
  }

  pthread_mutex_unlock(&mutex);
  //ftime(&tb);
  //start_millisec = tb.millitm;
  while(work_flag)
  {
    pthread_mutex_lock(&mutex);
    // move first player's car 
    switch(direct_p1)
    {
      case UP:
      {
        opposite_direct = DOWN;
        break;
      }
      case DOWN:
      {
        opposite_direct = UP;
        break;
      }
      case LEFT:
      {
        opposite_direct = RIGHT;
        break;
      }
      case RIGHT:
      {
        opposite_direct = LEFT;
        break;
      }
    }
    if(direct_prev_p1 != opposite_direct)
    {
      delete_car(ptr_car_p1, direct_prev_p1, info.xres_virtual, background_color);
      *ptr_car_p1 = RED;
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
        break;
      }
      case DOWN:
      {
        opposite_direct = UP;
        break;
      }
      case LEFT:
      {
        opposite_direct = RIGHT;
        break;
      }
      case RIGHT:
      {
        opposite_direct = LEFT;
        break;
      }
    }
    if(direct_prev_p2 != opposite_direct)
    {
      delete_car(ptr_car_p2, direct_prev_p2, info.xres_virtual, background_color);
      *ptr_car_p2 = BLUE;
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
    
    #ifdef DEBUG
    ftime(&tb);
    fprintf(log, "time: %ld\n", tb.millitm - start_millisec);
    #endif
    //ftime(&tb);
    //usleep(62500 - ( 
    //            (tb.millitm >= start_millisec) ? tb.millitm - start_millisec : 1000 - start_millisec + tb.millitm
    //            )*1000);
    usleep(62500);
    #ifdef DEBUG
    ftime(&tb);
    start_millisec = tb.millitm;
    #endif
  }
  //close all
  if( pthread_join(tid_control, NULL) != 0 || pthread_kill(tid_syncing, 17) != 0 )
  {
    #ifdef DEBUG
    fclose(log);
    #endif
    close(sockfd);
    munmap(ptr, map_size);
    close(fb);
    endwin();
    fprintf(stderr, "Error of working thread\n");
    return -1;
  }

  #ifdef DEBUG
  fclose(log);
  #endif
  close(sockfd);
  munmap(ptr, map_size);
  close(fb);
  endwin();

  //print result of game
  printf("\033c\n\t*\t\t\t\t\t\t\t\t\t*\t\t*\n*\t\t\t\t*\t\t\t\t*\n\n\t*\t\t\t\t*\t\t\t\t\t\t*\n");
  if(who_lose[index_player] == 0 && who_lose[0] != who_lose[1])
    printf("\t\t\t\t\t\t\tYou win!\n");
  else
    printf("\t\t\t\t\t\t\tYou lose:(\n");
  return 0;
}
