/*
  Server finds images in directory.
  Creates entries into catalog.csv.
  Waits for clients.
  Responds to requests.
*/


/****Necessary includes****/
#include "md5sum.h"
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<unistd.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<dirent.h>
#include<sys/sendfile.h>
#include<fcntl.h>



/*****Required global variables****/
char* logpath;
char* jpg_array[200];
char* png_array[200];
char* gif_array[200];
char* tiff_array[200];



int jpg_count = 0;
int png_count = 0;
int gif_count = 0;
int tiff_count = 0;


char* file_array[200];
char* file_sizes[200];
int file_count = 0;




//Writes file size and file data to the socket
void write_to_socket(char* file_name, int sockfd)
{


 
   FILE* fp;
  
   if((fp = fopen(file_name,"rb")) == NULL)
     {
       printf("WHAT THE FUCK %s\n",strerror(errno));}
       //printf("Cannot open file\n");
  
  fseeko(fp,0,SEEK_END);
  int sz = ftello(fp);
  fseeko(fp,0,SEEK_SET);
  

  char sizeBuf[256];
  
  sprintf(sizeBuf,"%d",sz);

  write(sockfd,sizeBuf,256);
  int n;
  while(1)
    {
      unsigned char buf[256] = {0};
      
      
      
      int n = fread(buf,1,256,fp);
  
      if(n>0)
	{
	  
	  write(sockfd,buf,n);
	}
      
      if(n<256)
	{
	  if(feof(fp))
	    printf("Finished sending: %s\n",file_name);
	  break;
	}
    }
  fclose(fp);

}
      
    


//Writes file info to the catalog.
void write_to_catalog(char* file_name, int file_size, unsigned char* sum)
{
  int i;
  
  FILE* fp;
  fp = fopen(logpath,"a");
  fprintf(fp,"%s",file_name);
  fprintf(fp,",");
  fprintf(fp,"%d",file_size);
  fprintf(fp,",");
  for(i=0;i<MD5_DIGEST_LENGTH;i++)
    {
      fprintf(fp,"%02x",sum[i]);
    }
  fprintf(fp,"%s\n"," ");
  fclose(fp);
}

int is_jpg(char* file_name)
{
  if(strstr(file_name,"jpg")) return 1;
  return 0;
}

int is_png(char* file_name)
  {
    if(strstr(file_name,"png")) return 1;
    return 0;
  }

  int is_gif(char* file_name)
  {
    if(strstr(file_name,"gif")) return 1;
    return 0;
  }

  int is_tiff(char* file_name)
  {
    if(strstr(file_name,"tiff")) return 1;
    return 0;
  }



//Adds files to the corresponding arrays.
void addToArray(char* file_path)
{
  if(is_jpg(file_path))
    {
      jpg_array[jpg_count] = file_path;
      jpg_count++;
    }
  if(is_png(file_path)) 
    {
      png_array[png_count] = file_path;
      png_count++;
    }
  if(is_gif(file_path)) 
    {
      gif_array[gif_count] = file_path;
      gif_count++;
    }
  if(is_tiff(file_path)) 
    {
      tiff_array[tiff_count] = file_path;
      tiff_count++;
    }

  file_array[file_count] = file_path;
  file_count++;
}



//Recursively finds images in directories and sub-directories
int find_images(char* in_dir,char* catpath)
{
  
  
  DIR* indir;
  struct dirent* file;
  
  if((indir=opendir(in_dir)) == NULL)
    {
      perror("Unable to open input dir\n");
      exit(1);
    }
  
  while((file = readdir(indir))!=NULL)
    {
      if((file->d_name[0]) == '.') continue;
      
      if(file->d_type == DT_DIR)
	{
	  
	  char* path2 = malloc(200);
	  strcpy(path2,in_dir);
	  strcat(path2,"/");
	  strcat(path2,file->d_name);
	  find_images(path2,catpath);
	}
      
      if(is_png(file->d_name)||is_gif(file->d_name)
	 ||is_tiff(file->d_name)||is_jpg(file->d_name))
	{
	  char* file_path = malloc(200);
	  strcpy(file_path,in_dir);
	  strcat(file_path,"/");
	  strcat(file_path,file->d_name);
	 
	  unsigned char sum[MD5_DIGEST_LENGTH];
	  int i;
	  addToArray(file_path);
	  md5sum(file_path,sum);
	  
	  struct stat st = {0};
	  
	  stat(file_path,&st);
	  
	  write_to_catalog(file->d_name,(int)st.st_size,sum);
	  
	  
	}}
  return 0;}


int main(int argc, char* argv[])
{
  
  if(argc!=3)
    {
      perror("Incorrect number of arguments\n");
      perror("Usage: ./server <port_number> <directory>\n");
      exit(1);
    }

  logpath = malloc(200);

  
  strcpy(logpath,argv[2]);
  strcat(logpath,"/");
  strcat(logpath,"catalog.csv");
 

  struct stat st = {0};
  
  if(stat(logpath,&st) !=-1)
    {
      perror("Catalog file exists. Deleting it.\n");
      remove(logpath);
      FILE* fp;
      fp = fopen(logpath,"a+");
      perror("catalog.csv created\n");
      fclose(fp);
    }
  
  else
    {
      
      FILE* fp;
      fp = fopen(logpath,"a+");
      perror("catalog.csv created\n");
      fclose(fp);
    }


  FILE* fp;
  fp = fopen(logpath,"a+");
  
  fprintf(fp,"File name,Size,Sum\n");
  fclose(fp);
  int i;
  int result;
  
  if((result=find_images(argv[2],logpath))!=0)
    printf("Error finding images\n");

  int listenfd;
  int confd;
  
  struct sockaddr_in serv_addr;
  
  char sendBuff[1025];
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  
  memset(&serv_addr,'0',sizeof(serv_addr));
  memset(sendBuff,'0',sizeof(sendBuff));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(atoi(argv[1]));

  bind(listenfd,(struct sockaddr*)&serv_addr, sizeof(serv_addr));

  if(listen(listenfd,10) == -1)
    {
      perror("Could not listen\n");
      return -1;
    }
  
  
  
  printf("Listening for clients...\n");

  while(1)
    {
      
      confd = accept(listenfd, (struct sockaddr*)NULL, NULL);
      
      int n;
      
      char buf[10];
      
      printf("accepted a connection\n");
      
      write_to_socket(logpath,confd);

      read(confd,buf,11);

      
      
	  

	  
	  if(is_jpg(buf) || is_png(buf) || is_gif(buf) || is_tiff(buf))
	    {
	      printf("Client in passive mode. Requesting %s files:\n",buf);
	      
	      if(is_jpg(buf))
		{

		  for(i=0;i<jpg_count;i++)
		    {
		      char buffer[4];
		      read(confd,buffer,5);
		      
		      if(strstr(buffer,"ready"))
			write_to_socket(jpg_array[i],confd);
		    }
		}

	      if(is_png(buf))
		{

		  for(i=0;i<png_count;i++)
		    {
		      char buffer[4];
		      read(confd,buffer,5);
		      
		      if(strstr(buffer,"ready"))
			{
			  //printf("this file: %s\n",png_array[i]);
			  write_to_socket(png_array[i],confd);
			}
		    }
		}

	      if(is_gif(buf))
		{

		  for(i=0;i<gif_count;i++)
		    {
		      char buffer[4];
		      read(confd,buffer,5);
		      
		      if(strstr(buffer,"ready"))
			{
			  printf("This file:%s\n",gif_array[i]);
			write_to_socket(gif_array[i],confd);
			}
		    }
		}
	    
	      if(is_tiff(buf))
		{

		  for(i=0;i<tiff_count;i++)
		    {
		      char buffer[4];
		      read(confd,buffer,5);
		      
		      if(strstr(buffer,"ready"))
			write_to_socket(tiff_array[i],confd);
		    }
		}


	    

	    }
	  
    if(strstr(buf,"interactive")) 
      {
	

	printf("Client in interactive mode\n");
	
	while(1)
	  {
	    
	    char input[1];
	    bzero(input,2);
	    read(confd,input,2);
	    if(strstr(input,"0")) 
	      {
		printf("Client entered 0. Terminating connection..\n");
		close(confd);
		exit(0);
	      }
	    else
	      {
		
		printf("input from client:%s\n",input);
		write_to_socket(file_array[atoi(input)-1],confd);
		
	      }
	  }
	close(confd);
	return 0;
	
      }
       

	  
	  
	 

	 
      
	   
	
	    
	      
		
	      
  
	
      
      

      

      
      
      //sleep(1);
      
      return 0;
    }
  close(confd);
  
  return 0;
}
