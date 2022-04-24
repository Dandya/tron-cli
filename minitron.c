#include "functions.h"

int work_flag= 1;
char direct_prev_p1 = RIGHT;
char direct_new_p1 = RIGHT;
char direct_prev_p2 = LEFT;
char direct_new_p2= LEFT;

int main(int argc, char** argv)
{
  int sockfd;
  struct sockaddr_in opponent_addr;
  
  sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  
  opponent_addr.sin_family = AF_INET;
  opponent_addr.sin_port = htons(12345);
  opponent_addr.sin_addr.s_addr = inet_addr(argv[1]);

  bind(sockfd, (struct sockaddr*)&opponent_addr, sizeof(opponent_addr));
  
  pthread_t tid_control, tid_syncing;
  pthread_attr_t attr;
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

  if( pthread_attr_init(&attr) != 0 )
  {
    fprintf(stderr, "Error of init attr\n");
    return 1;
  }
  printf("All wait\n");
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
  printf("Threads created\n"); 
  while(work_flag)
  {
    usleep(62500);
    pthread_mutex_lock(&mutex);
    pthread_mutex_unlock(&mutex);
  }
  
  if( pthread_join(tid_control, NULL) != 0 || pthread_join(tid_syncing, NULL) != 0 )
  {
    fprintf(stderr, "Error of working thread\n");
  }
  close(sockfd);
  return 0;
}
