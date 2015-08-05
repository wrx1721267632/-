/*************************************************************************
	> File Name: my_client.c
	> Author: 
	> Mail: 
	> Created Time: 2015年08月03日 星期一 20时20分33秒
 ************************************************************************/

#include<stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

void output_data(int conn_fd)
{
    char recv_buf[32];
    while(1){
        recv(conn_fd, recv_buf, sizeof(recv_buf), 0);
        if(strcmp(recv_buf, "exit\n") == 0) {
            break;
        }
        printf("recive a data: %s",recv_buf);
    }
}

void get_data(char *input_buf)
{
    int i = 0;
    char c;
    while( i < 30) {
        c = getchar();
        if(c == '\n') {
            break;
        }
        input_buf[i++] = c;
    }
    input_buf[i++] = '\n';
    input_buf[i++] = '\0';
}

void input_data(int conn_fd)
{
    char input_buf[32];
    while(1) {
        get_data(input_buf);
        send(conn_fd, input_buf, sizeof(input_buf),0); 
        if(strcmp(input_buf, "exit\n") == 0) {
            break;
        }
    }
}

int main(int argc, char **argv)
{
    int i;
    int ret;
    int conn_fd;
    int serv_port;
    struct sockaddr_in serv_addr;
    char recv_buf[1024];
    pthread_t thid1;
    pthread_t thid2;


    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(4507);
    inet_aton("192.168.20.118",&serv_addr.sin_addr);
    
    conn_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(conn_fd < 0) {
        exit(1);
    }
    
    if(connect(conn_fd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) < 0) {
        puts(strerror(errno));
        exit(1);
    }

    if((ret = recv(conn_fd,(void *)recv_buf, 17, 0)) < 0) {
        printf("error 5!\n");
        exit(1);
    }
    puts(recv_buf);
    pthread_create(&thid1, NULL, (void *) input_data,  (int *)conn_fd); 
    pthread_create(&thid2, NULL, (void *) output_data,  (int *)conn_fd); 
    pthread_join(thid1, NULL);
    close(conn_fd);
    return 0;
}
