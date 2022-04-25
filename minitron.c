#include "functions.h"

/*
 * Problems:
 * 1) Как узнать собственный адрес?
 * TODO: 
 * 1) Проверка на размеры экрана (+2 пикселя для каждой координаты)
 * 2) Сравние IP
*/

int work_flag= 1;
char direct_prev_p1 = RIGHT;
char direct_new_p1 = RIGHT;
char direct_prev_p2 = LEFT;
char direct_new_p2= LEFT;

void handler(int none)
{
  work_flag = 0;
}

int main(int argc, char* argv[])
{ 
  printf("\033c");
  initscr();
  noecho();
  cbreak();
  curs_set(0);
  keypad(stdscr, TRUE);
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

  int sockfd;
  struct sockaddr_in opponent_addr, server_addr;
  
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  
  opponent_addr.sin_family = AF_INET;
  opponent_addr.sin_port = htons(12345);
  opponent_addr.sin_addr.s_addr = inet_addr(argv[1]);
 
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(12345);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
  
  pthread_t tid_control, tid_syncing;
  pthread_attr_t attr;
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

  if( pthread_attr_init(&attr) != 0 )
  {
    fprintf(stderr, "Error of init attr\n");
    return 1;
  }
//  printf("All wait\n");
  struct args_keys args = {sockfd, &opponent_addr, &mutex};

  if( pthread_create(&tid_control, &attr,(void *)control_thread, &args) != 0 )
  {
    fprintf(stderr, "Error of create thread\n");
    return 2;
  }

  if( pthread_create(&tid_syncing, &attr,(void *)syncing_thread, &args) != 0 )
  {
    fprintf(stderr, "Error of create thread\n");
    return 2;
  }
//  printf("Threads created\n"); 
  refresh();
  for(int i = 0; i<500+2; i++)
  {
      ptr[i] = 0x00FFFFFF;
      ptr[i+(300+1)*info.yres_virtual] = 0x00FFFFFF;
  }
  while(ptr[0] != 0x00FFFFFF)
      ptr[0] = 0x00FFFFFF;
  while(work_flag)
  { 
 // draw_area(ptr, 500, 300, info.xres_virtual, info.yres_virtual);
    usleep(62500);
    pthread_mutex_lock(&mutex);
    pthread_mutex_unlock(&mutex);
  }
  
  if( pthread_join(tid_control, NULL) != 0 || pthread_kill(tid_syncing, 17) != 0 )
  {
    fprintf(stderr, "Error of working thread\n");
  }

  close(sockfd);
  munmap(ptr, map_size);
  close(fb);
  endwin();
  return 0;
}
