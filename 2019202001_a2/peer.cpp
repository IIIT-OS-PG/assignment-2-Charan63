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
#include<vector>
#include<algorithm>
using namespace std;


struct ipport
{

char *ip;
int port;

};


int login_indicator;

static int q[100][100];
static int sizofq[100];

//strings q[100];

void join_group_server(int newid)
{
  //char ip[20];
  //int port;

  int uid,gid;

  //recv(newid,ip,sizeof(ip),0);
  //recv(newid,&port,sizeof(port),0);
  recv(newid,&uid,sizeof(uid),0);
  recv(newid,&gid,sizeof(gid),0);
 
  //cout<<endl<<"From server : "<<uid<<endl;

  /*ipport soc;
  soc.ip= new char[20];
  strcpy(soc.ip,ip);
  soc.port = port;
  */

   

  q[gid][sizofq[gid]++]=uid;

  //cout<<"\nadded to q ";
  //cout<<endl<<q[sizofq-1]<<endl;

}

void list_requests_server(int newid)
{
   int gid;
   recv(newid,&gid,sizeof(gid),0); 
   
   int siz = sizofq[gid];

   send(newid,&siz,sizeof(siz),0);
   send(newid,q[gid],sizeof(q[gid]),0);
   
}



void *multiclientfunc(void* temp)
{
  
  char command[20];
  
  int newid = *(int *)temp;
 
  recv(newid,command,sizeof(command),0);

  if(strcmp(command,"join_group")==0)
  {
     join_group_server(newid);
  }
  else if(strcmp(command,"list_requests")==0)
  {
     list_requests_server(newid);
  }

 
}


void *serverfunc(void* tempsoc)
{

struct ipport *soc = (struct ipport*)tempsoc;

char *ip = new char[20]; 
ip = soc->ip;

int port = soc->port;


int serverid = socket(AF_INET,SOCK_STREAM,0);
  
   if(serverid<0)
   {
      cout<<"\nserver socket failed\n";
      exit(1);
   }

   struct sockaddr_in addr;

   addr.sin_family = AF_INET; 
   addr.sin_addr.s_addr=inet_addr(ip);
   addr.sin_port=htons(port);

   int bindval = bind(serverid,(struct sockaddr*)&addr,sizeof(addr));

   if(bindval<0)
   {
        cout<<"\nserver bind failed\n";
        exit(1);
   }

   int listenval = listen(serverid,5);

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


}


int tokenize(char cmd[], char *tokens[],string ch)
{
  char* token = strtok(cmd,ch.c_str());
  int k=0;
  while(token!=NULL)
  {
    strcpy(tokens[k++],token);
    token = strtok(NULL,ch.c_str());

  }

return k;   

}



void logout(char ip[20],int port,char *tokens[],int num)
{

  int clientid = socket(AF_INET,SOCK_STREAM,0);
  
  if(clientid<0)
  {
      cout<<"\nclient socket failed\n";
      exit(1);
  }

   struct sockaddr_in addr;
   addr.sin_family = AF_INET; 
   addr.sin_addr.s_addr=inet_addr("127.0.0.1");
   addr.sin_port=htons(2020);

   int connectid = connect(clientid,(struct sockaddr*)&addr,sizeof(addr));

  if(connectid<0)
  {
   cout<<"\nclient connect failed\n";
   exit(1);
  }
    
  char command[20];
  strcpy(command,tokens[0]);

  char iptemp[20];
  strcpy(iptemp,ip);

  send(clientid,command,sizeof(command),0);
  send(clientid,iptemp,sizeof(iptemp),0);
  send(clientid,&port,sizeof(port),0);
  
  int res;
  recv(clientid,&res,sizeof(res),0);

  if(res==1)
  {
     exit(1); 
  }

  close(clientid);

}



void login(char ip[20],int port,char *tokens[],int num)
{

  int clientid = socket(AF_INET,SOCK_STREAM,0);
  
  if(clientid<0)
  {
      cout<<"\nclient socket failed\n";
      exit(1);
  }

   struct sockaddr_in addr;
   addr.sin_family = AF_INET; 
   addr.sin_addr.s_addr=inet_addr("127.0.0.1");
   addr.sin_port=htons(2020);

   int connectid = connect(clientid,(struct sockaddr*)&addr,sizeof(addr));

  if(connectid<0)
  {
   cout<<"\nclient connect failed\n";
   exit(1);
  }
    
  int uid = atoi(tokens[1]);
  char pwd[20];
  strcpy(pwd,tokens[2]);

  char command[20];
  strcpy(command,tokens[0]);

  char iptemp[20];
  strcpy(iptemp,ip);

  //cout<<"sender :"<<command<<" "<<ip<<" "<<port<<" "<<uid<<" "<<pwd<<endl;
  
  send(clientid,command,sizeof(command),0);
  send(clientid,iptemp,sizeof(iptemp),0);
  send(clientid,&port,sizeof(port),0);
  send(clientid,&uid,sizeof(uid),0);
  send(clientid,pwd,sizeof(pwd),0);

  int res;
  recv(clientid,&res,sizeof(res),0);

  if(res==1)
  {
      cout<<"\nAuthentication successful\n";
      login_indicator = 1;

  }
  else
  {
     cout<<"\nAuthentication Failed\n";
     exit(1);
  }

  
  close(clientid);

}



void list_requests(char ip[],int port,char *tokens[],int num)
{

if(login_indicator!=1)
{
    cout<<"\nLogin and try again\n";
    exit(1);
}

int clientid = socket(AF_INET,SOCK_STREAM,0);
  
if(clientid<0)
{
   cout<<"\nclient socket failed\n";
   exit(1);
}


struct sockaddr_in addr;

addr.sin_family = AF_INET; 
addr.sin_addr.s_addr=inet_addr("127.0.0.1");
addr.sin_port=htons(2020);


int connectid = connect(clientid,(struct sockaddr*)&addr,sizeof(addr));
if(connectid<0)
{
   cout<<"\nclient connect failed\n";
   exit(1);
}

   
int gid = atoi(tokens[1]);


char command[20];
strcpy(command,tokens[0]);

char iptemp[20];
strcpy(iptemp,ip);

send(clientid,command,sizeof(command),0);
send(clientid,&gid,sizeof(gid),0);
send(clientid,iptemp,sizeof(iptemp),0);
send(clientid,&port,sizeof(port),0);

 
int uid;
recv(clientid,&uid,sizeof(uid),0);

char owneripport[30];
recv(clientid,owneripport,sizeof(owneripport),0);

char* ownertokens[2];

for(int i=0;i<2;i++)
ownertokens[i] = new char[20]();

int tok = tokenize(owneripport,ownertokens,":");

close(clientid);


int client_server = socket(AF_INET,SOCK_STREAM,0);
  
if(client_server<0)
{
   cout<<"\nclient_server socket failed\n";
   exit(1);
}


struct sockaddr_in addr_server;

addr_server.sin_family = AF_INET; 
addr_server.sin_addr.s_addr=inet_addr(ownertokens[0]);
addr_server.sin_port=htons(atoi(ownertokens[1]));

int connect_server = connect(client_server,(struct sockaddr*)&addr_server,sizeof(addr_server));
if(connect_server<0)
{
   cout<<"\nconnect_server connect failed\n";
   exit(1);
}

send(client_server,command,sizeof(command),0);
send(client_server,&gid,sizeof(gid),0);

int sizeofvec;

recv(client_server,&sizeofvec,sizeof(sizeofvec),0);

int tempq[sizeofvec];

recv(client_server,tempq,sizeof(tempq),0);

for(int i=0;i<sizeofvec;i++)
{
    cout<<tempq[i]<<" ";

}

cout<<endl;

close(client_server);

}

void accept_request(char ip[],int port,char *tokens[],int num)
{

if(login_indicator!=1)
{
    cout<<"\nLogin and try again\n";
    exit(1);

}

int clientid = socket(AF_INET,SOCK_STREAM,0);
  
if(clientid<0)
{
   cout<<"\nclient socket failed\n";
   exit(1);
}


struct sockaddr_in addr;

addr.sin_family = AF_INET; 
addr.sin_addr.s_addr=inet_addr("127.0.0.1");
addr.sin_port=htons(2020);


int connectid = connect(clientid,(struct sockaddr*)&addr,sizeof(addr));
if(connectid<0)
{
   cout<<"\nclient connect failed\n";
   exit(1);
}

int gid = atoi(tokens[1]);

int uid = atoi(tokens[2]);

//cout<<gid<<" "<<uid;


char command[20];
strcpy(command,tokens[0]);

char iptemp[20];
strcpy(iptemp,ip);


auto it = find(q[gid],q[gid]+sizofq[gid],uid);

int index = it-q[gid];

//cout<<"\nFrom client\n";

//cout<<index<<" ";

for(int i=index;i<sizofq[gid]-1;i++)
{
     q[gid][i]=q[gid][i+1];

}

int fl=0;

if(sizofq[gid]>0&&index<sizofq[gid])
sizofq[gid]--;
else
fl=1;


send(clientid,command,sizeof(command),0);

if(fl==0&&index<=sizofq[gid])
{
send(clientid,&gid,sizeof(gid),0);
send(clientid,&uid,sizeof(uid),0);
}
else
{
uid=-1,gid=-1;
send(clientid,&gid,sizeof(gid),0);
send(clientid,&uid,sizeof(uid),0);
}


}



void leave_group(char ip[],int port,char *tokens[],int num)
{

if(login_indicator!=1)
{
    cout<<"\nLogin and try again\n";
    exit(1);

}

int clientid = socket(AF_INET,SOCK_STREAM,0);
  
if(clientid<0)
{
   cout<<"\nclient socket failed\n";
   exit(1);
}


struct sockaddr_in addr;

addr.sin_family = AF_INET; 
addr.sin_addr.s_addr=inet_addr("127.0.0.1");
addr.sin_port=htons(2020);


int connectid = connect(clientid,(struct sockaddr*)&addr,sizeof(addr));
if(connectid<0)
{
   cout<<"\nclient connect failed\n";
   exit(1);
}

int gid = atoi(tokens[1]);



char command[20];
strcpy(command,tokens[0]);

char iptemp[20];
strcpy(iptemp,ip);

send(clientid,command,sizeof(command),0);
send(clientid,&gid,sizeof(gid),0);
send(clientid,iptemp,sizeof(iptemp),0);
send(clientid,&port,sizeof(port),0);

}

void join_group(char ip[],int port,char *tokens[],int num)
{

if(login_indicator!=1)
{
    cout<<"\nLogin and try again\n";
    exit(1);

}

int clientid = socket(AF_INET,SOCK_STREAM,0);
  
if(clientid<0)
{
   cout<<"\nclient socket failed\n";
   exit(1);
}


struct sockaddr_in addr;

addr.sin_family = AF_INET; 
addr.sin_addr.s_addr=inet_addr("127.0.0.1");
addr.sin_port=htons(2020);


int connectid = connect(clientid,(struct sockaddr*)&addr,sizeof(addr));
if(connectid<0)
{
   cout<<"\nclient connect failed\n";
   exit(1);
}

   
int gid = atoi(tokens[1]);


char command[20];
strcpy(command,tokens[0]);

char iptemp[20];
strcpy(iptemp,ip);

//cout<<"\nsending to tracker";
//cout<<endl<<command<<" "<<gid<<" "<<iptemp<<" "<<port<<endl;


send(clientid,command,sizeof(command),0);
send(clientid,&gid,sizeof(gid),0);
send(clientid,iptemp,sizeof(iptemp),0);
send(clientid,&port,sizeof(port),0);
  

int uid;
recv(clientid,&uid,sizeof(uid),0);
char owneripport[30];
recv(clientid,owneripport,sizeof(owneripport),0);

char* ownertokens[2];

for(int i=0;i<2;i++)
ownertokens[i] = new char[20]();

int tok = tokenize(owneripport,ownertokens,":");

close(clientid);


int client_server = socket(AF_INET,SOCK_STREAM,0);
  
if(client_server<0)
{
   cout<<"\nclient_server socket failed\n";
   exit(1);
}


//cout<<"\nowner:details\n";
//cout<<ownertokens[0]<<" "<<ownertokens[1]<<endl;


struct sockaddr_in addr_server;

addr_server.sin_family = AF_INET; 
addr_server.sin_addr.s_addr=inet_addr(ownertokens[0]);
addr_server.sin_port=htons(atoi(ownertokens[1]));

int connect_server = connect(client_server,(struct sockaddr*)&addr_server,sizeof(addr_server));
if(connect_server<0)
{
   cout<<"\nconnect_server connect failed\n";
   exit(1);
}

send(client_server,command,sizeof(command),0);
send(client_server,&uid,sizeof(uid),0);
send(client_server,&gid,sizeof(gid),0);
//send(client_server,iptemp,sizeof(iptemp),0);
//send(client_server,&port,sizeof(port),0);

close(client_server);


}
void create_group(char ip[],int port,char *tokens[],int num)
{

if(login_indicator!=1)
{
    cout<<"\nLogin and try again\n";
    exit(1);

}


int clientid = socket(AF_INET,SOCK_STREAM,0);
  
if(clientid<0)
{
   cout<<"\nclient socket failed\n";
   exit(1);
}


struct sockaddr_in addr;

addr.sin_family = AF_INET; 
addr.sin_addr.s_addr=inet_addr("127.0.0.1");
addr.sin_port=htons(2020);


int connectid = connect(clientid,(struct sockaddr*)&addr,sizeof(addr));
if(connectid<0)
{
   cout<<"\nclient connect failed\n";
   exit(1);
}

   
int gid = atoi(tokens[1]);

char command[20];
strcpy(command,tokens[0]);

char iptemp[20];
strcpy(iptemp,ip);

  send(clientid,command,sizeof(command),0);
  send(clientid,iptemp,sizeof(iptemp),0);
  send(clientid,&port,sizeof(port),0);
  send(clientid,&gid,sizeof(gid),0);
  
  close(clientid);

}

void list_groups(char ip[],int port,char *tokens[],int num)
{


if(login_indicator!=1)
{
    cout<<"\nLogin and try again\n";
    exit(1);

}


int clientid = socket(AF_INET,SOCK_STREAM,0);
  
if(clientid<0)
{
   cout<<"\nclient socket failed\n";
   exit(1);
}


struct sockaddr_in addr;

addr.sin_family = AF_INET; 
addr.sin_addr.s_addr=inet_addr("127.0.0.1");
addr.sin_port=htons(2020);


int connectid = connect(clientid,(struct sockaddr*)&addr,sizeof(addr));
if(connectid<0)
{
   cout<<"\nclient connect failed\n";
   exit(1);
}

  char command[20];
  strcpy(command,tokens[0]);

  send(clientid,command,sizeof(command),0);

  int no;

  recv(clientid,&no,sizeof(no),0);

  int g[no];

  recv(clientid,g,sizeof(g),0);

  cout<<endl;

  for(int i=0;i<no;i++)
  { 
     cout<<g[i]<<" ";
  }



}

void upload(char ip[],int port,char *tokens[],int num)
{

if(login_indicator!=1)
{
    cout<<"\nLogin and try again\n";
    exit(1);

}

int clientid = socket(AF_INET,SOCK_STREAM,0);
  
if(clientid<0)
{
   cout<<"\nclient socket failed\n";
   exit(1);
}

struct sockaddr_in addr;

addr.sin_family = AF_INET; 
addr.sin_addr.s_addr=inet_addr("127.0.0.1");
addr.sin_port=htons(2020);

int connectid = connect(clientid,(struct sockaddr*)&addr,sizeof(addr));
if(connectid<0)
{
   cout<<"\nclient connect failed\n";
   exit(1);
}
  
  char path[100];

  strcpy(path,tokens[1]);

  cout<<path<<endl;

  char filename[20];

  char* token = strtok(path,"/");
  int k=0;
  
  while(token!=NULL)
  {
    strcpy(filename,token);
    token = strtok(NULL,"/");
  }

  //cout<<filename<<endl; 

  FILE* fp = fopen(tokens[1],"rb");
   
  fseek(fp,0,SEEK_END);

  int filesize = ftell(fp);

  fseek(fp,0,SEEK_SET);

  //cout<<filesize<<endl;

  FILE* fp1 = fopen(filename,"wb");

  int buff_size = 524288;

  char buffer[buff_size];

  int bytes_recv;

  unsigned char hashed[20];

  char totalhash[(int)ceil(filesize/(double)buff_size)*20];

  int hashcount=0;

  while(filesize>0)
  {
        int b = fwrite(buffer,sizeof(char),buff_size,fp1);
        SHA1((unsigned char*)buffer,sizeof(buffer)-1,hashed);  
        if(hashcount==0)
        strcpy(totalhash,(char*)hashed);
        else
        strcat(totalhash,(char*)hashed);
        hashcount++;
        memset(buffer,0,buff_size);
        filesize=filesize-b;
  } 

fclose(fp1);

char command[20];
strcpy(command,tokens[0]);

send(clientid,command,sizeof(command),0);
send(clientid,totalhash,sizeof(totalhash),0);


//cout<<strlen(totalhash);
/*
  while((bytes_recv = recv(newid,buffer,buff_size,0))>0 && filesize>0)
  {
     fwrite(buffer,sizeof(char),bytes_recv,fp1);
    
      SHA1((unsigned char*)buffer,sizeof(buffer)-1,hashed);  
      if(hashcount==0)
      strcpy(totalhash,(char*)hashed);
      else
      strcat(totalhash,(char*)hashed);
      hashcount++;
      memset(buffer,0,buff_size);
      filesize=filesize-bytes_recv;
  }
    fclose(fp1);
    cout<<totalhash;
*/

}

void create_user(char ip[],int port,char *tokens[],int num)
{


if(login_indicator!=1)
{
    cout<<"\nLogin and try again\n";
    exit(1);

}


int clientid = socket(AF_INET,SOCK_STREAM,0);
  
if(clientid<0)
{
   cout<<"\nclient socket failed\n";
   exit(1);
}


struct sockaddr_in addr;

addr.sin_family = AF_INET; 
addr.sin_addr.s_addr=inet_addr("127.0.0.1");
addr.sin_port=htons(2020);


int connectid = connect(clientid,(struct sockaddr*)&addr,sizeof(addr));
if(connectid<0)
{
   cout<<"\nclient connect failed\n";
   exit(1);
}

   
int uid = atoi(tokens[1]);
char pwd[20];
strcpy(pwd,tokens[2]);

  char command[20];
  strcpy(command,tokens[0]);

  char iptemp[20];
  strcpy(iptemp,ip);

  //cout<<"sender :"<<command<<" "<<iptemp<<" "<<port<<" "<<uid<<" "<<pwd<<endl;  
  send(clientid,command,sizeof(command),0);
  //send(clientid,iptemp,sizeof(iptemp),0);
  //send(clientid,&port,sizeof(port),0);
  send(clientid,&uid,sizeof(uid),0);
  send(clientid,pwd,sizeof(pwd),0);

  close(clientid);


}


void clientfunc(char ip[],int port)
{
   
   char input[300];
   char * tokens[4];
 
   for(int i=0;i<4;i++)
   tokens[i] = new char[100]();

   int num;

   while(1)
   {
      cin.getline(input,300);
      int num = tokenize(input,tokens," ");
      /*
      cout<<"\nclientfunc:\n";
      for(int i=0;i<num;i++)
      cout<<tokens[i]<<" ";
      cout<<endl;
      */
      if(strcmp(tokens[0],"create_user")==0)
      {
          create_user(ip,port,tokens,num);
      }
      else if(strcmp(tokens[0],"login")==0)
      { 
          login(ip,port,tokens,num);
      }
      else if(strcmp(tokens[0],"create_group")==0)
      {
           create_group(ip,port,tokens,num);
      }
      else if(strcmp(tokens[0],"join_group")==0)
      {
           //cout<<"\njoin group called\n";
           join_group(ip,port,tokens,num);
      }
      else if(strcmp(tokens[0],"list_requests")==0)
      {
           list_requests(ip,port,tokens,num);
      }
      else if(strcmp(tokens[0],"accept_request")==0)
      {
           accept_request(ip,port,tokens,num);
      }
      else if(strcmp(tokens[0],"list_groups")==0)
      { 
           list_groups(ip,port,tokens,num);   
      }
      else if(strcmp(tokens[0],"leave_group")==0)
      { 
           leave_group(ip,port,tokens,num);   
      }
      else if(strcmp(tokens[0],"logout")==0)
      {
           logout(ip,port,tokens,num);
      }

      
             
   }


/*
    send(clientid,path,100,0);

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

*/

}


int main(int argc,char* argv[])
{

struct ipport soc;
soc.ip = argv[1];
soc.port = atoi(argv[2]);

pthread_t serverthread;

int sthread_debug = pthread_create(&serverthread,NULL,serverfunc,(void *)(&soc));

   if(sthread_debug<0)
   {
         cout<<"\nserver thread not created\n";
         exit(1);    
   }


  clientfunc(soc.ip,soc.port);
  

 return 0;


}

