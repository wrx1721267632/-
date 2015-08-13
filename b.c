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
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

//++++++++++++++++++++++++++++++++宏定义++++++++++++++++++++++++++++++++++++++++++++++++++++
#define MAX_TO 100
//最大发送长度
#define NO printf("\033[?25l");					
//隐藏光标
#define YES printf("\033[?25h");				
//显示光标
#define MOVEUP(x) printf("\033[%dA", (x))    	
//上移动光标

//+++++++++++++++++++++++++++++++函数定义++++++++++++++++++++++++++++++++++++++++++++++++++++
void Password(char *a, char *b);				
//输入密码
void output_data(int conn_fd);					
//获取服务器发来的数据
void get_data(char *input_buf);					
//消息的输入
void input_data(int conn_fd);					
//向服务器发送数据
char my_page(void);								
//登录页面
int my_denglu(int conn_fd);						
//登录使得函数
void my_zhuce(int conn_fd);						
//注册的函数
void my_delete(int conn_fd);
//管理员删除普通用户的函数
void my_guanli(int conn_fd);
//管理员登录函数
void my_jilu(int conn_fd);
//将聊天记录文件发送给服务器

//=================================================全局变量===============================
pthread_mutex_t mutex;
	//聊天记录文件互斥锁
int signal = 0;
	//发送接受线程结束的标志
char username_all[32];
	//登录人帐号
int type = 0;
	//聊天类型（0为无类型，1为私聊，2为群聊）
char filename_all[32];
	//发送文件的文件名
char filename_all_recive[32];
	//接收文件的文件名

//===================================================功能函数==================================
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

//====================================================获取密码=================================
void Password(char *a, char *b)
//a是输入密码前面的提示
//b时保存密码的数组
{
	NO; 
	char ch;
    int i=0,j;
    //将从键盘处获取的值存入数组中 
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
	   } else if(ch == 13){
	   //若和键盘获取回车键则退处输入
		   break;
	   } else if(ch > 32 && ch < 127){
	     if(i<20){
            b[i]=ch;
	        fflush(stdin);
	     	i++;
 	     }
	   }
	   //用星号掩盖键盘处的输入
       printf("\r%s",a);
	   for(j=0; j<i; j++)
		   printf("*");
	   for(j=0; j<20-i; j++)
		   printf(" ");
	}
	b[i] = '\0';
	printf("\n");
	YES;
}

//====================================================获取需要发送的信息=============================
void get_data(char *input_buf)
//input_ buf是保存输入数据的数组
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

//======================================================发送文件的函数=================================
//将标志和内容放在一起发送，收到特殊标志的信息后，特殊处理，将第一个空格后的文件内容取出
void send_file(int conn_fd)
//服务器端的套接字
{
	int fd;
	//需要传送的文件的文件描述符
	char a[100];
	//从文件中取出的信息
	char b[100];
	//标志和文件整合在一起
	char c[20] = "@###@";
	//发送文件的标志，用来告诉接收到的函数这是发送文件的，与文件名一起构成完整的标志
	int i, j;
	
	//整合特殊标志
	strcat(c, filename_all);
	j = strlen(c);
	c[j++] = ' ';
	fd = open(filename_all, O_RDONLY);
	while(i = read(fd, a, 100-j)){
		strcpy(b, c);
		strcat(b, a);
		send(conn_fd, b, MAX_TO,0);
		memset(b,0,sizeof(b));
		memset(a,0,sizeof(a));
		if(i < (100-j)){
			break;
		}
		//send(conn_fd, b, MAX_TO,0);
	}
	close(fd);
	printf("发送完毕!\n");
}

//=========================================================接收消息的线程函数========================
void output_data(int conn_fd)
{
    char recv_buf[100];
    //接收套接字传来的字符串
    time_t timep;
    //时间变量，提供当前时间
    char a[200];
    int fp;
    //聊天记录文件的描述符
    char buf[100];
    int i;
    int j;
    int fd;
    //接受文件的描述符
    pthread_t thid3;
    int len;
    
    while(1){
        if(recv(conn_fd, recv_buf, MAX_TO, 0) == 0) {
        	printf("服务器退出\n");
        	exit(0);
        	signal = 1;
        	//当服务器崩掉后提示并强制退出客户端
        }
        
        if(strncmp(recv_buf,"@###@",5) == 0){
        	for(j=0; (recv_buf[j] != ' ') ; j++){
        		buf[j] = recv_buf[j];
			}
			buf[j] = '\0';
        	fd = open(buf, O_WRONLY|O_APPEND|O_CREAT, 0777);
        	len = strlen(recv_buf);
        	len = len -j -1;
        	for(i = j+1; (i<100) && (recv_buf[i] != EOF); i++) {
        		a[i-j-1] = recv_buf[i];
        	}
        	write(fd, a, len);
        	close(fd);
        	continue;
        	//发现文件传送的特殊标志则进行特殊处理
        }
        
        strcpy(buf, recv_buf);
        for(i=0; buf[i] != ' '; i++);
        for(j=i+1; (recv_buf[j] != ' ') && (j<100) && (recv_buf[j] != '\0'); j++)
        {
        	buf[j-i-1] = recv_buf[j];
        }
        buf[j-i-1] = ' ';
        buf[j-i] = '\0';
        //buf是去掉前面提示来自谁的消息后，准确的消息内容，方便进行一些特殊处理
        
        if(strcmp(buf,"#file ") == 0){
        	signal = 3;
        	printf("%s\n", recv_buf);
        	for(i=j+1; recv_buf[i] != '\n'; i++){
        		filename_all_recive[i-j-1] = recv_buf[i];
        	}
        	filename_all_recive[i-j-1] = '\0';
        	printf("dou you want recive the file!(#y/#n)\n");
        	continue;
        	//当准确的内容消息是要发送文件的请求时特殊处理
        }            
        
       	if((strcmp(buf, "#y ") == 0) && (type == 1) && (signal == 3)){
       		signal = 0;
        	printf("对方以开始接收！\n");
        	strcpy(a,"接收到文件 ");
        	strcat(a, filename_all);
        	strcat(a,"\n");
        	send(conn_fd, a, MAX_TO,0);
        	pthread_create(&thid3, NULL, (void *) send_file,  (int *)conn_fd);
        	continue;
        	//#y为同意文件的发送，进行特殊处理
        }
        
        if(strcmp(recv_buf, "-exit\n") == 0) {
            break;
           	//当输入-exit则退出
        }
        
        if(strcmp(recv_buf, "#xiugai\n") == 0) {
            printf("userpassword: ");
            scanf("%s", recv_buf);
            send(conn_fd, recv_buf, MAX_TO,0);
            continue;
           	//当接受到#xiugai是为了修改密码进行特殊的处理
        }
        
        if(strcmp(recv_buf, "#exit\n") == 0) {
        	type = 0;
            send(conn_fd, "#exit\n", MAX_TO,0);
            continue;
            //#exit可退出群聊或者私聊
        }
        
        if(strcmp(recv_buf, "服务器： don,t online or don't have the person!\n") == 0) {
            signal = 2;
            type = 0;
            //当查询到无此用户名时，将不再进行私聊
        }  

        if(signal == 1){
            break;
            //为了接受和传出消息的两个线程函数同步退出而存在的标识
        }
        
        time (&timep); 
        printf("\r %s\n",recv_buf);
        printf("\033[01;34m \r      %s\n\033[0m",ctime(&timep));
        //这三句话时在接收到的消息后加上时间
        
        pthread_mutex_lock(&mutex);
		fp = open(username_all, O_WRONLY|O_CREAT|O_APPEND, 0777);
		sprintf(a, " %s\n   %s\n",recv_buf,ctime(&timep));
		write(fp, a, strlen(a));
		close(fp);
		pthread_mutex_unlock(&mutex);
		//保存聊天记录在文件中
    }
    pthread_exit(0);
}

//===================================================查找当前目录是否存在应有的文件========================
int find_file(char *buf)
{
	DIR *dir;
	struct dirent* ptr;

	if((dir = opendir("./")) == NULL){
		perror("opendir");
		return -1;
		//获取当前目录下的文件信息
	}
	
	while((ptr = readdir(dir)) != NULL){
		if(strcmp(ptr->d_name, buf) == 0){
			return 1;
		}
		//查找当前目录与输入文件名是否有相匹配的
	}
	return -1;
}

//========================================================消息输出的线程函数========================
void input_data(int conn_fd)
{
    char input_buf[100];
    //存储输出消息的数组
    int ch;
    char buf[32];
    //存储特殊消息的数组
    time_t timep;
    int j;
    int fp;
    //聊天记录的文件描述符
    char a[100];
    //存储接收到的文件重新存储后的新文件名
    int fd;
    //接收文件的文件描述符
    int i;
    
    while(1) {
        get_data(input_buf);    
        if(strncmp(input_buf, "#to ", 4) == 0){
        	type = 1;
        	for(j=4; input_buf[j] != '\n'; j++){
	        	buf[j-4] = input_buf[j];
	    	}
	    	buf[j-4] = '\0';
	    	//当输出的信息开始为#to 的时候，将其后发送的消息均看作私聊消息
        }
        
        if(strcmp(input_buf, "#all\n") == 0){
        	type = 2;
        	//若输入#all则将type改为群聊
        }
        
        if(strcmp(input_buf, "#exit\n") == 0){
        	type = 0;
        	//#exit为无类型
        }
        
        if((type == 1) && (strncmp(input_buf, "#file ",6) == 0)){
        	signal = 3;
        	//若消息开始为#file 则将signal改为文件类型
        	for(j=6; input_buf[j] != '\n'; j++){
	        	buf[j-6] = input_buf[j];
	    	}
	    	buf[j-6] = '\0';
	    	strcpy(filename_all, buf);
	    	//将要传出的文件的文件名赋给全局变量，当调用传送文件函数时方便打开文件
	    	
	    	//find_file的函数，是查找当前目录下名为buf的文件，若为-1则是没有，不为-1则是有
        	if(find_file(buf) > -1){
        		printf("等待对方同意！\n");
        		send(conn_fd, input_buf, MAX_TO,0);
        		//若存在要找的文件则将数据传送给服务器	
        	}
        	
        	else{
        		printf("don't find the file!\n");
        	}
        	continue;
        	//若不存在，则不传送数据
        }
       	
       	//若即是私聊类型又是传送文件的类型，且传送的数据为#y，就将本地文件中与该文件同名的文件清空
        if((type == 1) && (signal == 3) && (strcmp(input_buf, "#y\n") == 0)){
        	signal = 0;
        	strcpy(a, "@###@");
        	strcat(a, filename_all_recive);
        	fd = open(a, O_WRONLY|O_CREAT|O_TRUNC, 0777);
        	close(fd);
        }       
        
        if(type == 1){
        	MOVEUP(1);
        	time (&timep);
        	printf("\033[01;35m \rinput(to %s): %s\n      %s\033[0m", buf,input_buf,ctime(&timep));
        	send(conn_fd, input_buf, MAX_TO,0);
        	
        	//将聊天记录包存在聊天记录文件中
        	pthread_mutex_lock(&mutex);
			fp = open(username_all, O_WRONLY|O_CREAT|O_APPEND, 0777);
			sprintf(a, "input(to %s): %s\n   %s",buf,input_buf,ctime(&timep));
			write(fp, a, strlen(a));
			close(fp);
			pthread_mutex_unlock(&mutex);
			        
			if(strcmp(input_buf, "-exit\n") == 0) {
            	fd = open(username_all, O_RDONLY);
				while(1){
					i = read(fd, a, 99);
					send(conn_fd, a, 99,0);
					memset(a,0,sizeof(a));
					if(i != 99){				
						break;
					}								
				}
				close(fd);
            	break;
            	//退出时，将聊天记录文件传送给服务器端
        	}
        
        	usleep(10); 
        	continue;
        }
        
        if(type == 2){
        	MOVEUP(1);
        	time (&timep);
        	printf("\033[01;35m \rinput(to all): %s\n      %s\033[0m",input_buf,ctime(&timep));
        	send(conn_fd, input_buf, MAX_TO,0);
        	
        	//将聊天记录包存在聊天记录文件中
        	pthread_mutex_lock(&mutex);
			fp = open(username_all, O_WRONLY|O_CREAT|O_APPEND, 0777);
			sprintf(a, "input(to all): %s\n   %s",input_buf,ctime(&timep));
			write(fp, a, strlen(a));
			close(fp);
			pthread_mutex_unlock(&mutex);
			
			if(strcmp(input_buf, "-exit\n") == 0) {
            	fd = open(username_all, O_RDONLY);
				while(1){
					i = read(fd, a, 99);
					send(conn_fd, a, 99,0);
					memset(a,0,sizeof(a));
					if(i != 99){				
						break;
					}								
				}
				close(fd);
            	break;
            	//退出时，将聊天记录文件传送给服务器端
        	}
  
        	usleep(10); 
        	continue;
        }
        
        MOVEUP(1);
        time (&timep);
        printf("\033[01;35m \rinput: %s\n      %s\033[0m", input_buf,ctime(&timep));
        send(conn_fd, input_buf, MAX_TO,0);
    	usleep(10); 
        
        if(strcmp(input_buf, "-exit\n") == 0) {
          	fd = open(username_all, O_RDONLY);
			while(1){
				i = read(fd, a, 99);
				send(conn_fd, a, 99,0);
				memset(a,0,sizeof(a));
				if(i != 99){				
					break;
				}								
			}
			close(fd);
            break;
            //退出时，将聊天记录文件传送给服务器端
        }
        
        if(signal == 1){
            break;
        }
    }
    signal = 1;
    pthread_exit(0);
}

//============================================登陆界面====================================
char my_page(void)
{
	char ch;
	while(1){
		system("clear");
		printf("\t\t1.登录\n");
    	printf("\t\t2.注册\n");
    	printf("\t\t3.管理员\n");
    	printf("\t\t0.退出\n");
    	printf("请输入需要的操作序号: ");
    	scanf("%c",&ch);
    	getchar();
    	
    	if((ch == '1') || (ch == '2') || (ch == '3')|| (ch == '0')){
    		break;
    	}
    	printf("\n请正确输入！\n");
    	printf("输入任意键退出!\n");
    	getch();
    }
    return ch;
    //函数返回值为选则的操作序号
}

//===============================================选择登录后执行操作的函数=========================
int my_denglu(int conn_fd)
{
	char recv_buf[1024];
	//储存传送的密码及文件的数组
	int ret;
	pthread_t thid1;
	//发送数据线程函数第一个参数值
    pthread_t thid2;
    //发送数据线程函数第一个参数值
	
	send(conn_fd, "denglu", MAX_TO, 0);
	//若选择了登录，需发送给服务器端，告诉服务器自己的选项
	
    printf("username: ");
    get_data(recv_buf);
    strcpy(username_all, recv_buf);
    username_all[strlen(username_all)-1] = '\0';
    strcat(username_all,".txt");
    send(conn_fd, recv_buf, 32, 0);
    printf("userpassword: ");
    Password("userpassword: ",recv_buf);
    send(conn_fd, recv_buf,32, 0);
    //将登录的账号和密码发送给服务器端
    
    recv(conn_fd, recv_buf, 3, 0);
    printf("\ny/n:  %s", recv_buf);
    //若账号和密码正确则能接收到y，否则接受y
    
    //若帐号密码不正确则退出该函数，继续进入初始界面
    if(strcmp(recv_buf, "n\n") == 0) {
    	printf("输入任意键继续！\n");
    	getch();    		   
    	return 0;
    }
	
	//若登录成功则接收到welcome
    if((ret = recv(conn_fd,recv_buf, 100, 0)) < 0) {
    	printf("error 5!\n");
    	exit(1);
    }
   	printf("\033[01;32m       %s\033[0m\n",recv_buf);
   	
   	//执行发送数据的接受数据的线程函数
    pthread_create(&thid1, NULL, (void *) input_data,  (int *)conn_fd); 
    pthread_create(&thid2, NULL, (void *) output_data,  (int *)conn_fd);
    pthread_join(thid1, NULL);
    return thid1;
}

//=============================================管理员删除用户的操作的函数============================
void my_delete(int conn_fd)
{
	char recv_buf[1024];
	
	recv(conn_fd, recv_buf, MAX_TO, 0);
	//消除之前系统传送的无用的消息
	send(conn_fd, "delete",32,0);
	//告诉服务器，管理员选择删除用户的这一操作
	
	//删除成功与否，都由服务器端传送的数据而知
	while(1){
		printf("请输入要删除的用户名！\n");
		get_data(recv_buf);
		//recv_buf[strlen(recv_buf)-1] = '\0';
		send(conn_fd, recv_buf,32,0);
		if(strcmp(recv_buf, "-exit\n") == 0){
			return;
		}
		recv(conn_fd, recv_buf, MAX_TO, 0);
		printf("%s\n", recv_buf);
	}
}

//=============================================管理员查找用户聊天记录的函数==============================
void my_jilu(int conn_fd)
{
	char recv_buf[1024];
	//存储从键盘获取的数据的数组
	char a[32];
	//存储发送数据的数组
	int fd;
	//接收需要的聊天记录文件的文件描述符
	int j;
	
	send(conn_fd, "jilu",32,0);
	//告诉服务器，管理员选择删除用户的这一操作
	while(1){
		printf("请输入要查找记录的用户名！\n");
		get_data(recv_buf);
		recv_buf[strlen(recv_buf)-1] = '\0';
		strcpy(a,recv_buf);
		strcat(a, ".txt");
		send(conn_fd, a ,32,0);
		//将输入的帐号加上.txt发送给服务器，服务器就可以直接查看其目录下是否有该文件
		
		if(strcmp(recv_buf, "-exit") == 0){
			return ;
		}
		
		//若服务器未找到，则客户端收到don‘t find，否则收到find it
		recv(conn_fd, recv_buf, MAX_TO, 0);
		printf("%s\n",recv_buf);
		if(strcmp(recv_buf,"don't find!") == 0){
			continue;
		} else if( strcmp(recv_buf,"find it!") == 0){
			fd = open(a, O_WRONLY|O_CREAT|O_TRUNC, 0777);
        	close(fd);
        	//将本地与接受文件同名的文件清空
        	
        	//将文件信息写入服务器本地的文件内
        	fd = open(a, O_WRONLY|O_APPEND|O_CREAT, 0777);
        	while(1){
        		if(recv(conn_fd, recv_buf, 99, 0) != 99){
        			break;
        		}
        		j = strlen(recv_buf);    		
        		if( (write(fd, recv_buf, j)) != 99){
        			break;
        		}
        	}
        	close(fd);
        	printf("it's ok!\n");
		}
	}	
}

//===================================================管理员登录后操作选择界面===========================
void my_guanli(int conn_fd)
{
	char recv_buf[1024];
	//存储需要发送的帐号和密码信息的数组
	char ch;
	
	send(conn_fd, "guanli", MAX_TO, 0);
	//告诉服务器端，是管理员登陆的
	
    printf("username: ");
    get_data(recv_buf);
    send(conn_fd, recv_buf, 32, 0);
    printf("userpassword: ");
    Password("userpassword: ",recv_buf); 
    send(conn_fd, recv_buf,32, 0);
    
    //成功返回y，失败返回n
    recv(conn_fd, recv_buf, 3, 0);
    printf("\ny/n:  %s", recv_buf);
    
    if(strcmp(recv_buf, "n\n") == 0) {
    	printf("输入任意键继续！\n");
    	getch();    		   
    	exit(0);
    }

   	printf("\033[01;32m       welcome!\033[0m\n");
   	getch();
   	while(1){
   		system("clear");
   		printf("\t\t1.删除\n");
   		printf("\t\t2.记录\n");
   		printf("\t\t0.退出\n");
   		printf("请输入需要的操作序号: ");
    	scanf("%c",&ch);
    	getchar();
    	
    	if((ch == '1') || (ch == '2') || (ch == '0')){
    		break;
    	}
    	printf("\n请正确输入！\n");
    	printf("输入任意键继续!\n");
    	getch();
    }
    switch(ch){
    	    case '1':
    	    	my_delete(conn_fd);
    	    	return ;
    			break;
    	    case '2':
    	    	my_jilu(conn_fd);
    	    	return ;
    			break;
    	    case '0':
    	    	close(conn_fd);
    			exit(0);
    	    default:
    	    	printf("\nplease input a right number!\n");
    	    	break;
    }
}

//=======================================================选择注册后执行的函数=================================
void my_zhuce(int conn_fd)
{
	char recv_buf[1024];
	
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
    		printf("\n");
    		printf("\033[01;32m       %s\033[0m\n",recv_buf);
    		printf("请登录!\n");
    }
    printf("输入任意键继续！\n");
    getch();
}

//========================================================主函数=====================================
int main(int argc, char **argv)
{
    int i;
    char ch;
    int ret;
    int conn_fd;
    //服务器端的套接字
    struct sockaddr_in serv_addr;
    //套接字地址的结构体
    char recv_buf[1024]; 
    pthread_t thid1;
    //接收数据线程函数的返回值  

    //服务器地址的初始化
    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(4507);
    inet_aton("127.0.0.1",&serv_addr.sin_addr);
    
	//创建服务器端对应的套接字
    conn_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(conn_fd < 0) {
        exit(1);
    }
    
    //与服务器连接
    if(connect(conn_fd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) < 0) {
        puts(strerror(errno));
        exit(1);
    }
    
    //聊天记录文件的互斥锁
    pthread_mutex_init(&mutex, NULL);

    while(1){
        ch = my_page();
    	
    	switch(ch){
    	    case '1':
    	    	if((thid1 = my_denglu(conn_fd)) == 0){
    			    continue;
    			}
    			exit(0);
    			break;
    	    case '2':
    	    	my_zhuce(conn_fd);
    			break;
    		case '3':
    			my_guanli(conn_fd);
    			exit(0);
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
//=============================================================END====================================
