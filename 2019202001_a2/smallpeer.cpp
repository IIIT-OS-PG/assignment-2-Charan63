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
#include<unordered_map>
#include<sstream>
using namespace std;


struct ipportfn
{

char *ip;
int port;
char *filename;
int threads_no;
int no_of_chunks;
char *path;

};


struct ipport
{

char *ip;
int port;

};


unordered_map<string,vector<int>> filemap;
unordered_map<string,int> ipmap;
unordered_map<int,string> invipmap;
unordered_map<int,vector<int>> chunkmap;
unordered_map<string,string> pathmap;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int global_thread_count=0;

void download_server(int newid)
{

char fname[20];
recv(newid,fname,sizeof(fname),0);

int k = filemap[fname].size();

send(newid,&k,sizeof(k),0);

int temparr[k];

for(int i=0;i<k;i++)
{

temparr[i]=filemap[fname][i];

}

send(newid,temparr,sizeof(temparr),0);


int selected_chunks;

recv(newid,&selected_chunks,sizeof(selected_chunks),0);


FILE* fp = fopen(pathmap[fname].c_str(),"rb");


for(int z=0;z<selected_chunks;z++)
{

int chunkno;
recv(newid,&chunkno,sizeof(chunkno),0);
cout<<"chunk  no"<<chunkno<<endl;
int buff_size = 524288;
fseek(fp,chunkno*524288,SEEK_SET);
char buffer[buff_size];
int bytes_read = fread(buffer,sizeof(char),buff_size,fp);
cout<<"bytes_read "<<bytes_read<<endl;
send(newid,buffer,bytes_read,0);
memset(buffer,'\0',sizeof(buffer));

}

fclose(fp);


}


void *multiclientfunc(void* temp)
{
  
  char command[20];
  
  int newid = *(int *)temp;
 
  recv(newid,command,sizeof(command),0);

  if(strcmp(command,"download")==0)
  {
     download_server(newid);
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

  int addrlen = sizeof(addr);
  int newid;

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


int selectrandom(vector<int> temp)
{
   int x = temp.size();
   
   int rand1 = rand()%x;

   return temp[rand1];
  
}

void* downloader(void* soc_temp)
{

struct ipportfn *soc = (struct ipportfn*)soc_temp;

char *ip = new char[20](); 
ip=soc->ip;

int port = soc->port;

char *fname = new char[20]();
fname=soc->filename;

int no_of_threads = soc->threads_no;

int chunks = soc->no_of_chunks;

char path[100];

strcpy(path,"../Videos");


cout<<endl<<endl<<path<<endl<<endl;


strcat(path,"/");
strcat(path,fname);

string ipkey(ip);
ipkey=ipkey+":"+to_string(port);


cout<<ip<<" "<<port<<" "<<fname<<" "<<no_of_threads<<" "<<chunks<<" "<<path<<" "<<ipkey<<endl;


int client_server = socket(AF_INET,SOCK_STREAM,0);
  
if(client_server<0)
{
   cout<<"\nDownloader client_server socket failed\n";
    exit(1);
}


struct sockaddr_in addr_server;
addr_server.sin_family = AF_INET; 
addr_server.sin_addr.s_addr=inet_addr(ip);
addr_server.sin_port=htons(port);

int connect_server = connect(client_server,(struct sockaddr*)&addr_server,sizeof(addr_server));

if(connect_server<0)
{
   cout<<"\nDownloader connect_server connect failed\n";
   cout<<ip<<" "<<port<<endl;
   exit(1);
}


char command[20];
strcpy(command,"download");
send(client_server,command,sizeof(command),0);
send(client_server,fname,sizeof(fname),0);
int k;
recv(client_server,&k,sizeof(k),0);

cout<<"No : of chunks "<<k<<endl;


int arr[k];
recv(client_server,arr,sizeof(arr),0);


for(int i=0;i<k;i++)
{
    cout<<arr[i]<<" ";

}

cout<<endl;

pthread_mutex_lock(&lock);

int index = ipmap[ipkey];

cout<<" ip no "<<index<<endl;

for(int i=0;i<k;i++)
{
    chunkmap[arr[i]].push_back(index);

    for(int j=0;j<chunkmap[arr[i]].size();j++)
    {
          cout<<chunkmap[arr[i]][j]<<" ";

    }

   cout<<endl;


}

cout<<endl;


global_thread_count++;


pthread_mutex_unlock(&lock);


//busy wait untill all threads finish update

while(global_thread_count<no_of_threads);

//piece selection


char buffer[524288];

int bytecount;


int selected_chunks = 0;

vector<int>chunkvec(chunks);


for(int i=0;i<chunks;i++)
{
   int ind = selectrandom(chunkmap[i]);
   string ipandport = invipmap[ind];

   cout<<"chunk "<<i<<" "<<ipandport<<endl;

   if(ipandport==ipkey)
   {
       chunkvec[selected_chunks++]=i; 

   }
   
}


cout<<endl<<selected_chunks<<endl;

cout<<endl<<chunkvec.size()<<endl;


for(int i=0;i<selected_chunks;i++)
{
    cout<<chunkvec[i]<<" ";

}


cout<<endl;

send(client_server,&selected_chunks,sizeof(selected_chunks),0);

FILE *fp = fopen(path,"wb");

for(int i=0;i<selected_chunks;i++)
{

int k = chunkvec[i];

send(client_server,&k,sizeof(k),0);

int n = recv(client_server,buffer,sizeof(buffer),0);

fseek(fp,k*524288,SEEK_SET);
fwrite(buffer,sizeof(char),n,fp);

memset(buffer,'\0',sizeof(buffer));

cout<<endl<<k<<" done\n";
}

//fclose(fp);


}


void download(char ip[],int port,char *tokens[],int num)
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


char path[100];

strcpy(path,tokens[3]);

cout<<endl<<" path from starting "<<path<<endl;;


char filename[20];
 
  strcpy(filename,tokens[2]);

int gid = stoi(tokens[1]);

char command[20];
strcpy(command,tokens[0]);


cout<<gid<<" "<<command<<" "<<filename<<endl;



send(clientid,command,sizeof(command),0);
send(clientid,&gid,sizeof(gid),0);
send(clientid,filename,sizeof(filename),0);

int count;

recv(clientid,&count,sizeof(count),0);

char ipports[count][30];


count=count-1;

cout<<count<<endl;


for(int i=0;i<count;i++)
{

recv(clientid,ipports[i],sizeof(ipports[i]),0);

string ip_p(ipports[i]);

ipmap[ip_p]=i;
invipmap[i]=ip_p;

cout<<ip_p<<endl;


}

int no_of_chunks;

recv(clientid,&no_of_chunks,sizeof(no_of_chunks),0);

cout<<endl<<endl<<no_of_chunks<<endl<<endl;


close(clientid);


int k=0;

char sock[30];

char* temptokens[2];

temptokens[0]=new char[20];
temptokens[1]=new char [10];
int tok;

cout<<count<<endl;


pthread_t download_thread[count];

while(k<count)
{

pthread_t download_thread[k];
strcpy(sock,ipports[k]);
tok = tokenize(sock,temptokens,":");

struct ipportfn soc;
soc.ip = temptokens[0];
soc.port = stoi(temptokens[1]);
soc.filename=filename;
soc.threads_no=count;
soc.no_of_chunks=no_of_chunks;
soc.path=path;
//cout<<" inside while : "<<path<<" "<<soc.path<<endl;

int thread_debug = pthread_create(&download_thread[k],NULL,downloader,(void *)(&soc));

if(thread_debug<0)
{
   cout<<"\nDownloader Thread not created\n";
   exit(1);    
}

k++;

//void* status;
//pthread_join(download_thread,&status);

}

for(int i=0;i<count;i++)
{

void* status;
pthread_join(download_thread[i],&status);

}



}


void list_files(char ip[],int port,char *tokens[],int num)
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

send(clientid,command,sizeof(command),0);


int gid = stoi(tokens[1]);

send(clientid,&gid,sizeof(gid),0);

//cout<<gid<<endl;

int noe;
recv(clientid,&noe,sizeof(noe),0);

//cout<<noe<<endl;

char filearray[noe][20];


for(int i=0;i<noe;i++)
{

recv(clientid,filearray[i],sizeof(filearray[i]),0);

cout<<filearray[i]<<" ";


}

cout<<endl;

}

void upload(char ip[],int port,char *tokens[],int num)
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


char path[100];

  strcpy(path,tokens[1]);

  char filename[20];

  char* token = strtok(path,"/");
  int k=0;
  
  while(token!=NULL)
  {
    strcpy(filename,token);
    token = strtok(NULL,"/");
  }

  string s1(filename);
  string s2(tokens[1]);


  cout<<"\nfilename :"<<s1<<endl;

  pathmap[s1]=s2;


  FILE* fp = fopen(tokens[1],"rb");
   
  fseek(fp,0,SEEK_END);

  long int filesize = ftell(fp);

  cout<<"\nfilesize :"<<filesize<<endl;

  fseek(fp,0,SEEK_SET);

  int buff_size = 524288;

  char buffer[buff_size];

  int bytes_read;

  unsigned char hashed[20];

  char totalhash[(int)ceil(filesize/(double)buff_size)*20*2];

  cout<<"\nactualsiz :"<<(int)ceil(filesize/(double)buff_size)*20*2<<endl;


  int hashcount=0;

  string hexvalofhash="";
  stringstream stemp;

  int no_of_chunks = (int)ceil(filesize/(double)buff_size);


  while((bytes_read = fread(buffer,sizeof(char),buff_size,fp))>0&&filesize>0)
  {
      SHA1((const unsigned char*)buffer,strlen(buffer),hashed);
     
      for(int i=0;i<20;i++)
      stemp<<std::hex<<(int)hashed[i];      
    
      hexvalofhash=hexvalofhash+stemp.str();

      //cout<<stemp.str()<<endl;
      
       stemp.str(std::string());
      
      hashcount++;
      memset(buffer,'\0',buff_size);
      filesize = filesize - bytes_read;
  }

  fclose(fp);

  strcpy(totalhash,hexvalofhash.c_str());

char command[20];
strcpy(command,tokens[0]);

int gid = stoi(tokens[2]);

char iptemp[20];

strcpy(iptemp,ip);

int siz = strlen(totalhash);

  cout<<command<<" "<<iptemp<<" "<<port<<" "<<gid<<" "<<filename<<" "<<siz<<" "<<no_of_chunks<<endl;

int x = no_of_chunks;


send(clientid,command,sizeof(command),0);
send(clientid,iptemp,sizeof(iptemp),0);
send(clientid,&port,sizeof(port),0);
send(clientid,&gid,sizeof(gid),0);
send(clientid,&x,sizeof(x),0);
send(clientid,filename,sizeof(filename),0);
send(clientid,&siz,sizeof(siz),0);
send(clientid,totalhash,sizeof(totalhash),0);


string fname(filename);

for(int i=0;i<no_of_chunks;i++)
{

filemap[fname].push_back(i);
string ktmp(tokens[1]);
pathmap[fname]=ktmp;

}


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
      if(strcmp(tokens[0],"upload")==0)
      {
          upload(ip,port,tokens,num);
      }
      else if(strcmp(tokens[0],"download")==0)
      {
           download(ip,port,tokens,num);

      }
      else if(strcmp(tokens[0],"list_files")==0)
      {
           list_files(ip,port,tokens,num);

      }

   }

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
