#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    if(argc != 2){
        printf("Please pass MBs of memory to be allocated. Exiting...\n");
        exit(EXIT_FAILURE);
    }
    int i, x = atoi(argv[1]);
    char *p;
    printf("Trying to allocate %dMB with this program\n", x);
    int y = x/5;
    x = x-(5*y);
    for(i = 0; i < y; ++i){
        p = malloc(5 * (1<<20));
        if(p == NULL){
            printf("Malloc failed at %d MB\n", (i+1)*5);
            return 0;
        }
        memset(p, 0, 5*(1<<20));
        printf("%dMB allocated\n", (i+1)*5);
    }

    p = malloc(x * (1<<20));
        if(p == NULL){
            printf("Malloc failed at %d MB\n", (i)*5+x);
            return 0;
        }
        memset(p, 0, x*(1<<20));
        printf("%dMB allocated\n", (i)*5+x);

    printf("Done!\n");
    return 0;
}