/////////////////////////////////////////////////////////////////////// 
// File Name : srv.c // 
// Date : 2014/06/13 // 
// Os : Ubuntu 12.04 LTS 64bits // 
// Author : Lee Jae Hae // 
// Student ID : 2009720151 // 
// ----------------------------------------------------------------- // 
// Title : System Programming Assignment #3 // 
// Description :  // 
///////////////////////////////////////////////////////////////////////
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <fcntl.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>

#define BUF_MAX_SIZE 8192
#define FLAGS (O_RDWR | O_CREAT | O_TRUNC)
#define ASCII_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define BIN_MODE (ASCII_MODE | S_IXUSR | S_IXGRP | S_IXOTH)

void command(char buf[BUF_MAX_SIZE]);//command check
void NLST(char buf[BUF_MAX_SIZE], char buf2[BUF_MAX_SIZE]);//ls and dir command
void PWD(char buf2[BUF_MAX_SIZE]);//pwd command
void CWD(char buf[BUF_MAX_SIZE]);//cd command
void CDUP();//cd .. command
void MKD(char buf[BUF_MAX_SIZE]);//mkdir command
void DELE(char buf[BUF_MAX_SIZE]);//delete command
void RMD(char buf[BUF_MAX_SIZE]);//redir command
void RNFR_RNTO(char buf[BUF_MAX_SIZE], char *dest);//rename command
int make_argv(char buf[BUF_MAX_SIZE], char argv[1024][128]);//make arguments
void ch_binary();
void ch_ascii();
void RETR(char buf[BUF_MAX_SIZE]);
void STOR(char buf[BUF_MAX_SIZE]);
void sort(int argc, char argv[1024][128]);//sort function
void uitoa(unsigned int in, char *output);//in->output
void convert_str_to_addr(char *str, char ip[25], unsigned int *port);
void log_infor(char ip[25], int port, char id[32]);

void sh_chld(int); // signal handler for SIGCHLD 
void sh_alrm(int); // signal handler for SIGALRM 
void sh_int(int); // signal handler for SIGINT

int id_match(char *id);
int pass_match(char *id, char *pass);

mode_t cur_mode;

pid_t PID;
struct timeval start;

int client_fd; 

int fd_log, fd_trans;
char reply[1024];

struct sockaddr_in client_addr; 
char id[32];

int main(int argc, char **argv) 
{ 
	char buff[BUF_MAX_SIZE], a[BUF_MAX_SIZE]; 
	char pass[32]; 
	int n, i; 
	struct sockaddr_in server_addr; 
	int server_fd;

	char host_ip[25];
	char add_temp[25];
	unsigned int port_num;
	int sockfd;
	struct sockaddr_in serv_addr;

	int len; 
	int port;

	time_t timer;
	struct tm *t;
	char t_t[16];
	char tt[32];

	int fp;
	char *token, *temp;

	int fp_checkIP; // FILE stream to check client’s IP
	int access=0;

	char *IP;
	char checkIP[16];

	char _IP[4][4];
	char _checkIP[4][4];
	int che=0;

	signal(SIGALRM, sh_alrm);
	signal(SIGCHLD, sh_chld);
	signal(SIGINT, sh_int);

	cur_mode=BIN_MODE;

	fd_log=open("logfile", FLAGS);

	memset(tt, 0, 32);
	timer=time(NULL);
	t=localtime(&timer);
	gettimeofday(&start, NULL);

	switch(t->tm_wday)
	{
		case 0: strcpy(tt, "SUN "); break;
		case 1: strcpy(tt, "MON "); break;
		case 2: strcpy(tt, "TUE "); break;
		case 3: strcpy(tt, "WED "); break;
		case 4: strcpy(tt, "THU "); break;
		case 5: strcpy(tt, "FIR "); break;
		case 6: strcpy(tt, "SAT "); break;
	}
	switch(t->tm_mon)
	{
		case 0: strcat(tt, "JAN "); break;
		case 1: strcat(tt, "FEB "); break;
		case 2: strcat(tt, "MAR "); break;
		case 3: strcat(tt, "APR "); break;
		case 4: strcat(tt, "MAY "); break;
		case 5: strcat(tt, "JUN "); break;
		case 6: strcat(tt, "JUL "); break;
		case 7: strcat(tt, "AUG "); break;
		case 8: strcat(tt, "SEP "); break;
		case 9: strcat(tt, "OCT "); break;
		case 10: strcat(tt, "NOV"); break;
		case 11: strcat(tt, "DEC"); break;
	}
	memset(t_t, 0, 16);
	uitoa((t->tm_mday)+1, t_t);
	strcat(tt, t_t);
	strcat(tt," ");
	memset(t_t, 0, 16);
	uitoa(t->tm_hour, t_t);
	strcat(tt, t_t);
	strcat(tt,":");
	memset(t_t, 0, 16);
	uitoa(t->tm_min, t_t);
	strcat(tt, t_t);
	strcat(tt,":");
	memset(t_t, 0, 16);
	uitoa(t->tm_sec, t_t);
	strcat(tt, t_t);
	strcat(tt, " ");
	memset(t_t, 0, 16);
	uitoa((t->tm_year)+1900, t_t);
	strcat(tt, t_t);

	write(fd_log, tt, strlen(tt));
	write(fd_log, "Server is started\n", 18);

	server_fd = socket(PF_INET, SOCK_STREAM, 0);
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family=AF_INET;
	server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	server_addr.sin_port=htons(atoi(argv[1]));
	bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	listen(server_fd, 15);

	while(1)
	{
		pid_t pid;
		len = sizeof(client_addr);
		client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &len);
		
		if((PID = fork()) == 0) //child process
		{
			fp_checkIP = open("access.txt", O_RDONLY);
			memset(buff, 0, sizeof(buff));
			len=read(fp_checkIP, buff, BUF_MAX_SIZE);

			IP = inet_ntoa(client_addr.sin_addr);//printf("%s\n",IP);
			temp = strtok(IP, ".");
			strcpy(_IP[0], temp);
			temp = strtok(NULL, ".");
			strcpy(_IP[1], temp);
			temp = strtok(NULL, ".");
			strcpy(_IP[2], temp);
			temp = strtok(NULL, "\n");
			strcpy(_IP[3], temp);

			while(access == 0)
			{
				memset(_checkIP[0], 0, sizeof(_checkIP[0]));
				memset(_checkIP[1], 0, sizeof(_checkIP[1]));
				memset(_checkIP[2], 0, sizeof(_checkIP[2]));
				memset(_checkIP[3], 0, sizeof(_checkIP[3]));
				if(che == 0)
				{
					temp = strtok(buff, ".");
					che++;
				}
				else
					temp = strtok(NULL, ".");

				if(!temp)
					break;

				strcpy(_checkIP[0], temp);
				temp = strtok(NULL, ".");
				strcpy(_checkIP[1], temp);
				temp = strtok(NULL, ".");
				strcpy(_checkIP[2], temp);
				temp = strtok(NULL, "\n");
				strcpy(_checkIP[3], temp);

				for(i=0; i<4; i++)
				{
					if(strcmp(_IP[i], _checkIP[i])!=0)
						break;
				}
	
				if(i==4)
					i=3;
	
				if(strcmp(_checkIP[i], "*")==0 || strcmp(_IP[i], _checkIP[i])==0)
					access=1;
			}

			close(fp_checkIP);

			if(access==1)
				write(client_fd, "ok", 2);
			else
				write(client_fd, "no", 2);


			memset(buff, 0, sizeof(buff));
			strcpy(buff, "220 ");

			timer=time(NULL);
			t=localtime(&timer);

			fp=open("motd", O_RDONLY);
			len=read(fp, a, BUF_MAX_SIZE);

			i=0;
			while(a[len-i]!=')')
			{
				a[len-i]='\0';
				i++;
			}
			a[len-i]='\0';
			strcat(buff, a);
			
			switch(t->tm_wday)
			{
				case 0: strcat(buff, " SUN "); break;
				case 1: strcat(buff, " MON "); break;
				case 2: strcat(buff, " TUE "); break;
				case 3: strcat(buff, " WED "); break;
				case 4: strcat(buff, " THU "); break;
				case 5: strcat(buff, " FIR "); break;
				case 6: strcat(buff, " SAT "); break;
			}
			switch(t->tm_mon)
			{
				case 0: strcat(buff, "JAN "); break;
				case 1: strcat(buff, "FEB "); break;
				case 2: strcat(buff, "MAR "); break;
				case 3: strcat(buff, "APR "); break;
				case 4: strcat(buff, "MAY "); break;
				case 5: strcat(buff, "JUN "); break;
				case 6: strcat(buff, "JUL "); break;
				case 7: strcat(buff, "AUG "); break;
				case 8: strcat(buff, "SEP "); break;
				case 9: strcat(buff, "OCT "); break;
				case 10: strcat(buff, "NOV"); break;
				case 11: strcat(buff, "DEC"); break;
			}
			memset(t_t, 0, 16);
			uitoa((t->tm_mday)+1, t_t);
			strcat(buff, t_t);
			strcat(buff," ");
			memset(t_t, 0, 16);
			uitoa(t->tm_hour, t_t);
			strcat(buff, t_t);
			strcat(buff,":");
			memset(t_t, 0, 16);
			uitoa(t->tm_min, t_t);
			strcat(buff, t_t);
			strcat(buff,":");
			memset(t_t, 0, 16);
			uitoa(t->tm_sec, t_t);
			strcat(buff, t_t);
			strcat(buff," KST ");
			memset(t_t, 0, 16);
			uitoa((t->tm_year)+1900, t_t);
			strcat(buff, t_t);
			strcat(buff,") ready.\n");

			write(client_fd, buff, BUF_MAX_SIZE);

			close(fp);

			i=0;
			while(access&&i<3)
			{
				memset(buff, 0, sizeof(buff));
				n=read(client_fd, buff, BUF_MAX_SIZE);
				memset(id, 0, sizeof(id));
				strcpy(id, buff);

				memset(buff, 0, sizeof(buff));
				if(id_match(id)==1)
				{
					strcpy(buff, "331 Password required for sslab.");
					write(client_fd, buff, BUF_MAX_SIZE);

					memset(buff, 0, sizeof(buff));
					n=read(client_fd, buff, BUF_MAX_SIZE);
					memset(pass, 0, sizeof(pass));
					strcpy(pass, buff);

					memset(buff, 0, sizeof(buff));
					if(pass_match(id, pass)==1)
					{
						strcpy(buff, "230 User sslab logged in.");
						write(client_fd, buff, BUF_MAX_SIZE);
						log_infor(inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), id);
						write(fd_log, "FOG_IN\n", 7);

						break;
					}
					else
					{
						strcpy(buff, "530 Not logged in.");
						write(client_fd, buff, BUF_MAX_SIZE);
						log_infor(inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), id);
						write(fd_log, "FOG_FAIL\n", 9);i++;
					}
				}
				else
				{
					strcpy(buff, "530 Not logged in.");
					write(client_fd, buff, BUF_MAX_SIZE);
					log_infor(inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), id);
						write(fd_log, "FOG_FAIL\n", 9);i++;
				}
			}

			if(i==3)
				kill(getpid(), SIGINT);

			while(access)
			{
				memset(buff, 0, sizeof(buff));
				n=read(client_fd, buff, BUF_MAX_SIZE);

				if(strstr(buff, "NLST") || strstr(buff, "RETR") || strstr(buff, "STOR"))
				{
					read(client_fd, add_temp, 25);

					convert_str_to_addr(add_temp, host_ip, (unsigned int *) &port_num);
//command connection을 통해서 받은 정보를 주소 추출

					sockfd = socket(AF_INET, SOCK_STREAM,0);
					memset(&serv_addr, 0, sizeof(serv_addr));
					serv_addr.sin_family=AF_INET;
					serv_addr.sin_addr.s_addr=inet_addr(host_ip);
					serv_addr.sin_port=htons(port_num);

//data connection
					connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr));

					memset(reply, 0, sizeof(reply));
					strcpy(reply, "200 PORT command successful.\n");

					if(strstr(buff, "NLST"))
						strcat(reply, "150 Opening ASCII mode data connection.\n");
					if(strstr(buff, "RETR"))
						strcat(reply, "150 Opening BINARY mode data connection for exam1 (24644 bytes)\n");
					if(strstr(buff, "STOR"))
						strcat(reply, "150 150 Opening BINARY mode data connection for vhdl.\n");

					command(buff);

					n=strlen(buff);

					memset(a, 0, sizeof(a));
					strcpy(a, reply);
					strcat(a, buff);
					strcat(a, "226 Transfer complete.\n OK. ");
					memset(add_temp, 0, sizeof(add_temp));
					uitoa(n, add_temp);
					strcat(a, add_temp);
					strcat(a, " bytes received.\n");
					memset(buff, 0, sizeof(buff));
					strcpy(buff, a);

					write(sockfd, buff, BUF_MAX_SIZE);
					
					close(sockfd);
				}
				else
				{
					command(buff);

				if(strncmp(buff, "QUIT", 4) == 0)
					kill(getpid(), SIGINT);
				
				n=strlen(buff);
				write(client_fd, buff, BUF_MAX_SIZE);
				}

			}
		}
		else //parent process
		{
			
		}

		close(client_fd);
	}
	
	return 0;
}

void sh_chld(int signum) {
	

}

void sh_alrm(int signum) {
	
}

void sh_int(int signum) {
	time_t timer;
	struct tm *t;
	char t_t[16];
	char tt[32];

	int n;

	struct timeval now;

	if(PID==0)
	{
		gettimeofday(&now, NULL);

		n=(int)now.tv_sec - (int)start.tv_sec;
		memset(t_t, 0, 16);
		uitoa(n, t_t);

		log_infor(inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), id );
		write(fd_log, " LOG_OUT\n", 9);
		write(fd_log, "[total service time : ", 22);
		write(fd_log, t_t, strlen(t_t));
		write(fd_log, " sec]\n", 6);
	}
	else
	{
	memset(tt, 0, 32);
	timer=time(NULL);
	t=localtime(&timer);

	switch(t->tm_wday)
	{
		case 0: strcpy(tt, "SUN "); break;
		case 1: strcpy(tt, "MON "); break;
		case 2: strcpy(tt, "TUE "); break;
		case 3: strcpy(tt, "WED "); break;
		case 4: strcpy(tt, "THU "); break;
		case 5: strcpy(tt, "FIR "); break;
		case 6: strcpy(tt, "SAT "); break;
	}
	switch(t->tm_mon)
	{
		case 0: strcat(tt, "JAN "); break;
		case 1: strcat(tt, "FEB "); break;
		case 2: strcat(tt, "MAR "); break;
		case 3: strcat(tt, "APR "); break;
		case 4: strcat(tt, "MAY "); break;
		case 5: strcat(tt, "JUN "); break;
		case 6: strcat(tt, "JUL "); break;
		case 7: strcat(tt, "AUG "); break;
		case 8: strcat(tt, "SEP "); break;
		case 9: strcat(tt, "OCT "); break;
		case 10: strcat(tt, "NOV"); break;
		case 11: strcat(tt, "DEC"); break;
	}
	memset(t_t, 0, 16);
	uitoa((t->tm_mday)+1, t_t);
	strcat(tt, t_t);
	strcat(tt," ");
	memset(t_t, 0, 16);
	uitoa(t->tm_hour, t_t);
	strcat(tt, t_t);
	strcat(tt,":");
	memset(t_t, 0, 16);
	uitoa(t->tm_min, t_t);
	strcat(tt, t_t);
	strcat(tt,":");
	memset(t_t, 0, 16);
	uitoa(t->tm_sec, t_t);
	strcat(tt, t_t);
	strcat(tt, " ");
	memset(t_t, 0, 16);
	uitoa((t->tm_year)+1900, t_t);
	strcat(tt, t_t);

	write(fd_log, tt, strlen(tt));
	write(fd_log, " Server is terminated\n", 22);
	}

	close(fd_log);
	exit(1);
}

int id_match(char *id)
{
	FILE *fp;
	struct passwd *pw;

	fp = fopen("passwd", "r");

	while(pw = fgetpwent(fp))
	{
		if(strcmp(id, pw->pw_name)==0)
		{
			fclose(fp);
			return 1;
		}
	}

	fclose(fp);
	return 0;
}

int pass_match(char *id, char *pass)
{
	FILE *fp;
	struct passwd *pw;

	fp = fopen("passwd", "r");

	while(pw = fgetpwent(fp))
	{
		if(strcmp(id, pw->pw_name)==0 && strcmp(pass, pw->pw_passwd)==0)
		{
			fclose(fp);
			return 1;
		}
	}

	fclose(fp);
	return 0;
}

void command(char buf[BUF_MAX_SIZE])
{
	int ck=0;
	int n;
	char *token, *temp;
	char buf2[BUF_MAX_SIZE];

	memset(buf2, 0, sizeof(buf2));

	token=strtok(buf, "\n");
	while(token)
	{
		if(strncmp(token, "NLST", 4)==0)
		{
			NLST(buf, buf2);
		}
		else if(strncmp(token, "NIST", 4)==0)
		{
			NLST(buf, buf2);
		}
		else if(strncmp(token, "PWD", 3)==0) 
		{
			PWD(buf2);strcat(buf2, reply);
		}
		else if(strncmp(token, "CWD", 3)==0) 
		{
			CWD(buf);strcat(buf2, reply);
		}
		else if(strncmp(token, "CDUP", 4)==0) 
		{
			CDUP();strcat(buf2, reply);
		}
		else if(strncmp(token, "MKD", 3)==0) 
		{
			MKD(buf);strcat(buf2, reply);
		}
		else if(strncmp(token, "DELE", 4)==0) 
		{
			DELE(buf);strcat(buf2, reply);
		}
		else if(strncmp(token, "RMD", 3)==0) 
		{
			RMD(buf);strcat(buf2, reply);
		}
		else if(strncmp(token, "RNFR", 4)==0)
		{
			temp=strtok(NULL, "\n");
			RNFR_RNTO(token, temp);strcat(buf2, reply);
		}
		else if(strncmp(token, "QUIT", 4)==0)
		{
			strcpy(buf2, token);
			memset(reply, 0, sizeof(reply));
			strcpy(reply, "221 Goodbye.");
			strcat(buf2, reply);
		}
		else if(strncmp(token, "TYPE", 4)==0) 
		{
			temp=strtok(NULL, "\n");
			if(strcmp(token, "I")==0)
				ch_binary(buf);
			else
				ch_ascii(buf);

			strcat(buf2, reply);
		}
		else if(strncmp(token, "RETR", 4)==0) 
		{
			RETR(buf);
		}
		else if(strncmp(token, "STOR", 4)==0) 
		{
			STOR(buf);
		}
		else
		{
			n=strlen(token);
			if(buf2==NULL)
			{
				strcpy(buf2, token);
				strcat(buf2, "\n");
			}
			else
			
			{
				strcat(buf2, token);
				strcat(buf2, "\n");
			}
		}

		token=strtok(NULL, "\n");
	}

	memset(buf, 0, sizeof(buf));
	strcpy(buf, buf2);
	memset(buf2, 0, sizeof(buf2));

}

void NLST(char buf[BUF_MAX_SIZE], char buf2[BUF_MAX_SIZE])
{
	char temp[BUF_MAX_SIZE];
	char *token;
	char getStr[128], getStr_o[128];
	char argv[1024][128];
	char data[1024][128];
	char dir_data[1024][128];
	char output[8192];
	int i, j, k, n, argc, max, a;
	int row, remain, cnt, dir_cnt;
	int option_check=0;

	unsigned int ui_temp;

	struct passwd *uid;
	struct group *gid;

	struct tm *atime;

	struct dirent *d;
	DIR *dp;

	struct stat buf_stat;

	if(strncmp(buf, "LIST", 4)==0)
		option_check=3;

	strcpy(temp, buf);

	token=strtok(buf, "\t");
	
	while(token=strtok(NULL, "\t"))
	{
		if(strcmp(token, "-a")==0)
		{
			if(option_check==0)
				option_check=1;
			else
				option_check=3;
		}
		else if(strcmp(token, "-l")==0)
		{
			if(option_check==0)
				option_check=2;
			else
				option_check=3;
		}
		else if(strcmp(token, "-al")==0)
		{
			if(option_check==0)
				option_check=3;
		}
		else if(strcmp(token, "-la")==0)
		{
			if(option_check==0)
				option_check=3;
		}
	}

	strcpy(buf, temp);

	argc=make_argv(buf, argv);
	for(i=1; i<argc; i++)
	{
		if(argv[i][0]=='-')
		{
			for(j=i; j<argc-1; j++)
			{
				strcpy(argv[j], argv[j+1]);
			}

			argc--;
			i--;		
		}
	}

	getcwd(getStr_o, 128);
	
	dir_cnt=0; k=0; a=0;
while(1)
{
	memset(data, 0, sizeof(data));
	memset(output, 0, sizeof(output));
	if(option_check==0)//ls
	{
		cnt=0; i=0;

		if(argc==1)
		{
			getcwd(getStr, 128);
			dp=opendir(getStr);
			while (d = readdir(dp)) 
			{
				if(strncmp(d->d_name, ".", 1)!=0)
				{
					strcpy(data[i], d->d_name);
					stat(data[i], &buf_stat);
					if(S_ISDIR(buf_stat.st_mode))
						strcat(data[i], "/");
					cnt++; i++;
				}
			}
			closedir(dp);k++;
		}
		else
		{
			j=0;
			for(i=1; i<argc; i++)
			{
				stat(argv[i], &buf_stat);
				if(S_ISDIR(buf_stat.st_mode)){}
				else
				{
					strcpy(data[j], argv[i]);
					cnt++;j++;
				}
			}

			j=0;
			for(i=1; i<argc; i++)
			{
				stat(argv[i], &buf_stat);
				if(S_ISDIR(buf_stat.st_mode))
				{
					strcpy(dir_data[j], argv[i]);
					strcat(dir_data[j], "/");
					j++; dir_cnt++;
				}
			}

			k=cnt; max=cnt+dir_cnt;
		}

		sort(cnt, data);

		row=cnt/5;
		remain=cnt%5;
		if(remain!=0)
			row++;

		for(i=0; i<row; i++)
		{
			if(remain==0)
			{
				for(j=0; j<5; j++)
				{
					if(output==NULL)
						strcpy(output, data[j*row+i]);
					else
						strcat(output, data[j*row+i]);
					strcat(output, "\t");
				}
			}
			else if(i==row-1)
			{
				for(j=0; j<remain; j++)
				{
					if(output==NULL)
						strcpy(output, data[j*row+i]);
					else
						strcat(output, data[j*row+i]);
						strcat(output, "\t\t");
				}
			}
			else
			{
				for(j=0; j<=remain; j++)
				{
					if(output==NULL)
						strcpy(output, data[j*row+i]);
					else
						strcat(output, data[j*row+i]);
					strcat(output, "\t\t");
				}
				for(; j<5; j++)
				{
					if(output==NULL)
						strcpy(output, data[j*(row-1)+i+remain]);
					else
						strcat(output, data[j*(row-1)+i+remain]);
					strcat(output, "\t\t");
				}
					strcat(output, " ");
			}
			
			if(data==NULL)
				strcpy(output, "\n");
			else
				strcat(output, "\n");
		}

		if(buf2==NULL)
			strcpy(buf2, output);
		else	
			strcat(buf2, output);
	}
	else if(option_check==1)//ls -a
	{
		cnt=0; i=0;

		if(argc==1)
		{
			getcwd(getStr, 128);
			dp=opendir(getStr);
			while (d = readdir(dp)) 
			{
				strcpy(data[i], d->d_name);
				stat(data[i], &buf_stat);
				if(S_ISDIR(buf_stat.st_mode))
					strcat(data[i], "/");
				cnt++; i++;
			}
			closedir(dp);k++;
		}
		else
		{
			j=0;
			for(i=1; i<argc; i++)
			{
				stat(argv[i], &buf_stat);
				if(S_ISDIR(buf_stat.st_mode)){}
				else
				{
					strcpy(data[j], argv[i]);
					cnt++;j++;
				}
			}

			j=0;
			for(i=1; i<argc; i++)
			{
				stat(argv[i], &buf_stat);
				if(S_ISDIR(buf_stat.st_mode))
				{
					strcpy(dir_data[j], argv[i]);
					strcat(dir_data[j], "/");
					j++; dir_cnt++;
				}
			}

			k=cnt; max=cnt+dir_cnt;
		}sort(cnt, data);
		row=cnt/5;

		remain=cnt%5;
		if(remain!=0)
			row++;

		for(i=0; i<row; i++)
		{
			if(remain==0)
			{
				for(j=0; j<5; j++)
				{
					if(output==NULL)
						strcpy(output, data[j*row+i]);
					else
						strcat(output, data[j*row+i]);
					strcat(output, "\t");
				}
			}
			else if(i==row-1)
			{
				for(j=0; j<remain; j++)
				{
					if(output==NULL)
						strcpy(output, data[j*row+i]);
					else
						strcat(output, data[j*row+i]);

						strcat(output, "\t\t");
				}
			}
			else
			{
				for(j=0; j<=remain; j++)
				{
					if(output==NULL)
						strcpy(output, data[j*row+i]);
					else
						strcat(output, data[j*row+i]);
					strcat(output, "\t\t");
				}
				for(; j<5; j++)
				{
					if(output==NULL)
						strcpy(output, data[j*(row-1)+i+remain]);
					else
						strcat(output, data[j*(row-1)+i+remain]);
					strcat(output, "\t\t");
				}

				strcat(output, " ");
			}
			
			if(data==NULL)
				strcpy(output, "\n");
			else
				strcat(output, "\n");
		}

		if(buf2==NULL)
			strcpy(buf2, output);
		else	
			strcat(buf2, output);
	}
	else if(option_check==2)//ls -l
	{
		cnt=0; i=0;

		if(argc==1)
		{
			getcwd(getStr, 128);
			dp=opendir(getStr);
			while (d = readdir(dp)) 
			{
				if(strncmp(d->d_name, ".", 1)!=0)
				{
					strcpy(data[i], d->d_name);
					cnt++; i++;
				}
			}
			closedir(dp);k++;
		}
		else
		{
			j=0;
			for(i=1; i<argc; i++)
			{
				stat(argv[i], &buf_stat);
				if(S_ISDIR(buf_stat.st_mode)){}
				else
				{
					strcpy(data[j], argv[i]);
					cnt++;j++;
				}
			}

			j=0;
			for(i=1; i<argc; i++)
			{
				stat(argv[i], &buf_stat);
				if(S_ISDIR(buf_stat.st_mode))
				{
					strcpy(dir_data[j], argv[i]);
					strcat(dir_data[j], "/");
					j++; dir_cnt++;
				}
			}

			k=cnt; max=cnt+dir_cnt;
		}

		sort(cnt, data);

		for(i=0; i<cnt; i++)
		{
			stat(data[i], &buf_stat);

			if(S_ISDIR(buf_stat.st_mode))
				strcpy(output, "d");
			else
				strcpy(output, "-");

			if(S_IRUSR & buf_stat.st_mode)
				strcat(output, "r");
			else
				strcat(output, "-");

			if(S_IWUSR & buf_stat.st_mode)
				strcat(output, "w");
			else
				strcat(output, "-");

			if(S_IXUSR & buf_stat.st_mode)
				strcat(output, "x");
			else
				strcat(output, "-");

			if(S_IRGRP & buf_stat.st_mode)
				strcat(output, "r");
			else
				strcat(output, "-");

			if(S_IWGRP & buf_stat.st_mode)
				strcat(output, "w");
			else
				strcat(output, "-");

			if(S_IXGRP & buf_stat.st_mode)
				strcat(output, "x");
			else
				strcat(output, "-");

			if(S_IROTH & buf_stat.st_mode)
				strcat(output, "r");
			else
				strcat(output, "-");

			if(S_IWOTH & buf_stat.st_mode)
				strcat(output, "w");
			else
				strcat(output, "-");

			if(S_IXOTH & buf_stat.st_mode)
				strcat(output, "x");
			else
				strcat(output, "-");

			strcat(output, "\t");

			ui_temp=(unsigned int)buf_stat.st_nlink;
			uitoa(ui_temp, output);
			strcat(output, "\t");

			uid=getpwuid(buf_stat.st_uid);
			strcat(output, uid->pw_name);
			strcat(output, "\t");

			gid=getgrgid(buf_stat.st_uid);
			strcat(output, gid->gr_name);
			strcat(output, "\t");

			ui_temp=(unsigned int)buf_stat.st_size;
			uitoa(ui_temp, output);
			strcat(output, "\t");

			atime=gmtime(&buf_stat.st_atime);
			switch(atime->tm_mon)
			{
				case 0: strcat(output, "JAN "); break;
				case 1: strcat(output, "FEB "); break;
				case 2: strcat(output, "MAR "); break;
				case 3: strcat(output, "APR "); break;
				case 4: strcat(output, "MAY "); break;
				case 5: strcat(output, "JUN "); break;
				case 6: strcat(output, "JUL "); break;
				case 7: strcat(output, "AUG "); break;
				case 8: strcat(output, "SEP "); break;
				case 9: strcat(output, "OCT "); break;
				case 10: strcat(output, "NOV"); break;
				case 11: strcat(output, "DEC"); break;
			}
			uitoa(atime->tm_mday, output);
			strcat(output, " ");
			uitoa(atime->tm_hour, output);
			strcat(output, ":");
			uitoa(atime->tm_min, output);
			strcat(output, "\t");

			strcat(output, data[i]);
			if(S_ISDIR(buf_stat.st_mode))
				strcat(output, "/");
			strcat(output, "\n");

			if(buf2==NULL)
				strcpy(buf2, output);
			else	
				strcat(buf2, output);
		}
	}
	else//ls -al
	{
		cnt=0; i=0;

		if(argc==1)
		{
			getcwd(getStr, 128);
			dp=opendir(getStr);
			while (d = readdir(dp)) 
			{
				strcpy(data[i], d->d_name);
				cnt++; i++;
			}
			closedir(dp);k++;
		}
		else
		{
			j=0;
			for(i=1; i<argc; i++)
			{
				stat(argv[i], &buf_stat);
				if(S_ISDIR(buf_stat.st_mode)){}
				else
				{
					strcpy(data[j], argv[i]);
					cnt++;j++;
				}
			}

			j=0;
			for(i=1; i<argc; i++)
			{
				stat(argv[i], &buf_stat);
				if(S_ISDIR(buf_stat.st_mode))
				{
					strcpy(dir_data[j], argv[i]);
					strcat(dir_data[j], "/");
					j++; dir_cnt++;
				}
			}

			k=cnt; max=cnt+dir_cnt;
		}

		sort(cnt, data);

		for(i=0; i<cnt; i++)
		{
			stat(data[i], &buf_stat);

			if(S_ISDIR(buf_stat.st_mode))
				strcpy(output, "d");
			else
				strcpy(output, "-");

			if(S_IRUSR & buf_stat.st_mode)
				strcat(output, "r");
			else
				strcat(output, "-");

			if(S_IWUSR & buf_stat.st_mode)
				strcat(output, "w");
			else
				strcat(output, "-");

			if(S_IXUSR & buf_stat.st_mode)
				strcat(output, "x");
			else
				strcat(output, "-");

			if(S_IRGRP & buf_stat.st_mode)
				strcat(output, "r");
			else
				strcat(output, "-");

			if(S_IWGRP & buf_stat.st_mode)
				strcat(output, "w");
			else
				strcat(output, "-");

			if(S_IXGRP & buf_stat.st_mode)
				strcat(output, "x");
			else
				strcat(output, "-");

			if(S_IROTH & buf_stat.st_mode)
				strcat(output, "r");
			else
				strcat(output, "-");

			if(S_IWOTH & buf_stat.st_mode)
				strcat(output, "w");
			else
				strcat(output, "-");

			if(S_IXOTH & buf_stat.st_mode)
				strcat(output, "x");
			else
				strcat(output, "-");

			strcat(output, "\t");

			ui_temp=(unsigned int)buf_stat.st_nlink;
			uitoa(ui_temp, output);
			strcat(output, "\t");

			uid=getpwuid(buf_stat.st_uid);
			strcat(output, uid->pw_name);
			strcat(output, "\t");

			gid=getgrgid(buf_stat.st_uid);
			strcat(output, gid->gr_name);
			strcat(output, "\t");

			ui_temp=(unsigned int)buf_stat.st_size;
			uitoa(ui_temp, output);
			strcat(output, "\t");

			atime=gmtime(&buf_stat.st_atime);
			switch(atime->tm_mon)
			{
				case 0: strcat(output, "JAN "); break;
				case 1: strcat(output, "FEB "); break;
				case 2: strcat(output, "MAR "); break;
				case 3: strcat(output, "APR "); break;
				case 4: strcat(output, "MAY "); break;
				case 5: strcat(output, "JUN "); break;
				case 6: strcat(output, "JUL "); break;
				case 7: strcat(output, "AUG "); break;
				case 8: strcat(output, "SEP "); break;
				case 9: strcat(output, "OCT "); break;
				case 10: strcat(output, "NOV"); break;
				case 11: strcat(output, "DEC"); break;
			}
			uitoa(atime->tm_mday, output);
			strcat(output, " ");
			uitoa(atime->tm_hour, output);
			strcat(output, ":");
			uitoa(atime->tm_min, output);
			strcat(output, "\t");

			strcat(output, data[i]);
			if(S_ISDIR(buf_stat.st_mode))
				strcat(output, "/");
			strcat(output, "\n");

			if(buf2==NULL)
				strcpy(buf2, output);
			else	
				strcat(buf2, output);
		}
	}

		if(k==max || dir_cnt==0)
			break;
		else if(k==0 && cnt==0 && dir_cnt==1)
		{
			chdir(getStr_o);
			chdir(dir_data[1]);
		}
		else if(dir_cnt>0)
		{
			chdir(getStr_o);
			if(buf2==NULL)
				strcpy(buf2, "\n");
			else	
				strcat(buf2, "\n");
			memset(getStr, 0, sizeof(getStr));
			strncpy(getStr, dir_data[a], strlen(dir_data[a])-1);
			n=strlen(getStr);
			if(buf2==NULL)
				strcpy(buf2, getStr);
			else	
				strcat(buf2, getStr);
			strcat(buf2, ":\n");
			chdir(getStr); a++;
		} argc=1;
}
	
}

void PWD(char buf2[BUF_MAX_SIZE])
{
	char getStr[128];

	getcwd(getStr, 128);

	if(buf2==NULL)
		strcpy(buf2, getStr);
	else	
		strcat(buf2, getStr);
	strcat(buf2, "\n");

	memset(reply, 0, sizeof(reply));
	strcpy(reply, "257 ");
	strcat(reply, getStr);
	strcat(reply, " is current directory.\n");
}

void CWD(char buf[BUF_MAX_SIZE])
{
	char *token;

	token=strtok(buf, "\t");
	token=strtok(NULL, "\n");

	chdir(token);

	memset(reply, 0, sizeof(reply));
	strcpy(reply, "250 CWD command successful.\n");
}

void CDUP()
{
	chdir("..");

	memset(reply, 0, sizeof(reply));
	strcpy(reply, "250 CWD command successful.\n");
}

void MKD(char buf[BUF_MAX_SIZE])
{
	char *token;

	token=strtok(buf, "\t");

	while(token=strtok(NULL, "\t\n"))
	{
		mkdir(token, 0775);
	}

	memset(reply, 0, sizeof(reply));
	strcpy(reply, "250 MKD command successful.\n");
}

void DELE(char buf[BUF_MAX_SIZE])
{
	char *token;
	struct stat exist_check;

	token=strtok(buf, "\t");

	while(token=strtok(NULL, "\t\n"))
	{
		stat(token, &exist_check);

		if(S_ISDIR(exist_check.st_mode))
			mkdir(token, 775);
		else
			unlink(token);
	}

	memset(reply, 0, sizeof(reply));
	strcpy(reply, "250 DELE command successful.\n");
}

void RMD(char buf[BUF_MAX_SIZE])
{
	char *token;

	token=strtok(buf, "\t");

	while(token=strtok(NULL, "\t\n"))
	{
		rmdir(token);
	}

	memset(reply, 0, sizeof(reply));
	strcpy(reply, "250 RMD command successful.\n");
}

void RNFR_RNTO(char buf[BUF_MAX_SIZE], char *dest)
{
	char *token, *token1;

	token1=strtok(dest, "\t");
	token1=strtok(NULL, "\t");

	token=strtok(buf, "\t");

	while(token=strtok(NULL, "\t\n"))
	{
		rename(token, token1);
	}

	memset(reply, 0, sizeof(reply));
	strcpy(reply, "350 File exists, ready for destination name.\n250 RNTO command successful.\n");
}
////////////////////////////////////////////
void ch_binary()
{
	cur_mode=BIN_MODE;
	strcpy(reply, "200 Type set to I.\n");
}
void ch_ascii()
{
	cur_mode=ASCII_MODE;
	strcpy(reply, "200 Type set to A.\n");
}
void RETR(char buf[BUF_MAX_SIZE])
{/*
	int fd;
	char *token;
	char temp[32]={0};

	token=strtok(buf, "\t");
	token=strtok(NULL, "\n");
	strcpy(temp, token);
	fd=open(token, FLAGS);

	memset(buf, 0, BUF_MAX_SIZE);
	strcpy(buf, temp);
	strcat(buf, "\t");
	read(fd, buf, BUF_MAX_SIZE);

	close(fd);
	log_infor(inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), id);*/
}
void STOR(char buf[BUF_MAX_SIZE])
{/*
	int fd;
	char *token;
	char temp[BUF_MAX_SIZE];

	token=strtok(buf, "\t");
	token=strtok(NULL, " \t");

	fd=open(token, FLAGS);

	memset(temp, 0, BUF_MAX_SIZE);

	token=strtok(NULL, "\n");
	if(token)
	{
		strncpy(temp, token, strlen(token));
		strncat(temp, "\n", strlen(token));
	}

	while(token=strtok(NULL, "\n"))
	{
		strncat(temp, token, strlen(token));
		strncat(temp, "\n", strlen(token));
	}
	write(1, temp, strlen(temp));
	write(fd, temp, BUF_MAX_SIZE);

	close(fd);
	log_infor(inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), id);*/
}
//////////////////////////////////////////
int make_argv(char buf[BUF_MAX_SIZE], char argv[1024][128])
{
	int argc=1;
	char *token;

	token=strtok(buf, "\t");
	strcpy(argv[0], token);

	while(token=strtok(NULL, "\t"))
	{
		strcpy(argv[argc], token);
		argc++;
	}

	return argc;
}

void sort(int argc, char argv[1024][128])
{
	int i, j;
	char temp[128];

	for(i=0; i<argc-1; i++)
	{
		for(j=0; j<argc-i-1; j++)
		{
			if(strcasecmp(argv[j], argv[j+1])>0)
			{
				strcpy(temp, argv[j]);
				strcpy(argv[j], argv[j+1]);
				strcpy(argv[j+1], temp);
			}
		}
	}
}

void uitoa(unsigned int in, char *output)
{
	char out[30]={0,};
	unsigned int temp;
	int i, check=1;

	temp=in;
	while(temp/=10)
	{
		check++;
	}

	for(i=check; i>0; i--)
	{
		out[i-1]=48+in%10;
		in/=10;
	}
	strcat(output, out);
}

void convert_str_to_addr(char *str, char ip[25], unsigned int *port)
{ //client로부터 받은 PORT명령어에 붙은 IP주소와 포트번호를 변경
	char *token;
	char port_temp[8];
	unsigned int *temp;

	token = strtok(str, " ");

	token = strtok(NULL, ",");
	strcpy(ip, token);
	strcat(ip, ".");

	token = strtok(NULL, ",");
	strcat(ip, token);
	strcat(ip, ".");

	token = strtok(NULL, ",");
	strcat(ip, token);
	strcat(ip, ".");

	token = strtok(NULL, ",");
	strcat(ip, token);

	token = strtok(NULL, ",");
	*port = atoi(token)*256;

	token = strtok(NULL, "\n");
	*port += atoi(token);
	/* your converting algorithm */
}
void log_infor(char ip[25], int port, char id[32])
{
	time_t timer;
	struct tm *t;
	char tt[32];
	char t_t[32];

	memset(tt, 0, 32);
	timer=time(NULL);
	t=localtime(&timer);

	switch(t->tm_wday)
	{
		case 0: strcpy(tt, "SUN "); break;
		case 1: strcpy(tt, "MON "); break;
		case 2: strcpy(tt, "TUE "); break;
		case 3: strcpy(tt, "WED "); break;
		case 4: strcpy(tt, "THU "); break;
		case 5: strcpy(tt, "FIR "); break;
		case 6: strcpy(tt, "SAT "); break;
	}
	switch(t->tm_mon)
	{
		case 0: strcat(tt, "JAN "); break;
		case 1: strcat(tt, "FEB "); break;
		case 2: strcat(tt, "MAR "); break;
		case 3: strcat(tt, "APR "); break;
		case 4: strcat(tt, "MAY "); break;
		case 5: strcat(tt, "JUN "); break;
		case 6: strcat(tt, "JUL "); break;
		case 7: strcat(tt, "AUG "); break;
		case 8: strcat(tt, "SEP "); break;
		case 9: strcat(tt, "OCT "); break;
		case 10: strcat(tt, "NOV"); break;
		case 11: strcat(tt, "DEC"); break;
	}
	memset(t_t, 0, 16);
	uitoa((t->tm_mday)+1, t_t);
	strcat(tt, t_t);
	strcat(tt," ");
	memset(t_t, 0, 16);
	uitoa(t->tm_hour, t_t);
	strcat(tt, t_t);
	strcat(tt,":");
	memset(t_t, 0, 16);
	uitoa(t->tm_min, t_t);
	strcat(tt, t_t);
	strcat(tt,":");
	memset(t_t, 0, 16);
	uitoa(t->tm_sec, t_t);
	strcat(tt, t_t);
	strcat(tt," ");
	memset(t_t, 0, 16);
	uitoa((t->tm_year)+1900, t_t);
	strcat(tt, t_t);

	write(fd_log, tt, strlen(tt));
	write(fd_log, " [", 2);
	write(fd_log, ip, strlen(ip));
	write(fd_log, ":", 1);

	memset(tt, 0, 32);
	uitoa(port, tt);
	write(fd_log, tt, strlen(tt));
	write(fd_log, "] ", 2);
	write(fd_log, id, strlen(id));
	write(fd_log, " ", 1);

}
