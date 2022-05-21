extern int work_flag;
extern int start_flag;
extern int need_answer;
extern char number_step;

struct args_keys
{
  int sockfd;
  char* ptr_direct;
  char* ptr_is_ready_player;
  struct sockaddr_in* ptr_p2_addr;
  pthread_mutex_t* ptr_mtx;
};

int get_local_ip(unsigned long addr_c)
{
    struct ifaddrs *ifaddr;
    unsigned long addr_s;
    int mask;
    if (getifaddrs(&ifaddr) == -1) 
    {
        perror("getifaddrs");
        return 0;
    }

    for (struct ifaddrs *ifa = ifaddr; ifa != NULL;
                ifa = ifa->ifa_next) 
    {
        if (ifa->ifa_addr == NULL)
            continue;

        if (ifa->ifa_addr->sa_family == AF_INET) 
        {
            addr_s = ((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr;
            if(addr_s != 0)
            {
                mask = ((struct sockaddr_in *)ifa->ifa_netmask)->sin_addr.s_addr; 
                if((addr_s&mask) == (addr_c&mask))
                {
                    return addr_s;
                }
            }
        }
    }
    return 0;
}

void control_thread_nsync(struct args_keys* args)
{
  int sockfd = args->sockfd;
  char* ptr_direct = args->ptr_direct;
  pthread_mutex_t* ptr_mtx = args->ptr_mtx;
  struct sockaddr* ptr_p2_addr = (struct sockaddr*)args->ptr_p2_addr;
  int len_sockaddr = sizeof(*ptr_p2_addr);
  char direction;
  
  direction = getchar();
  *(args->ptr_is_ready_player) = 1;
  sendto(sockfd, &direction, 1, 0, ptr_p2_addr, len_sockaddr);

  direction = 0;
  //wait start game
  while( start_flag != 1 )
  {
      usleep(1);
      sendto(sockfd, &direction, 1, 0, ptr_p2_addr, len_sockaddr);
  }

  while( direction != 'q' && work_flag )
  {
    direction = getchar();
    if(direction == UP || direction == DOWN || direction == LEFT || direction == RIGHT)
    {
      pthread_mutex_lock(ptr_mtx);
      *ptr_direct = direction; 
      sendto(sockfd, &direction, 1, 0, ptr_p2_addr, len_sockaddr);
      pthread_mutex_unlock(ptr_mtx);
    }
  }
  work_flag = 0;
}

void interaction_thread_nsync(struct args_keys* args)
{
  int sockfd = args->sockfd;
  char* ptr_direct = args->ptr_direct;
  pthread_mutex_t* ptr_mtx = args->ptr_mtx;
  struct sockaddr* ptr_p2_addr = (struct sockaddr*)args->ptr_p2_addr;
  int len_sockaddr = sizeof(*ptr_p2_addr);
  char direction;

  recvfrom(sockfd, &direction, 1, 0, ptr_p2_addr, &len_sockaddr);
  *(args->ptr_is_ready_player) = 1;
  
  //wait start game
  while(start_flag != 1)
  {
      usleep(1);
  }

  while(work_flag)
  {
    recvfrom(sockfd, &direction, 1, 0, ptr_p2_addr, &len_sockaddr);
    if(direction == UP || direction == DOWN || direction == LEFT || direction == RIGHT)
    {
      pthread_mutex_lock(ptr_mtx);
      *ptr_direct = direction; 
      pthread_mutex_unlock(ptr_mtx);
    }
  }
}

void control_thread_sync(struct args_keys* args)
{
  int sockfd = args->sockfd;
  char* ptr_direct = args->ptr_direct;
  pthread_mutex_t* ptr_mtx = args->ptr_mtx;
  struct sockaddr* ptr_p2_addr = (struct sockaddr*)args->ptr_p2_addr;
  int len_sockaddr = sizeof(*ptr_p2_addr);
  char direction = 0;
  
  getchar();
  *(args->ptr_is_ready_player) = 1;
  sendto(sockfd, &direction, 1, 0, ptr_p2_addr, len_sockaddr);

  direction = 0;
  //wait start game
  while( start_flag != 1 )
  {
      usleep(1);
      sendto(sockfd, &direction, 1, 0, ptr_p2_addr, len_sockaddr);
  }

  while( direction != 'q' && work_flag )
  {
    direction = getchar();
    if(direction == UP || direction == DOWN || direction == LEFT || direction == RIGHT)
    {
      pthread_mutex_lock(ptr_mtx);
      *ptr_direct = direction; 
      pthread_mutex_unlock(ptr_mtx);
    }
  }
  work_flag = 0;
}

void interaction_thread_sync(struct args_keys* args)
{
  int sockfd = args->sockfd;
  char* ptr_direct = args->ptr_direct;
  pthread_mutex_t* ptr_mtx = args->ptr_mtx;
  struct sockaddr* ptr_p2_addr = (struct sockaddr*)args->ptr_p2_addr;
  int len_sockaddr = sizeof(*ptr_p2_addr);
  char direction;

  recvfrom(sockfd, &direction, 1, 0, ptr_p2_addr, &len_sockaddr);
  *(args->ptr_is_ready_player) = 1;
  
  //wait start game
  while(start_flag != 1)
  {
      usleep(1);
  }

  while(work_flag)
  {
    recvfrom(sockfd, &direction, 1, 0, ptr_p2_addr, &len_sockaddr);
    direction -= number_step % 2;
    if((direction == UP || direction == DOWN 
                || direction == LEFT || direction == RIGHT) && need_answer)
    {
      *ptr_direct = direction;
      need_answer = 0;
    }
  }
}
