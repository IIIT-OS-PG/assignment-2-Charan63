#include<iostream>
#include<fstream>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<math.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include <arpa/inet.h>
#include<pthread.h>
#include <openssl/sha.h>

using namespace std;

void *multiclientfunc(void* temp)
{
  
  char path[100];
  int filesize;
  int newid = *(int *)temp;
 
  recv(newid,path,sizeof(path),0);

  char filename[20];

  char* token = strtok(path,"/");
  int k=0;
  
  while(token!=NULL)
  {
    strcpy(filename,token);
    token = strtok(NULL,"/");
  }   

  recv(newid,&filesize,sizeof(filesize),0);

  FILE* fp1 = fopen(filename,"wb");

  int buff_size = 524288;

  char buffer[buff_size];

  int bytes_recv;

  unsigned char hashed[20];

  unsigned char totalhash[(int)ceil(filesize/(double)buff_size)*20];

  int hashcount=0;

  while((bytes_recv = recv(newid,buffer,buff_size,0))>0 && filesize>0)
  {
     fwrite(buffer,sizeof(char),bytes_recv,fp1);
     
     SHA1((unsigned char*)buffer,sizeof(buffer)-1,hashed);  
     if(hashcount==0)
     strcpy((char*)totalhash,(char*)hashed);
     else
     strcat((char*)totalhash,(char*)hashed);

     hashcount++;

     memset(buffer,'\0',buff_size);
     filesize=filesize-bytes_recv;
  }

  cout<<"\nTransfer done\n";

  fclose(fp1);

}


int main()
{


   //pthread_t serverthread;
   //pthread_t clientthread;

   //int thread_debug = pthread_create(&serverthread,NULL,server,(void *)temp);
   
   int serverid = socket(AF_INET,SOCK_STREAM,0);
  
   if(serverid<0)
   {
      cout<<"\nserver socket failed\n";
      exit(1);
   }


   struct sockaddr_in addr;

   addr.sin_family = AF_INET; 
   addr.sin_addr.s_addr=inet_addr("127.0.0.1");
   addr.sin_port=htons(2000);

   int bindval = bind(serverid,(struct sockaddr*)&addr,sizeof(addr));

   if(bindval<0)
   {
        cout<<"\nserver bind failed\n";
        exit(1);
   }

   int listenval = listen(serverid,3);

   if(listenval<0)
   {
        cout<<"\nserver listen failed\n";
        exit(1);
   }

   //addr will be overwriten with client addr

  int addrlen = sizeof(addr);
  int newid;

  //as accept is a blocking call it is know continuously waiting for request to accept from different clients

 while((newid = accept(serverid,(struct sockaddr*)&addr,(socklen_t*)&addrlen))>0 )
 {
      pthread_t multiclient;

      int* temp = new int();
      *temp = newid; 

      int thread_debug = pthread_create(&multiclient,NULL,multiclientfunc,(void *)temp);

      if(thread_debug<0)
      {
         cout<<"\nThread not created\n";
         exit(1);    
      }

 }

 return 0;

}
