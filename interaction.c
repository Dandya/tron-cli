extern int work_flag;
extern char direct_prev_p1;
extern char direct_new_p1;
extern char direct_prev_p2;
extern char direct_new_p2;

struct args_keys
{
  int sockfd;
  struct sockaddr_in* ptr_p2_addr;
  pthread_mutex_t* ptr_mtx;
};

void control_thread(struct args_keys* args)
{
  int sockfd = args->sockfd;
  pthread_mutex_t* ptr_mtx = args->ptr_mtx;
  struct sockaddr_in* ptr_p2_addr = args->ptr_p2_addr;
  int len_sockaddr = sizeof(*ptr_p2_addr);
  char ch = 0;
  while( ch != 'q' && work_flag )
  {
    ch = getchar();
    if(ch == UP || ch == DOWN || ch == LEFT || ch == RIGHT)
    {
      pthread_mutex_lock(ptr_mtx);
      direct_prev_p1 = direct_new_p1;
      direct_new_p1 = ch; 
      sendto(sockfd, (const char*) &ch, 1, 0, (struct sockaddr*)ptr_p2_addr, len_sockaddr);
      //debug
      printf("I push: %c\n", direct_new_p1);    
      //end debub
      pthread_mutex_unlock(ptr_mtx);
    }
  }
  work_flag = 0;
}

int syncing_thread(struct args_keys* args)
{
  int sockfd = args->sockfd;
  pthread_mutex_t* ptr_mtx = args->ptr_mtx;
  struct sockaddr_in* ptr_p2_addr = args->ptr_p2_addr;
  int len_sockaddr = sizeof(*ptr_p2_addr);
  char direction;
  while(work_flag)
  {
    recvfrom(sockfd, &direction, 1, 0, (struct sockaddr*) ptr_p2_addr, &len_sockaddr);
    pthread_mutex_lock(ptr_mtx);
    direct_prev_p2 = direct_new_p2;
    direct_new_p2 = direction;
    //debug
    printf("I get: %c\n", direct_new_p2);
    //end debug
    pthread_mutex_unlock(ptr_mtx);
  }
}
