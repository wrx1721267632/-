/*************************************************************************
	> File Name: b.c
	> Author: 
	> Mail: 
	> Created Time: 2015年08月04日 星期二 19时15分05秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <termios.h>
#include <time.h> 

#define MAX_TO 100

#define NO printf("\033[?25l");
#define YES printf("\033[?25h");

#define MOVEUP(x) printf("\033[%dA", (x))    //上移动光标
#define MOVEDOWN(x) printf("\033[%dB", (x)) //下移动光标
#define MOVELEFT(y) printf("\033[%dD", (y))  //左移动光标
#define MOVERIGHT(y) printf("\033[%dC",(y))  //右移动光标

int signal = 0;

int getch(void) 
{ 
        struct termios tm, tm_old; 
        int fd = STDIN_FILENO, c; 
        if(tcgetattr(fd, &tm) < 0) 
                return -1; 
        tm_old = tm; 
        cfmakeraw(&tm); 
        if(tcsetattr(fd, TCSANOW, &tm) < 0) 
                return -1; 
        c = fgetc(stdin); 
        if(tcsetattr(fd, TCSANOW, &tm_old) < 0) 
                return -1; 
        return c; 
} 

void Password(char *a, char *b)
{
	NO; 
	char ch;
        int i=0,j;
     
	while(1)
	{
	   ch=getch();
	   if(ch == 127)
	   {
		if(i != 0){
		   fflush(stdin);
		   //__fpurge(stdin);
            //setbuf(stdin, NULL);
		   b[i-1]='\0';
	           i--;
	        }
	   }
	   else if(ch == 13)
		   break;
	   else if(ch > 32 && ch < 127)
	   {
	     if(i<20){
             b[i]=ch;
	         fflush(stdin);
 //            setbuf(stdin, NULL);
	     //__fpurge(stdin);
	     i++;
 	     }
	   }
      	   printf("\r%s",a);
	   for(j=0; j<i; j++)
		   printf("*");
		   //fflush(NULL);
	   for(j=0; j<20-i; j++)
		   printf(" ");
	   //fflush(stdin);
	}
	printf("\n");
	YES;
}

void output_data(int conn_fd)
{
    char recv_buf[100];
    time_t timep;
    
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
        if(strcmp(recv_buf, "#xiugai\n") == 0) {
            printf("userpassword: ");
            scanf("%s", recv_buf);
            //Password("userpassword: ", recv_buf);
            send(conn_fd, recv_buf, MAX_TO,0);
            continue;
        }
        if(strcmp(recv_buf, "#exit\n") == 0) {
            send(conn_fd, "#exit\n", MAX_TO,0);
            continue;
        }
        MOVEUP(1);
        time (&timep); 
        printf("\n %s\n",recv_buf);
        printf("\033[01;34m       %s\033[0m\n",ctime(&timep));
        printf("\033[01;35m input: \033[0m");
        fflush(NULL);
        //printf("      %s",ctime(&timep)); 
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
        if(c == ' '){
            continue;
        }
        if(c > 32 && c < 127){
            input_buf[i++] = c;
        }
    }
    input_buf[i++] = '\n';
    input_buf[i++] = '\0';
}



void input_data(int conn_fd)
{
    char input_buf[100];
    int ch;
    char buf[32];
    
    while(1) {
        printf("\033[01;35m input: \033[0m");
        get_data(input_buf);
        send(conn_fd, input_buf, MAX_TO,0);
        usleep(10); 
        if(strcmp(input_buf, "-exit\n") == 0) {
            break;
        }
        if(signal == 1){
            break;
        }
    }
    signal = 1;
    pthread_exit(0);
}

int main(int argc, char **argv)
{
    int i;
    char ch;
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
    	scanf("%c",&ch);
    	getchar();
    	if((ch != '1') && (ch != '2') && (ch != '0')){
    	    printf("\nplease input a right number!\n");
    	    continue;
    	}
    	
    	switch(ch){
    	    case '1':
    	        send(conn_fd, "denglu", MAX_TO, 0);
    		printf("username: ");
    		get_data(recv_buf);
    		send(conn_fd, recv_buf, 32, 0);
    		printf("userpassword: ");
    		Password("userpassword: ",recv_buf);
    		send(conn_fd, recv_buf,32, 0);
    		recv(conn_fd, recv_buf, 3, 0);
    		printf("\ny/n:  %s", recv_buf);
    		if(strcmp(recv_buf, "n\n") == 0) {
    		   printf("输入任意键继续！\n");
    		   getch();    		   
    		   continue;
    		}

    		if((ret = recv(conn_fd,recv_buf, 100, 0)) < 0) {
    		    printf("error 5!\n");
    		    exit(1);
    		}
    		
   		//puts(recv_buf);
   		printf("\033[01;32m       %s\033[0m\n",recv_buf);
    		pthread_create(&thid1, NULL, (void *) input_data,  (int *)conn_fd); 
    		pthread_create(&thid2, NULL, (void *) output_data,  (int *)conn_fd);
    		pthread_join(thid1, NULL);
    		exit(0);
    		break;
    	    case '2':
    	    	send(conn_fd, "zhuce", MAX_TO, 0);
    		printf("username: ");
    		get_data(recv_buf);
    		send(conn_fd, recv_buf, 32, 0);
    		printf("userpassword: ");
    		Password("userpassword: ",recv_buf);
    		send(conn_fd, recv_buf,32, 0);
    		recv(conn_fd, recv_buf, MAX_TO, 0);
    		printf("  %s\n", recv_buf);
    		if(strcmp(recv_buf, "\nhave tne person") == 0){
    		    printf("  %s\n", recv_buf);
    		} else if(strcmp(recv_buf, "\nzhuce sucess!") == 0){
    		    //recv(conn_fd,recv_buf, 17, 0);
    		    printf("\n");
    		    printf("\033[01;32m       %s\033[0m\n",recv_buf);
    		    //puts(recv_buf);
    		    printf("请登录!\n");
    		}
    		printf("输入任意键继续！\n");
    		getch();
    		break;
    	    case '0':
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

