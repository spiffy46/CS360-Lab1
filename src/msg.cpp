#include<algorithm>
#include<errno.h>
#include<netdb.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<iostream>
#include<string.h>
#include<sstream>


using namespace std;

int server;
int buflen = 1024;
char* buffer = new char[buflen+1];
// Left OFF HERE v
bool sendRequest(string r){
	const char* ptr = r.c_str();
	int remaining = r.length();
	int total;

	while(remaining){
		if((total = send(server,ptr, remaining, 0)) < 0){
			if(errno == EINTR){
				continue;
			}else{
				perror("write");
				return false;
			}
		}else if(total == 0){
			return false;
		}
		remaining -= total;
		ptr += total;
	}
	return true;	
}

void send_option(){
	string user;
	string subject;
	string completeMessage;
	string line;

	cin >> user >> subject;

	cout << "- Type your message. End with a blank line -" << endl;

	cin.ignore();
	getline(cin, line);

	while(line != ""){
		completeMessage = completeMessage + line + '\n';
		getline(cin, line);
	}

	int len = completeMessage.size();
	stringstream ss;
	ss << len;
	string reqBody = "put " + user + " " + subject + " " + ss.str() + '\n' + completeMessage;

 	send(server, reqBody.c_str(), reqBody.length(), 0);

	memset(buffer, 0, buflen);
	recv(server, buffer, buflen, 0);
	
	string ans = buffer;
	while(ans.find('\n') == string::npos){
		memset(buffer, 0, buflen);
		recv(server, buffer, buflen, 0);
		ans = ans + buffer;	
	}

	if(ans[0] == 'e'){
		cout << ans;
	}
}

void list(){
	int c;
	string dummy;
	string user;
	cin >> user;

	string reqBody = "list " + user + '\n';

	send(server, reqBody.c_str(), reqBody.length(), 0);

	memset(buffer, 0, buflen);
	recv(server, buffer, buflen, 0);

	string ans = buffer; 
	while(ans.find('\n') == string::npos){
		memset(buffer, 0, buflen);
		recv(server, buffer, buflen, 0);
		ans = ans + buffer;
	}

	if(ans[0] == 'e'){
		cout << ans;
		return;
	}
	stringstream ss;
	ss << ans;

	ss >> dummy;
	ss >> c;	

	while(count(ans.begin(), ans.end(), '\n') != (c+1)){
		memset(buffer, 0, buflen);
		recv(server, buffer, buflen, 0);
		ans = ans + buffer;
	}
	
	cout << ans.substr(ans.find('\n')+1);
}

void read(){
	int c;
	string dummy;
	string user;
	string index;
	
	cin >> user >> index;

	string reqBody = "get " + user + " " + index + '\n';

	send(server, reqBody.c_str(), reqBody.length(), 0);
	
	memset(buffer, 0, buflen);
	recv(server, buffer, buflen, 0);

	string ans = buffer;

	while(ans.find('\n') == string::npos){
		memset(buffer, 0, buflen);
		recv(server, buffer, buflen, 0);
		ans = ans + buffer;
	}

	if(ans[0] == 'e'){
		cout << ans;
		return;
	}

	stringstream ss;
	ss << ans;
	ss >> dummy;
	ss >> dummy;
	ss >> c;
	
	int start = ans.find('\n');
	while(ans.substr(start+1).length() != c){
		memset(buffer, 0, buflen);
		recv(server, buffer, buflen, 0);
		ans = ans + buffer;
	}
	
	cout << dummy << endl << ans.substr(start+1);
}

void reset(){
}

void printErrorMsg(){
	cout << "Invalid Format" <<endl;
}

void create(string host, int port){
	struct sockaddr_in server_addr;

	struct hostent *hostEntry;
	hostEntry = gethostbyname(host.c_str());
	if(!hostEntry){
		cout << "No such host name: " << host << endl;
		exit(-1);
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	memcpy(&server_addr.sin_addr, hostEntry->h_addr_list[0], hostEntry->h_length);

	server = socket(PF_INET, SOCK_STREAM, 0);
	if(server < 0){
		perror("socket");
		exit(-1);
	}

	if(connect(server,(const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
		perror("connect");
		exit(-1);
	} 
}

int main(int argc, char * argv[]){
	bool debug = false;

	if(argc < 5 || argc > 6){
		printErrorMsg();
		return 0;
	}
	if(argc == 6){
		if(argv[5] == "-d"){
			debug = true;
		}
	}
	
	string host = argv[2];
	int port = atoi(argv[4]);	

	//Establish connection to server
	create(host, port);
	
	string input;
	string command;

	cout << "% ";


	cin >> input;

	while(input != "quit"){
		if(input == "send"){
			send_option();
		}else if(input == "list"){
			list();
		}else if(input == "read"){
			read();
		}else if(input == "reset"){
			reset();
		}else{
			cout << "I don't recognize that command." << endl;	
		}

		cout << "% ";
		cin >> input;
	}

	close(server);

	return 0;
}
