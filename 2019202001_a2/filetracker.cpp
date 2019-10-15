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
#include<string.h>
#include<algorithm>
#include<unordered_map>
using namespace std;


struct ipport
{

char *ip;
int port;

};

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


void download(int newid)
{

int gid;
char fname[20];

recv(newid,&gid,sizeof(gid),0);
recv(newid,fname,sizeof(fname),0);


ifstream in;
in.open("file_details.txt");


char ch_ipport[30];
int ch_gid;
char ch_fname[20];

int flag=0;

int count=0;


char line2[1024];
char* tokens2[3];

  for(int i=0;i<3;i++)
  tokens2[i] = new char[30]();

int tok;

while(!in.eof())
{
  
   in.getline(line2,1024);
   tok = tokenize(line2,tokens2," ");

   ch_gid = stoi(tokens2[0]);
   strcpy(ch_fname,tokens2[1]);

   if(ch_gid==gid&&(strcmp(ch_fname,fname)==0))
   {
       count++;

   }

}

in.close();


//cout<<"\ncount inside tracker : "<<count<<endl;


in.open("file_details.txt");


char ipports[count][30];


int k=0;

while(!in.eof())
{

   in.getline(line2,1024);
   tok = tokenize(line2,tokens2," ");

   ch_gid = stoi(tokens2[0]);
   strcpy(ch_fname,tokens2[1]);
   strcpy(ch_ipport,tokens2[2]);

   if(ch_gid==gid&&(strcmp(ch_fname,fname)==0))
   {
       strcpy(ipports[k++],ch_ipport);

      // cout<<ch_ipport<<endl;

   }

}

in.close();

send(newid,&count,sizeof(count),0);

for(int i=0;i<count-1;i++)
{
   send(newid,ipports[i],sizeof(ipports[i]),0);

}


in.open("hash_details.txt");

int groupid;
string filename,hash;

int no_of_chunks;

while(!in.eof())
{

in>>groupid;
in>>filename;
in>>no_of_chunks;
in>>hash;

if(groupid==gid&&(strcmp(fname,filename.c_str())==0))
{
     //cout<<groupid<<" "<<filename<<" "<<no_of_chunks<<endl;

     break;
}

}



//cout<<endl<<no_of_chunks<<endl;

int x = no_of_chunks;

send(newid,&x,sizeof(x),0);

}

void list_files(int newid)
{

int gid;

recv(newid,&gid,sizeof(gid),0);

//cout<<gid<<endl;

ifstream in;
in.open("file_details.txt");

int ch_gid;

char ch_filename[20];

char ch_ipport[30];

unordered_map<string,int> filelist;

int k=0;

while(!in.eof())
{
    in>>ch_gid;
    in>>ch_filename;
    in>>ch_ipport;

    if(ch_gid==gid)
    {
       filelist[ch_filename]=gid;
       //cout<<ch_filename<<" "<<gid<<endl;  
    }

}

int siz = filelist.size();

char filearray[siz][20];

int z=0;
for(auto it=filelist.begin();it!=filelist.end();it++)
{
string st = it->first;
strcpy(filearray[z++],st.c_str());

//cout<<st<<" ";

}


send(newid,&siz,sizeof(siz),0);

//cout<<"\n No "<<siz<<endl;

for(int i=0;i<siz;i++)
{

//cout<<filearray[i]<<" ";

send(newid,filearray[i],sizeof(filearray[i]),0);

}


}

void upload(int newid)
{

char ip[20];
int port;
int gid;
int siz;
char fname[20];
int chunks;

recv(newid,ip,sizeof(ip),0);
recv(newid,&port,sizeof(port),0);
recv(newid,&gid,sizeof(gid),0);
recv(newid,&chunks,sizeof(chunks),0);
recv(newid,fname,sizeof(fname),0);
recv(newid,&siz,sizeof(siz),0);
char hash[siz];
recv(newid,hash,sizeof(hash),0);

//cout<<ip<<" "<<port<<" "<<gid<<" "<<siz<<" "<<fname<<" "<<chunks<<" "<<hash<<endl;

ifstream in;
in.open("file_details.txt");


char ipport[30];

strcpy(ipport,ip);
strcat(ipport,":");
strcat(ipport,to_string(port).c_str());

char ch_ipport[30];
int ch_port;
int ch_gid;
char ch_fname[20];

int flag=0;

while(in.is_open()&&!in.eof())
{
   in>>ch_gid;
   in>>ch_fname;
   in>>ch_ipport;

   if(gid==ch_gid&&(strcmp(ch_ipport,ipport)==0)&&(strcmp(ch_fname,fname)==0))
   {
      flag=1;
      break;

   }
}

in.close();


if(!flag)
{

ofstream out;

out.open("file_details.txt",ios_base::app|ios_base::out);
out<<gid<<" "<<fname<<" "<<ip<<":"<<port<<endl;
out.close();


out.open("hash_details.txt",ios_base::app|ios_base::out);
out<<gid<<" "<<fname<<" "<<chunks<<" "<<hash<<endl;
out.close();

}

}
void *reqhandler(void* temp)
{

  int newid = *(int *)temp;
  
  char command[20];
   
  recv(newid,command,sizeof(command),0);

  cout<<command<<endl;

  if(strcmp(command,"upload")==0)
  {
      upload(newid);
  }
  else if(strcmp(command,"download")==0)
  {
      download(newid);

  }
  else if(strcmp(command,"list_files")==0)
  {
      list_files(newid);

  }


}

void *trackerfunc(void* tempsoc)
{

struct ipport *soc = new ipport(); 

soc = (struct ipport*)tempsoc;

char *ip = new char[20]; 
ip = soc->ip;

int port = soc->port;

int trackerid = socket(AF_INET,SOCK_STREAM,0);
  
   if(trackerid<0)
   {
      cout<<"\ntracker socket failed\n";
      exit(1);
   }

   struct sockaddr_in trackeraddr;

   trackeraddr.sin_family = AF_INET; 
   trackeraddr.sin_addr.s_addr=inet_addr(ip);
   trackeraddr.sin_port=htons(port);

   int bindval = bind(trackerid,(struct sockaddr*)&trackeraddr,sizeof(trackeraddr));

   if(bindval<0)
   {
        perror("Tracker bind failed");
        exit(1);
   }

   int listenval = listen(trackerid,5);

   if(listenval<0)
   {
        perror("Tracker listen failed");
        exit(1);
   }

  int addrlen = sizeof(trackeraddr);
  int newid;

 while((newid = accept(trackerid,(struct sockaddr*)&trackeraddr,(socklen_t*)&addrlen))>0 )
 {
      pthread_t multireq;

      int* temp = new int();
      *temp = newid; 
   
      int thread_debug = pthread_create(&multireq,NULL,reqhandler,(void *)temp);

      if(thread_debug<0)
      {
         cout<<"\nThread not created\n";
         exit(1);    
      }

 }


}


int main(int argc,char* argv[])
{

ofstream out;
out.open("file_details.txt",ios_base::trunc);
out.close();
out.open("hash_details.txt",ios_base::trunc);
out.close();

pthread_t tracker_thread;

struct ipport soc;
soc.ip = argv[1];
soc.port = atoi(argv[2]);

int sthread_debug = pthread_create(&tracker_thread,NULL,trackerfunc,(void *)(&soc));

   if(sthread_debug<0)
   {
         cout<<"\nserver thread not created\n";
         exit(1);    
   }

 void* status;

 pthread_join(tracker_thread,&status);

 return 0;

}

