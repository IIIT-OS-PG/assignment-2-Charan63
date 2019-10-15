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

struct user_auth
{

struct ipport ipdetails;
int uid;
string pwd;

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

void create_user(int newid)
{

char ip[20];
int port;
int uid;
char pwd[20];

//recv(newid,ip,sizeof(ip),0);
//recv(newid,&port,sizeof(port),0);
recv(newid,&uid,sizeof(uid),0);
recv(newid,pwd,sizeof(pwd),0);

ifstream in;
in.open("user_details.txt");

char line[1024];

char* tokens[2];

for(int i=0;i<2;i++)
tokens[i] = new char[20]();

int tok,flag=0;

while(!in.eof())
{
   in.getline(line,1024);
   
   tok = tokenize(line,tokens," ");

   int ufid = stoi(tokens[0]);

   if(ufid==uid)
   {
      flag=1;
      break;
   }

}

in.close();

if(flag)
{
   //cout<<"\ncreate user with another uid\n";

}
else
{

ofstream out;
out.open("user_details.txt",ios_base::app|ios_base::out);
//out<<uid<<" "<<ip<<" "<<port<<" "<<pwd<<endl;
out<<uid<<" "<<pwd<<endl;

out.close();

}
//int flag=1;
//send(newid,&flag,sizeof(flag),0);

}



void leave_group(int newid)
{


int gid;
char ip[20];
int port;

recv(newid,&gid,sizeof(gid),0);
recv(newid,ip,sizeof(ip),0);
recv(newid,&port,sizeof(port),0);


//cout<<gid<<" "<<ip<<" "<<port<<endl;


  ifstream in2;
  in2.open("current_login.txt");

  char line2[1024];
  char* tokens2[2];

  for(int i=0;i<2;i++)
  tokens2[i] = new char[100]();

  char uid_char[100];

  string s(ip);
  s=s+":"+to_string(port);

  int tok;

  while(!in2.eof())
  {
     in2.getline(line2,1024);
   
     tok = tokenize(line2,tokens2," ");

     string temp(tokens2[1]);

     if(temp==s)
     {
       strcpy(uid_char,tokens2[0]);
       break;
     }
  }

  in2.close();

  int uid = atoi(uid_char);

  ifstream in;
  in.open("group_details.txt");

  string p;

  string temp = to_string(gid)+" "+to_string(uid);

  vector<string> v;

  while(!in.eof())
  {
     getline(in,p);
      
     if(p!=temp)
       v.push_back(p);
  }

  in.close();

  ofstream out;

  out.open("group_details.txt",ios::trunc);

  for(int i=0;i<v.size();i++)
  {
       out<<v[i]<<endl;

  }

  out.close();

  
}

void join_group(int newid)
{

int gid;
char ip[20];
int port;

recv(newid,&gid,sizeof(gid),0);
recv(newid,ip,sizeof(ip),0);
recv(newid,&port,sizeof(port),0);

//cout<<"\nrecieved at tracker:\n";
//cout<<gid<<" "<<ip<<" "<<port<<endl;

ifstream in;
in.open("group_details.txt");

char line[1024];
char* tokens[12];

for(int i=0;i<12;i++)
tokens[i] = new char[10]();

int tok,flag=0,file_pos;

char ownerid[10];

while(!in.eof())
{
   in.getline(line,1024);
   
   tok = tokenize(line,tokens," ");

   int gfid = stoi(tokens[0]);
   
   strcpy(ownerid,tokens[1]);

   if(gfid==gid)
   {
      flag=1;
      break;
   }

}

in.close();


//cout<<"\nowner id "<<ownerid<<endl;

if(!flag)
{
 // cout<<"\nGroup not Found\n";
}
else
{

  ifstream in2;
  in2.open("current_login.txt");

  char line2[1024];
  char* tokens2[2];

  for(int i=0;i<2;i++)
  tokens2[i] = new char[100]();

  char uid_char[100];

  string s(ip);
  s=s+":"+to_string(port);

  //cout<<endl<<s<<endl;

  //cout<<"\nreading file\n";

  while(!in2.eof())
  {
     in2.getline(line2,1024);
   
     //cout<<line2<<endl;
      // cout<<line2;

     tok = tokenize(line2,tokens2," ");
     string temp(tokens2[1]);

     //cout<<temp<<endl;


     if(temp==s)
     {
       strcpy(uid_char,tokens2[0]);
       break;
     }

  }

  in2.close();


  //cout<<"\nuid_char"<<endl<<uid_char<<endl;


  char owneripport[30];

  ifstream in1;
  in1.open("current_login.txt");

  char line1[1024];
  char* tokens1[2];

  for(int i=0;i<2;i++)
  tokens1[i] = new char[100]();

  while(!in1.eof())
  {

     in1.getline(line1,1024);
   
     tok = tokenize(line1,tokens1," ");

     if(strcmp(ownerid,tokens1[0])==0)
     {
         strcpy(owneripport,tokens1[1]);
         break;
     }
  

  }

  in1.close();

  int uid = atoi(uid_char);

  //cout<<"\nsending to clent";

  //cout<<endl<<uid<<" "<<owneripport<<endl;

  send(newid,&uid,sizeof(uid),0);
  send(newid,owneripport,sizeof(owneripport),0);    

}


}

void create_group(int newid)
{

char ip[20];
int port;
int gid;

recv(newid,ip,sizeof(ip),0);
recv(newid,&port,sizeof(port),0);
recv(newid,&gid,sizeof(gid),0);

string s(ip);
s=s+":"+to_string(port);

ifstream in1;
in1.open("group_details.txt");

char line[1024];

char* tokens[2];

for(int i=0;i<2;i++)
tokens[i] = new char[20]();

int tok,flag=0;

while(!in1.eof())
{
   in1.getline(line,1024);
   
   tok = tokenize(line,tokens," ");

   int gfid = stoi(tokens[0]);

   if(gfid==gid)
   {
      flag=1;
      break;
   }

}

in1.close();


if(flag);
//cout<<"\nGid already exist\n";
else
{

ifstream in;
in.open("current_login.txt");

char line[1024];

char* tokens[2];

for(int i=0;i<2;i++)
tokens[i] = new char[100]();


char uid_char[100];

while(!in.eof())
{
   in.getline(line,1024);
   
   tok = tokenize(line,tokens," ");

   string temp(tokens[1]);

   if(temp==s)
   {
       strcpy(uid_char,tokens[0]);
       break;
   }

}

in.close();

ofstream out;
out.open("group_details.txt",ios_base::app|ios_base::out);
out<<gid<<" "<<uid_char<<endl;
out.close();

}

}

void list_groups(int newid)
{

ifstream in;
in.open("group_details.txt");

int gid,uid;

unordered_map<int,int> m;

int count=0;

while(!in.eof())
{

in>>gid;
in>>uid;

if(m.find(gid)==m.end())
{
  m[gid]=uid;
  count++;
}

}


in.close();

int groups[count];

int k=0;

for(auto it=m.begin();it!=m.end();it++)
{
    groups[k++]=it->first;
}


send(newid,&count,sizeof(count),0);
send(newid,groups,sizeof(groups),0);

}


void accept_request(int newid)
{

int uid;
int gid;

recv(newid,&gid,sizeof(gid),0);
recv(newid,&uid,sizeof(uid),0);

if(uid!=-1&&gid!=-1)
{
ofstream out;
out.open("group_details.txt",ios_base::app|ios_base::out);
out<<gid<<" "<<uid<<endl;
out.close();
}

}


void logout(int newid)
{

char ip[20];
int port;

recv(newid,ip,sizeof(ip),0);
recv(newid,&port,sizeof(port),0);

ifstream in;
in.open("current_login.txt");

int uid;
string ipport;

string temp(ip);
temp = temp + ":"+to_string(port);

vector<string> svec;

   string p;

  vector<string> v;

  while(!in.eof())
  {
     getline(in,p);
      
     if(p.find(temp)==string::npos)
     svec.push_back(p);
  }

  in.close();

  ofstream out;
  out.open("current_login.txt",ios::trunc);

  for(int i=0;i<svec.size();i++)
  {
     out<<svec[i]<<endl;
  }

int flag=1;
send(newid,&flag,sizeof(flag),0);
  
}



void login(int newid)
{

char ip[20];
int port;
int uid;
char pwd[20];

recv(newid,ip,sizeof(ip),0);
recv(newid,&port,sizeof(port),0);
recv(newid,&uid,sizeof(uid),0);
recv(newid,pwd,sizeof(pwd),0);

//cout<<"reciever :"<<ip<<"\n"<<port<<"\n"<<uid<<"\n"<<pwd<<endl;

ifstream in;
in.open("user_details.txt");

int ch_uid;
char ch_pwd[20];


int flag=0;

while(!in.eof())
{
    in>>ch_uid;
    //in>>ch_ip;
    //in>>ch_port;
    in>>ch_pwd;
    
    /*
    if(ch_uid==uid&&ch_port==port&&strcmp(ch_ip,ip)==0&&strcmp(ch_pwd,pwd)==0)
    {
          flag=1;
          break;
    }
    */

    if(ch_uid==uid&&strcmp(ch_pwd,pwd)==0)
    {
          flag=1;
          break;
    }


}

in.close();

ofstream out;
out.open("current_login.txt",ios_base::app|ios_base::out);
out<<uid<<" "<<ip<<":"<<port<<endl;
out.close();

send(newid,&flag,sizeof(flag),0);
  
}


void *reqhandler(void* temp)
{

  int newid = *(int *)temp;
  
  char command[20];
   
  recv(newid,command,sizeof(command),0);

  cout<<command<<endl;

  if(strcmp(command,"create_user")==0)
  {
      create_user(newid);
  }
  else if(strcmp(command,"login")==0)
  {
      login(newid);
  }
  else if(strcmp(command,"create_group")==0)
  {
      create_group(newid);
  }
  else if(strcmp(command,"join_group")==0)
  {
      //cout<<"\njoin group called at tracker\n";  
  
      join_group(newid);
  }
  else if(strcmp(command,"list_requests")==0)
  {
      join_group(newid); //as tracker has to do same task in both
  }
  else if(strcmp(command,"accept_request")==0)
  {
      accept_request(newid);
  }
  else if(strcmp(command,"list_groups")==0)
  {
      list_groups(newid);
  }
  else if(strcmp(command,"leave_group")==0)
  {
      leave_group(newid);
  }
  else if(strcmp(command,"logout")==0)
  {
      logout(newid);  
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
out.open("current_login.txt",ios_base::trunc);
 
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

