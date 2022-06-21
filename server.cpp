#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <mutex>

#define MAX_LEN 200
#define NUM_COLORS 6

using namespace std;

struct terminal{
	int id;
	string name;
	int socket;
	thread th;
};

vector<terminal> clients;
string def_col="\033[0m";
string colors[]={"\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m","\033[36m"};
mutex cout_mutex,clients_mutex;
int seed=0;


void set_name(int id, char name[]);
void shared_print(string str, bool Line_ending);
void end_connection(int id);
void client_handling(int client_socket, int id);
int broadcast_message(string message, int sender_id);
int broadcast_message(int num, int sender_id);
string color(int code);

int main(){
	int server_socket;
	
	if((server_socket=socket(AF_INET,SOCK_STREAM,0))==-1){
	perror("socket: ");
	exit(-1);
	}
	
	struct sockaddr_in server;
	server.sin_family=AF_INET;
	server.sin_port=htons(10000);
	server.sin_addr.s_addr=INADDR_ANY;
	bzero(&server.sin_zero,0);
	
	if((bind(server_socket,(struct sockaddr *)&server,sizeof(struct sockaddr_in)))==-1){
	perror("The bind error: ");
	exit(-1);
	}
	
	if((listen(server_socket,8))==-1){
	perror("The listen error: ");
	exit(-1);
	}
	
	struct sockaddr_in client;
	int client_socket;
	unsigned int len=sizeof(sockaddr_in);
	cout << colors[NUM_COLORS-1]<<"\n\t|| ----------- WELCOME CHAT ROOM ----------- ||\n\n"<<def_col;
	
	while(1){
	
		if((client_socket=accept(server_socket,(struct sockaddr *)&client,&len))==-1){
		perror("The accept error: ");
		exit(-1);
		}
		
		seed++;
		thread t(client_handling,client_socket,seed);
		lock_guard<mutex> guard(clients_mutex);
		clients.push_back({seed, string("Anonymous"),client_socket,(move(t))});
	}
	
	for(int i=0; i<clients.size(); i++){
		if(clients[i].th.joinable()) {
		clients[i].th.join();
		}
	}
	
	close(server_socket);
	return 0;
}


// The function to Set the name of client
void set_name(int id, char name[]){
	for(int i=0; i<clients.size(); i++){
			if(clients[i].id==id) {
			clients[i].name=string(name);		
			}	
    }
}

// The function for synchronisation of cout statements
void shared_print(string str, bool Line_ending=true){	
	lock_guard<mutex> guard(cout_mutex);
	cout<<str;
	if(Line_ending) cout<<endl;
}

// The function to end the connection
void end_connection(int id){
	for(int i=0; i<clients.size(); i++){
		if(clients[i].id==id){
			lock_guard<mutex> guard(clients_mutex);
			clients[i].th.detach();
			clients.erase(clients.begin()+i);
			close(clients[i].socket);
			break;
		}
	}				
}

// The function for handling the client 
void client_handling(int client_socket, int id){
	char name[MAX_LEN],str[MAX_LEN];
	recv(client_socket,name,sizeof(name),0);
	set_name(id,name);	

	// Displaying the welcome message on entering the server
	
	string welcome_message=string(name)+string(" has banged into the server !");
	broadcast_message("#NULL",id);	
	broadcast_message(id,id);								
	broadcast_message(welcome_message,id);	
	shared_print(color(id)+welcome_message+def_col);
	
	while(true){
		int bytes_received=recv(client_socket,str,sizeof(str),0);
		if(bytes_received<=0)
			return;
		if(strcmp(str,"#exit")==0){
			// Displaying the leaving message on leaving the server
			
			string message=string(name)+string(" has left the server !");		
			broadcast_message("#NULL",id);			
			broadcast_message(id,id);						
			broadcast_message(message,id);
			shared_print(color(id)+message+def_col);
			end_connection(id);							
			return;
		}
		broadcast_message(string(name),id);					
		broadcast_message(id,id);		
		broadcast_message(string(str),id);
		shared_print(color(id)+name+" : "+def_col+str);		
	}	
}

// The function to Broadcast message to all clients except the sender
int broadcast_message(string message, int sender_id){

	char temp[MAX_LEN];
	strcpy(temp,message.c_str());
	
	for(int i=0; i<clients.size(); i++){
		if(clients[i].id!=sender_id) send(clients[i].socket,temp,sizeof(temp),0);
	}
	return 0;		
}

// The function to Broadcast a number to all clients except the sender
int broadcast_message(int num, int sender_id){
	for(int i=0; i<clients.size(); i++){
		if(clients[i].id!=sender_id) send(clients[i].socket,&num,sizeof(num),0);
	}
	return 0;		
}


//The string color
string color(int code){
	return colors[code%NUM_COLORS];
}
