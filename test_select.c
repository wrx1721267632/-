/*************************************************************************
	> File Name: test_select.c
	> Author: 
	> Mail: 
	> Created Time: 2015年08月03日 星期一 11时01分50秒
 ************************************************************************/

#include<stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

void display_time(const char *string)
{
    int seconds;

    seconds = time((time_t*)NULL);
    printf("%s , %d\n", string, seconds);
}

int main()
{
    fd_set readfds;
    struct timeval timeout;
    int ret;

    FD_ZERO(&readfds);
    FD_SET(0, &readfds);

    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    while(1) {
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
        display_time("before select\n");
        ret = select(1, &readfds, NULL, NULL, &timeout);
        display_time("after select\n");

        switch(ret) {
            case 0:
                printf("No data in ten seconds\n");
                exit(0);
                break;
            case -1:
                perror("select");
                exit(-1);
            break;
            default :
                getchar();
                printf("Data is available now.\n");
        }
    }

    return 0;
}
