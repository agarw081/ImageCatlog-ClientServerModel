/*
  The client attempts to make a connection with the server.
  This connection can be made in interactive or passive mode
  based on command line arguments.
  The client then creates an html file containing links to 
  downloaded files.
*/


/*****Required headers*****/
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<dirent.h>
#include "md5sum.h"


/****Required global variables***/
int inter = 0;

char* htmlpath;
int flag = 0;
char file_array[200][200];
char file_sizes[200][200];
int file_count=0;

unsigned char checksums[200][200];


char* jpg_array[200];
char* png_array[200];
char* gif_array[200];
char* tiff_array[200];

int jpg_count = 0;
int png_count = 0;
int gif_count = 0;
int tiff_count = 0;


/***Helper functions***/
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


void write_to_file(char* file_path,char* data)
{
  FILE* fp;
  fp = fopen(file_path,"wb");
  fprintf(fp,data,strlen(data));
  fclose(fp);
}


//Reads data from the socket and writes to the file.
void write_to_image(char* file_path,int sockfd)
{
 

  char sizeBuf[256];
  read(sockfd,sizeBuf,256);         //Get the size of the file.
  printf("Downloading file: %s\n",file_path);
  FILE* fp;
  fp = fopen(file_path,"ab");
  char buf[256];
  int num;
  int total=0;
 
      
  while(( num = read(sockfd,buf,256))>0)   //Read 256 bytes at once.
	{
	  total +=num;
	  fwrite(buf,1,num,fp);            //Write to file.
	  
	  if(total==atoi(sizeBuf))         //break if size equals total.
	    {
	      printf("Downloaded: %s\n",file_path);
	      break;
	    }
	}
      
      
	fclose(fp);
	if(flag!=0&&inter==0)
	  write(sockfd,"ready",5);    //Send signal to server "ready"

	flag = 1;                        
  
    
  
}
    
    
//Add file names to the corresponding arrays.
void addToArray(char* file_name,int i )
{
  
      
      if(is_jpg(file_name))
	{
	  
	  jpg_array[jpg_count] = file_name;
	  jpg_count++;
	}
      
      if(is_png(file_name))
	{
	  
	  png_array[png_count] = file_name;
	  png_count++;
	}

      if(is_gif(file_name))
	{
	  
	  gif_array[gif_count] = file_name;
	  gif_count++;
	}
      
      if(is_tiff(file_name))
	{
	  
	  tiff_array[tiff_count] = file_name;
	  tiff_count++;
	}
}



	  

  
  
  int main(int argc, char* argv[])
{

  char* recvBuff = malloc(512);

  if(argc<3)
    {
      perror("Incorrect number of arguments\n");
      exit(0);
    }



  struct stat s;

  int err = stat("images",&s);

  if(-1==err)
    {
      if(ENOENT == errno)
	{
	  printf("Directory for images does not exist, creating it\n");
	  mkdir("images",0777);
	  printf("Directory created\n");
	}
    }
  else
    {
      if(S_ISDIR(s.st_mode))
	{
	  printf("Directory for images exists\n");
	}
    }

  struct stat st = {0};
  
  if(stat("images/catalog.csv",&st)!=-1)
    {
      perror("Catalog exists. Deleting it..\n");
      remove("images/catalog.csv");
    }
 


  htmlpath = malloc(200);

  strcpy(htmlpath,"images/");
  strcat(htmlpath,"download.html");

    
  

  int sockfd;
  
  struct sockaddr_in serv_addr;
  
  if((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
    {
      perror("Could not create socket connection\n");
      exit(0);
    }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(atoi(argv[2]));
  serv_addr.sin_addr.s_addr = inet_addr(argv[1]);

  if(connect(sockfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr))<0)
    {
      perror("Connection failed\n");
      exit(0);
    }

  char* logpath = malloc(200);

  strcpy(logpath,"images/catalog.csv");

  
  write_to_image(logpath,sockfd);


  FILE* fp;

  fp = fopen(logpath,"r");
  
  char ch;

  int lines=0;
  while(!feof(fp))
    {
      ch = fgetc(fp);
      if(ch == '\n')    //Get the number of lines in log file.
	{
	  lines++;
	}
    }
  
  fclose(fp);

  char* line = NULL;
  
  size_t len = 0;

  fp = fopen(logpath,"r");
  getline(&line,&len,fp);             //Read log file line by line.
  

  printf("Dumping catalog.csv content\n");

  int i = 1;
  int j;
  int k;
  int m;
  int n;
  for(k = 0;k<lines-1;k++)
    {
      strcpy(file_array[file_count],"images/");
      printf("[%d] ",i);
      getline(&line,&len,fp);

      for(j=0;line[j]!=',';j++)
	{
	  
	  file_array[file_count][j+7] = line[j];
	  printf("%c",line[j]);
	}

      char* name = strtok(line,",");
      char* size = strtok(NULL,",");
      char* sum = strtok(NULL,",");
      strcpy(checksums[file_count],sum);
      //printf("%s\n",checksums[file_count]);
     
    
	  

      
      
      file_count++;
      i++;
      printf(" \n");
    }
      
  fclose(fp);
  

  for(i=0;i<file_count;i++)
    {
      
      addToArray(file_array[i],i);
    }





  if(argv[3]!=NULL)                  //Arg[3] present. Passive mode.
     {
      printf("Passive mode\n");
      
    

      write(sockfd,argv[3],11);

      write(sockfd,"ready",5);
  
      if(is_jpg(argv[3]))                 
	{                               //Write jpg images
	  for(i=0;i<jpg_count;i++)
	    write_to_image(jpg_array[i],sockfd);
	}

      if(is_png(argv[3]))
	{
	  for(i=0;i<png_count;i++)          //Write png images.
	    write_to_image(png_array[i],sockfd);
	}
      
       if(is_gif(argv[3]))
	 {                                //Write gif files.
	  for(i=0;i<gif_count;i++)
	    write_to_image(gif_array[i],sockfd);
	}

       if(is_tiff(argv[3]))
	 {                                 //Write tiff files.
	   for(i=0;i<tiff_count;i++)
	     write_to_image(tiff_array[i],sockfd);
	 }
	  
    }

   else
     {                              //argv[3] not present. interactive mode.
      inter = 1;
      printf("Interactive mode\n");
      write(sockfd,"interactive",11);

      char input[1];
      while(1)
	{

	  printf("Enter the file number to download\n");
	  scanf("%s",input);
	  
	  if(atoi(input)==0)
	    {
	      write(sockfd,input,2);
	      printf("Entered 0. Exiting...\n");
	      close(sockfd);
	      break;
	    }
	  printf("File number entered:%d\n",atoi(input));

	  write(sockfd,input,2);
	  write_to_image(file_array[atoi(input)-1],sockfd);
	}
      
    }

    printf("Done\n");


    //Create html file.
    FILE* html;
    
    html = fopen("./images/download.html","w+");
    
    fprintf(html,"<html><head>Downloaded Images</head>");
    fprintf(html,"<title>Downloaded Images</title>\n");
    fprintf(html,"<p>");


    /*
      For each file in the downloaded(images) folder, look for the
      file in the file_array,get the index, see if checksums 
      are equal. Accordingly write to html.
    */

    DIR* dir;
    struct dirent* file;

    dir = opendir("./images/");
    
    while((file = readdir(dir))!=NULL)
      {
	int i;
	int j;
	if(file->d_name[0] == '.') continue;
	if(strcmp(file->d_name,"catalog.csv")==0) continue;
	char* path = malloc(200);
	strcpy(path,"images/");
	strcat(path,file->d_name);
	unsigned char sum[MD5_DIGEST_LENGTH];
	char convertedsum[(MD5_DIGEST_LENGTH*2)+1];
	md5sum(path,sum);

	/**stackoverflow.com/questions/7627723/how-to-create-a-md5-hash-of-a-string-in-c**/
	for(j=0;j<MD5_DIGEST_LENGTH;j++)
	  {
	    sprintf(convertedsum+(j*2),"%02x",sum[j]);
	  }
	convertedsum[2*MD5_DIGEST_LENGTH]='\0';
	char* other_name = malloc(200);

	for(i=0;i<file_count;i++)
	  {
	    
	    if(strstr(path,file_array[i]))
	      {
		
		if(strncmp(convertedsum,checksums[i],MD5_DIGEST_LENGTH)==0)
		  {
		    
		    fprintf(html,"Checksum match!\n");
		    fprintf(html, "      ");
		    fprintf(html, "<a href = \"");
		    
		    fprintf(html,"%s\">",file->d_name);
		    fprintf(html,"%s",file->d_name);
		    fprintf(html,"</a></p>\n");

		  }
		else
		  {
		    
		    fprintf(html,"Checksum mismatch");
		    fprintf(html,"       ");
		    fprintf(html,"%s",file->d_name);
		    fprintf(html,"</p>");
		  }
	      }
	  }
      }

     
	    


	fprintf(html,"</body></html>");
	fclose(html);


  return 0;
    
     

}
