#include<iostream>
#include<fstream>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>
#include<stdio.h>
#include<cstdio>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include <arpa/inet.h>

using namespace std;

int main()
{

   int clientid = socket(AF_INET,SOCK_STREAM,0);
  
   if(clientid<0)
   {
      cout<<"\nclient socket failed\n";
      exit(1);
   }

   //server addr

   struct sockaddr_in addr;

   addr.sin_family = AF_INET; 
   addr.sin_addr.s_addr=inet_addr("127.0.0.1");
   addr.sin_port=htons(2000);

   int connectid = connect(clientid,(struct sockaddr*)&addr,sizeof(addr));

   if(connectid<0)
   {
        cout<<"\nclient connect failed\n";
        exit(1);
   }

   //int fd = open("temp.txt",O_RDONLY|O_CREAT,0666);

   //char buff[2048];
   //int bytes_read = read(fd,buff,2048);

    /*ifstream in;
    in.open("temp.txt");
    in.seekg(0,ios::end);
    int size = in.tellg();
    in.seekg(0,ios::beg);*/

    cout<<"\nEnter Path to file\n";
    char path[100];
    cin>>path;
    send(clientid,path,sizeof(path),0);

    FILE* fp = fopen(path,"rb");
   
    fseek(fp,0,SEEK_END);

    int size = ftell(fp);

    fseek(fp,0,SEEK_SET);

    cout<<size;

    send(clientid,&size,sizeof(size),0);

    int buff_size = 524288;

    char buff[buff_size];

    int bytes_read;

    while((bytes_read = fread(buff,sizeof(char),buff_size,fp))>0&&size>0)
    {
        send(clientid,buff,bytes_read,0);
        memset(buff,'\0',buff_size);
    }

  fclose(fp);
  close(clientid);

  return 0;

}
