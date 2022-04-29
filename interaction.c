extern int work_flag;

struct args_keys
{
  int sockfd;
  char* ptr_direct;
  struct sockaddr_in* ptr_p2_addr;
  pthread_mutex_t* ptr_mtx;
};

int get_local_ip(unsigned long addr_c)
{
    struct ifaddrs *ifaddr;
    int family;
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

        family = ifa->ifa_addr->sa_family;
        if (family == AF_INET || family == AF_INET6) 
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

void control_thread(struct args_keys* args)
{
  int sockfd = args->sockfd;
  char* ptr_direct = args->ptr_direct;
  pthread_mutex_t* ptr_mtx = args->ptr_mtx;
  struct sockaddr_in* ptr_p2_addr = args->ptr_p2_addr;
  int len_sockaddr = sizeof(*ptr_p2_addr);
  char direction;
  while( direction != 'q' && work_flag )
  {
    direction = getchar();
    if(direction == UP || direction == DOWN || direction == LEFT || direction == RIGHT)
    {
      pthread_mutex_lock(ptr_mtx);
      *ptr_direct = direction; 
      sendto(sockfd, &direction, 1, 0, (struct sockaddr*)ptr_p2_addr, len_sockaddr);
      //debug
//      printf("I push: %c, %ld\n", direct_new_p1, ptr_p2_addr->sin_addr.s_addr);    
      //end debub
      pthread_mutex_unlock(ptr_mtx);
    }
  }
  work_flag = 0;
}

int syncing_thread(struct args_keys* args)
{
  int sockfd = args->sockfd;
  char* ptr_direct = args->ptr_direct;
  pthread_mutex_t* ptr_mtx = args->ptr_mtx;
  struct sockaddr_in* ptr_p2_addr = args->ptr_p2_addr;
  int len_sockaddr = sizeof(*ptr_p2_addr);
  char direction;
  while(work_flag)
  {
    recvfrom(sockfd, &direction, 1, 0, (struct sockaddr*) ptr_p2_addr, &len_sockaddr);
    pthread_mutex_lock(ptr_mtx);
    *ptr_direct = direction;
    //debug
//    printf("I get: %c\n", direct_new_p2);
    //end debug
    pthread_mutex_unlock(ptr_mtx);
  }
}
