/*************************************************************************
	> File Name: b.c
	> Author: 
	> Mail: 
	> Created Time: 2015年08月04日 星期二 19时15分05秒
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

#define MAX_TO 100

int signal = 0;

void output_data(int conn_fd)
{
    char recv_buf[100];
    while(1){
        if(recv(conn_fd, recv_buf, MAX_TO, 0) == 0) {
        	printf("服务器退出\n");
        	exit(0);
        	signal = 1;
        }
        //usleep(10);
        //printf("recive a data: %s\n",recv_buf);
        if(strcmp(recv_buf, "-exit\n") == 0) {
            break;
        }
        if(strcmp(recv_buf, "服务器： don,t online or don't have the person!\n") == 0) {
            signal = 2;
        }
        
        if(signal == 1){
            break;
        }
        printf("\r %s\n",recv_buf);
        //fflush(NULL);  //可以输出缓存区的数据
    }
    pthread_exit(0);
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
    char input_buf[100];
    int ch;
    char buf[32];
    
    /*usleep(20);
    while(1){
    //system("clear");
    printf("\t\t1.私聊\n");
    printf("\t\t2.群聊\n");
    //printf("\t\t0.退出\n");
    printf("请输入需要的操作序号: ");
    scanf("%d",&ch);
    getchar();
    	//setbuf(stdin,NULL);
    if((ch != 1) && (ch != 2) ){
    	printf("\nplease input a right number!\n");
    	continue;
    }
    break;
    }*/
    while(1) {
        /*if(ch == 1){
            strcpy(input_buf,"to ");
            printf("please input your friendsname: ");           
            get_data(buf);
            strcat(input_buf,buf);
            send(conn_fd, input_buf, MAX_TO,0);
            ch = -1;
            usleep(30);
            if(signal == 2){
                ch = 1;  
                signal = 0;
       	    }
       	    continue;
        }
         if(ch == 2){
            strcpy(input_buf,"all\n");           
            send(conn_fd, input_buf, MAX_TO,0);
            ch = -1;
            continue;
        }*/
    	//printf("input : ");
        get_data(input_buf);
        send(conn_fd, input_buf, MAX_TO,0);
        usleep(10); 
        if(strcmp(input_buf, "-exit\n") == 0) {
            break;
        }
        if(signal == 1){
            break;
        }
        /*if((strcmp(input_buf, "exit\n") == 0) ){
            while(1){
    		//system("clear");
    		printf("\t\t1.私聊\n");
    		printf("\t\t2.群聊\n");
    		//printf("\t\t0.退出\n");
    		printf("请输入需要的操作序号: ");
    		scanf("%d",&ch);
    		getchar();
    	//setbuf(stdin,NULL);
    		if((ch != 1) && (ch != 2)){
    			printf("\nplease input a right number!\n");
    			continue;
    		}
    		break;
    	    }
        }*/ 
    }
    signal = 1;
    pthread_exit(0);
}

int main(int argc, char **argv)
{
    int i;
    int ch;
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
    inet_aton("127.0.0.1",&serv_addr.sin_addr);
    
    conn_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(conn_fd < 0) {
        exit(1);
    }
    
    if(connect(conn_fd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) < 0) {
        puts(strerror(errno));
        exit(1);
    }

    while(1){
        system("clear");
    	printf("\t\t1.登录\n");
    	printf("\t\t2.注册\n");
    	printf("\t\t0.退出\n");
    	printf("请输入需要的操作序号: ");
    	scanf("%d",&ch);
    	getchar();
    	//setbuf(stdin,NULL);
    	if((ch != 1) && (ch != 2) && (ch != 0)){
    	    printf("\nplease input a right number!\n");
    	    continue;
    	}
    	
    	switch(ch){
    	    case 1:
    	        send(conn_fd, "denglu", MAX_TO, 0);
    		printf("username: ");
    		get_data(recv_buf);
    		send(conn_fd, recv_buf, 32, 0);
    		printf("userpassword: ");
    		get_data(recv_buf);
    		send(conn_fd, recv_buf,32, 0);
//getchar();
    		recv(conn_fd, recv_buf, 3, 0);
    		printf("y/n:  %s", recv_buf);
    		if(strcmp(recv_buf, "n\n") == 0) {
    		   printf("please wait for 3 seconds!\n");
    		   sleep(3);
    		   continue;
    		}
//printf("aaa\n");
    		if((ret = recv(conn_fd,recv_buf, 100, 0)) < 0) {
    		    printf("error 5!\n");
    		    exit(1);
    		}
    		
    		/*system("clear");
    		printf("\t\t1.私聊\n");
    		printf("\t\t2.群聊\n");
    		printf("\t\t0.退出\n");
    		printf("请输入需要的操作序号: ");*/
   		puts(recv_buf);
//return 0;
    		pthread_create(&thid1, NULL, (void *) input_data,  (int *)conn_fd); 
    		pthread_create(&thid2, NULL, (void *) output_data,  (int *)conn_fd);
    		pthread_join(thid1, NULL);
    		exit(0);
    		break;
    	    case 2:
    	    	send(conn_fd, "zhuce", MAX_TO, 0);
    		printf("username: ");
    		get_data(recv_buf);
    		send(conn_fd, recv_buf, 32, 0);
    		printf("userpassword: ");
    		get_data(recv_buf);
    		send(conn_fd, recv_buf,32, 0);
    		recv(conn_fd, recv_buf, MAX_TO, 0);
    		printf("  %s\n", recv_buf);
    		if(strcmp(recv_buf, "have tne person") == 0){
    		    printf("  %s\n", recv_buf);
    		} else if(strcmp(recv_buf, "zhuce sucess!") == 0){
    		    //recv(conn_fd,recv_buf, 17, 0);
    		    puts(recv_buf);
    		    printf("请登录!\n");
    		    //pthread_create(&thid1, NULL, (void *) input_data,  (int *)conn_fd); 
    		    //pthread_create(&thid2, NULL, (void *) output_data,  (int *)conn_fd);
    		    //pthread_join(thid1, NULL);
    		}
    		sleep(3);
    		break;
    	    case 0:
    	    	close(conn_fd);
    		return 0;
    	    default:
    	    	printf("\nplease input a right number!\n");
    	    	break;
    	}
    } 
    pthread_join(thid1, NULL);
    close(conn_fd);
    return 0;
}

