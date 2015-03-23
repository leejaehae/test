/////////////////////////////////////////////////////////////////////// 
// File Name : srv.c // 
// Date : 2014/04/18 // 
// Os : Ubuntu 12.04 LTS 64bits // 
// Author : Lee Jae Hae // 
// Student ID : 2009720151 // 
// ----------------------------------------------------------------- // 
// Title : System Programming Assignment #1 // 
// Description : linux command program // 
///////////////////////////////////////////////////////////////////////
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

#define BUF_MAX_SIZE 8192

void command(char *buf);//command check
void NLST(char *buf);//ls and dir command
void PWD();//pwd command
void CWD(char *buf);//cd command
void CDUP();//cd .. command
void MKD(char *buf);//mkdir command
void DELE(char *buf);//delete command
void RMD(char *buf);//redir command
void RNFR_RNTO(char *buf, char *dest);//rename command
int make_argv(char *buf, char argv[1024][128]);//make arguments
void sort(int argc, char argv[1024][128]);//sort function
void uitoa(unsigned int in, char *output);//in->output

int main(void)
{
	char buf[BUF_MAX_SIZE];

	read(STDIN_FILENO, buf, BUF_MAX_SIZE);

	command(buf);

	return 0;
}

void command(char *buf)
{
	int ck=0;
	int n;
	char *token, *temp;

	token=strtok(buf, "\n");
	while(token)
	{
		if(strncmp(token, "NLST", 4)==0 || strncmp(token, "LIST", 4)==0) NLST(buf);
		else if(strncmp(token, "PWD", 3)==0) PWD();
		else if(strncmp(token, "CWD", 3)==0) CWD(buf);
		else if(strncmp(token, "CDUP", 4)==0) CDUP();
		else if(strncmp(token, "MKD", 3)==0) MKD(buf);
		else if(strncmp(token, "DELE", 4)==0) DELE(buf);
		else if(strncmp(token, "RMD", 3)==0) RMD(buf);
		else if(strncmp(token, "RNFR", 4)==0)
		{
			temp=strtok(NULL, "\n");
			RNFR_RNTO(token, temp);
		}
		else
		{
			n=strlen(token);
			write(STDOUT_FILENO, token, n);
			write(STDOUT_FILENO, "\n", 1);
		}

		token=strtok(NULL, "\n");
	}
}

void NLST(char *buf)
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

		n=strlen(output);
		write(STDOUT_FILENO, output, n);
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



		n=strlen(output);
		write(STDOUT_FILENO, output, n);
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

			n=strlen(output);
			write(STDOUT_FILENO, output, n);
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

			n=strlen(output);
			write(STDOUT_FILENO, output, n);
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
			write(STDOUT_FILENO, "\n", 1);memset(getStr, 0, sizeof(getStr));
			strncpy(getStr, dir_data[a], strlen(dir_data[a])-1);
			n=strlen(getStr);
			write(STDOUT_FILENO, getStr, n);
			write(STDOUT_FILENO, ":\n", 2);
			chdir(getStr); a++;
		} argc=1;
}
	
}

void PWD()
{
	char getStr[128];

	getcwd(getStr, 128);

	write(STDOUT_FILENO, getStr, strlen(getStr));
	write(STDOUT_FILENO, "\n", 1);
}

void CWD(char *buf)
{
	char *token;

	token=strtok(buf, "\t");
	token=strtok(NULL, "\n");

	chdir(token);
}

void CDUP()
{
	chdir("..");
}

void MKD(char *buf)
{
	char *token;

	token=strtok(buf, "\t");

	while(token=strtok(NULL, "\t\n"))
	{
		mkdir(token, 0775);
	}
}

void DELE(char *buf)
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
}

void RMD(char *buf)
{
	char *token;

	token=strtok(buf, "\t");

	while(token=strtok(NULL, "\t\n"))
	{
		rmdir(token);
	}
}

void RNFR_RNTO(char *buf, char *dest)
{
	char *token, *token1;

	token1=strtok(dest, "\t");
	token1=strtok(NULL, "\t");

	token=strtok(buf, "\t");

	while(token=strtok(NULL, "\t\n"))
	{
		rename(token, token1);
	}
}

int make_argv(char *buf, char argv[1024][128])
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
