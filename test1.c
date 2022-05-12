#include <time.h>
#include <stdio.h>

int main () {
   clock_t start_t, end_t, total_t;
   int i;

   start_t = clock();
   printf("Starting of the program, start_t = %ld\n", start_t);
    
   printf("Going to scan a big loop, start_t = %ld\n", clock());
   for(i=0; i< 10000000; i++) {
   }
   end_t = clock();
   printf("End of the big loop, end_t = %ld\n", end_t);
   
   total_t = (end_t - start_t);
   printf("Total time taken by CPU: %ld\n", (unsigned long)((((double)(end_t - start_t)) / ((double)CLOCKS_PER_SEC))*1000000));
   printf("Exiting of the program...%ld\n", clock());
   printf("%ld", CLOCKS_PER_SEC);
#define ADDR_A (301)
#define ADDR_B (30)
#if ADDR_A == 301
return 1;
#else
return 0;
#endif

}