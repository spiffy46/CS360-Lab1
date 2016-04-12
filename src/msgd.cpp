#include<errno.h>
#include<stdio.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<iostream>
#include<string.h>
#include<sstream>
#include<map>
#include<vector>
#include<algorithm>

using namespace std;

int server;
int buflen = 1024;
char* buffer = new char[buflen+1];
map<string, vector<string> > emails;

void printError(){
	cout << "Invalid format" << endl;
}

bool send_Response(int client, string r){
	const char* ptr = r.c_str();
	int nleft = r.length();
	int nwritten;

	while(nleft){
		if((nwritten = send(client, ptr, nleft, 0)) < 0){
			if(errno == EINTR){
				continue;
			}else{
				perror("write");
				return false;
			}
		}else if(nwritten == 0){
			return false;
		}
		nleft -= nwritten;
		ptr += nwritten;
	}
	return true;
}

string get_Request(int client){
	string request = "";
	while(request.find("\n") == string::npos){
		int read = recv(client,buffer,1024,0);
		if(read < 0){
			if(errno == EINTR){
				continue;}
			else{
				return "";}
		}else if (read == 0){
			return "";
		}
		request.append(buffer,read);
	}

	if(request[0] == 'p'){
		stringstream ss;
		ss << request;
		string dummy;
		int c;
		ss >> dummy;
		ss >> dummy;
		ss >> dummy;
		ss >> c;
		while(request.substr(request.find('\n')+1).length() != c){
			int read = recv(client, buffer, 1024, 0);
			if(read < 0){
				if(errno == EINTR)
					continue;
				else
					return "";
			}else if (read == 0){
				return "";
			}
			request.append(buffer,read);
		}
	}
	return request;
}

string handlePut(string r){
	vector<string> v;
	string name = "";
	string subject = "";
	int length;
	
	string test = r.substr(0,r.find('\n'));

	if(count(test.begin(), test.end(),' ') != 3){
		return "error invalid put format\n";
	}
	
	stringstream ss;
	ss << r;
	ss >> name;
	ss >> name;
	ss >> subject;
	ss >> length;

	string ans = subject + " " + r.substr(r.find('\n')+1);
	
	map<string, vector<string> >:: iterator it = emails.find(name);

	if(it != emails.end()){
		v = emails[name];
		v.push_back(ans);
		emails[name] = v;
	}else{
		v.push_back(ans);
		emails[name] = v;
	}
	
	return "OK\n";
}

string handleList(string r){
	vector<string> v;
	string name = "";

	if(count(r.begin(), r.end(), ' ') != 1){
		return "error invalid list\n";
	}
	stringstream ss;
	ss << r;
	ss >> name;
	ss >> name;

	map<string, vector<string> >:: iterator it = emails.find(name);

	if(it != emails.end())
	{	
		v = emails[name];
	}else
		return "list 0\n";

	ss.str(std::string());
	ss << v.size();
	string ans = "list " + ss.str() + '\n';
	for(int i = 0; i < v.size(); i++){
		ss.str(std::string());
		ss << v[i];
		ss >> name;
		ss.str(std::string());
		ss << (i+1);
		ans = ans + ss.str() + " " + name + '\n'; 
	}

	return ans;
}

string handleGet(string r){
	vector<string> v;
	string name;
	string ans;
	int index;
	stringstream ss;
	ss << r;
	ss >> name;
	ss >> name;
	ss >> index;

	map<string, vector<string> >:: iterator it = emails.find(name);

	if(it != emails.end())
	{
		v = emails[name];
	}else
		return "error no such message for that user\n";

	if(index > v.size()|| index < 1){
		return "error no such mesage for that user\n";
	}else{
		string message = v[index-1];
		message = message.substr(message.find(' ')+1);
		ss.str(std::string());
		ss << v[index-1];
		string subject;
		ss >> subject;
		ss.str(std::string());
		ss << message.length();
		ans = "message " + subject + " " + ss.str() + '\n' + message; 
	}
	return ans;
}

string handleReset(string r){
	emails.clear();
	return "OK\n";
}

void handle(int client){
	while(1){
		string response = "";
		string request = get_Request(client);
	
		if(request.empty()){
			send_Response(client, "error\n");	
			break;
		}
		
		stringstream ss;
		ss << request;
		string command;
		ss >> command;

		char lead = command[0];		

		switch(lead){
			case 'p':
				response = handlePut(request);
				break;
			case 'l':
				response = handleList(request);
				break;
			case 'g':
				response = handleGet(request);
				break;
			case 'r':
				response = handleReset(request);
				break;
			default:
				response = "error invalid command: should never get here :/\n";	
				break;
		}

		bool success = send_Response(client, response);
		if(!success)
			break;
	}
	close(client);
}

void serve(){
	int client;
	struct sockaddr_in clientAddress;
	socklen_t clientlen = sizeof(clientAddress);

	while((client = accept(server,(struct sockaddr *)&clientAddress,&clientlen))>0){
		handle(client);
	}

	close(server);
}

void create(int port){
	struct sockaddr_in server_addr;

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	server = socket(PF_INET, SOCK_STREAM, 0);
	if(server < 0){
		perror("socket");
		exit(-1);
	}

	int reuse = 1;
	if(setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0){
		perror("setsockopt");
		exit(-1);
	}	

	if(bind(server,(const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
		perror("bind");
		exit(-1);
	}

	if(listen(server, SOMAXCONN) < 0){
		perror("listen");
		exit(-1);
	}
}

int main(int argc, char *argv[]){
	bool debug = false;	

	if(argc < 3 || argc > 4)
	{
		printError();
		return 0;
	}

	if(argc == 4){
		if(argv[3] == "-d"){
			debug = true;
		}
	}
	int port = atoi(argv[2]);

	create(port);
	serve();
	return 0;	
}
