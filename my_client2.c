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

#define NO printf("\033[?25l");              	//隐藏光标
#define YES printf("\033[?25h");				//显示光标
#define MOVEUP(x) printf("\033[%dA", (x))    	//上移动光标
#define SILIAO   1				//私聊
#define QUNLIAO  2				//群聊
#define DENGLU   3				//登录
#define ZHUCE 	 4              //注册
#define EXIT     5

int signal = 0; 			//关闭标记
void Password(char *a, char *b);
void output_data(int conn_fd);
void get_data(char *input_buf);
void input_data(int conn_fd);

struct stat
{
    int  type = 0;
    char To[32] = "\0";
    char Form[32] = "\0";
    char data[100];
}packet_data;

struct shangxian
{
    int  type = 0;
    char To[32] = "\0";
    char Form[32] = "\0";
    char username[32];
    char password[32];
}packet_t;

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

void Password(char *b)
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
	     i++;
 	     }
	   }
      	   printf("\rpassword: ",a);
	   for(j=0; j<i; j++)
		   printf("*");
	   for(j=0; j<20-i; j++)
		   printf(" ");
	}
	printf("\n");
	YES;
}

void output_data(int conn_fd)
{
    time_t timep;
    
    while(1){
        if(recv(conn_fd, packet_data, sizeof(struct stat), 0) == 0) {
        	printf("服务器退出\n");
        	exit(0);
        	signal = 1;
        }
        if(strcmp(packet_data.data, "-exit\n") == 0) {
            break;
        }
        if(strcmp(packet_data.data, "#xiugai\n") == 0) {
            printf("userpassword: ");
            scanf("%s", pasket_data.data);
            send(conn_fd, pasket_data.data, sizeof(struct stat),0);
            continue;
        }
        /*if(strcmp(pasket_data.data, "#exit\n") == 0) {
        	strcpy(pasket_data.data, "#exit\n");
            send(conn_fd, pasket_data.data, sizeof(struct stat),0);
            continue;
        }*/
        if(strcmp(pasket_data.data, "服务器： don,t online or don't have the person!\n") == 0) {
            signal = 2;
        }               
        if(signal == 1){
            break;
        }
        time (&timep); 
        printf("\r %s\n",pasket_data.data);
        printf("\033[01;34m \r      %s\033[0m\n",ctime(&timep));
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
    int ch;
    int i;
    char buf[32];
    time_t timep;
    
    packet_t.type = 0;
    while(1) {
        get_data(packet_data.data);
        MOVEUP(1);
        time (&timep);
        printf("\033[01;35m \rinput: %s\n      %s\033[0m", packet_data.data,ctime(&timep));
        if(strncmp(packet_data.data,"#to ",4) == 0){
        	packet_data.type = SILIAO;
        	for(i=3 ;packet_data.data[i] != '\n'; i++){
        		packet_data.To[i-4] =  packet_data.data[i];
        	}
        	packet_data.To[i] = '\0';
        }
        if(strcmp(packet_data.data,"#all\n") == 0){
        	packet_data.type = QUNLIAO;
        }
        if(strcmp(packet_data.data, "-exit\n") == 0) {
        	packet_data.type = EXIT;
            break;
        }
        if(packet_t.type == 0){
        	printf("请先输入正确命令!\n");
        	continue;
        }
        send(conn_fd, packet_data, sizeof(struct stat),0);
        usleep(10); 
        if(signal == 1){
            break;
        }
    }
    signal = 1;
    pthread_exit(0);
}

char my_page(void)
{
	char ch;
	while(1){
		system("clear");
		printf("\t\t1.登录\n");
    	printf("\t\t2.注册\n");
    	printf("\t\t0.退出\n");
    	printf("请输入需要的操作序号: ");
    	scanf("%c",&ch);
    	getchar();
    	if((ch == '1') && (ch == '2') && (ch == '0')){
    		break;
    	}
    	printf("\n请正确输入！\n");
    	printf("输入任意键退出!\n");
    	getch();
    }
    return ch;
}

int my_denglu()
{
	printf("username: ");
    get_data(packet_t.username);
    packet_t.type = DENGLU;
    printf("password: ");
    Password(packet_t.password);
    send(conn_fd, packet_t,sizeof(struct shangxian), 0);
    recv(conn_fd, packet_data, sizeof(struct stat), 0);
    printf("\ny/n:  %s",  packet_data.data);
    if(strcmp( packet_data.data, "n\n") == 0) {
    	printf("输入任意键继续！\n");
    	getch();    		   
   		return 0;
    }

    recv(conn_fd, packet_data, sizeof(struct stat), 0);		
   	printf("\033[01;32m       %s\033[0m\n",packet_data.data);
   	strcpy(packet_Form, packet_t.username)
   	pthread_create(&thid1, NULL, (void *) input_data,  (int *)conn_fd); 
    pthread_create(&thid2, NULL, (void *) output_data,  (int *)conn_fd);
    pthread_join(thid1, NULL);
    return 1;
}

void my_zhuce()
{
    printf("username: ");
    get_data(packet_t.username);
    packet_t.type = ZHUCE;
    printf("password: ");
    Password(packet_t.password);
    send(conn_fd, packet_t,sizeof(struct shangxian), 0);
    recv(conn_fd, packet_data, sizeof(struct stat), 0);
    printf("  %s\n", recv_buf);
    if(strcmp(packet_data.data, "\nhave tne person") == 0){
    	printf("  %s\n", recv_buf);
    } else if(strcmp(packet_data.data, "\nzhuce sucess!") == 0){
    	printf("\n");
    	printf("\033[01;32m       %s\033[0m\n",recv_buf);
    	printf("请登录!\n");
    }
    printf("输入任意键继续！\n");
    getch();
}

int main(int argc, char **argv)
{
    int i;
    char ch;
    int conn_fd;
    int serv_port;
    struct sockaddr_in serv_addr;
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
    	ch = my_page();
    	switch(ch){
    	    case '1':
    			if(my_denglu() == 0){
    			    continue;
    			}
    			exit(0);
    			break;
    	    case '2':
    	    	my_zhuce();
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

