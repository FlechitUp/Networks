/* Client code in C
 * g++ -o client2 client2.cpp -lpthread -std=c++11
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <thread>
#include <vector>
#include <errno.h>

std::vector<std::thread> T; //
struct sockaddr_in stSockAddr; //
int Res; //
int SocketFD ; //
char buffer[5]; //

std::string fillZeros(int aux_size,int nroBytes){ // complete number with zeross =)
	std::string aux = std::to_string(aux_size);
	int dif = nroBytes - int(aux.size());
	for (int i = 0; i < dif; i++)
		aux = "0" + aux;
	return aux;
}

void read2(int SocketFD, char *buffer) {
	int n;
	for (;;){
		bzero(buffer, 5);
		do {
			n = read(SocketFD, buffer, 4); // Reading first 4 bytes
			if(n==0 /*&& T[1].joinable()==false*/){//FINALIZANDO CONEXION
				return;
			}
			int size_msg=atoi(buffer);
			bzero(buffer, 4); // Zeros for the 4 bytes that was reading

			n = read(SocketFD, buffer, 1); //reading 1 bytes
			std::string action(buffer);
			bzero(buffer, 1); //equal to the before

			if (action == "R"){ // Responsive when is Printing or Chating or error in Login
				char msg[size_msg+1];
				n = read(SocketFD, msg, size_msg);
				msg[size_msg]=0;
				printf ("[%s]\n", msg);
			} else if (action == "D"){//Responsive when is file
				n=read(SocketFD, buffer,2);
				int size_othername=atoi(buffer);
				bzero(buffer,2);
				char othernameBuffer[size_othername+1];
				othernameBuffer[size_othername]=0;
				n=read(SocketFD,othernameBuffer,size_othername);
				std::string othername(othernameBuffer);	//othername
				char filenameBuffer[size_msg+1];
				filenameBuffer[size_msg]=0;
				n=read(SocketFD,filenameBuffer,size_msg);
				std::string msg(filenameBuffer);	//filename
				std::cout << othername << " te envió: " << msg << std::endl;
				n=read(SocketFD, buffer, 4);
				int size_file=atoi(buffer);
				bzero(buffer,4);
				char msg_file[size_file];
				n=read(SocketFD,msg_file,size_file);
				FILE *newFile=fopen(("sd"+msg).c_str(),"w");
				for (int i=0;i<size_file;i++){
					fprintf(newFile, "%c", msg_file[i]);
				}
				fclose(newFile);
			}

			// n = read(SocketFD, buffer, atoi(buffer));

		} while (n == 0);
	}
}

bool existContent(std::string &filename, std::string &file){
	FILE *file_content;
	if(!(file_content=fopen(filename.c_str(),"r+"))){
		std::cout << "the file does not exist\n" << std::endl;
		return false;
	}
	char c;
	fscanf(file_content,"%c",&c);
	while(!feof(file_content)){
		file+=c;
		fscanf(file_content,"%c",&c);
	}
	fclose(file_content);
	return true;
}

void write2(int  SocketFD) {
	std::string msg , aux = "", op = "";
	int dif = 0;

	while (1) {
		msg="";
		std::cout << "------Menu (action)-----\n"
			 << "I -> Print list of user on the chat \n"
			 << "L -> Login to the char\n"
			 << "M -> Send a msg to a user on the chat\n"
			 << "O -> End chat or logout from chat\n"
			 << "M -> Send chat to any users on the chat\n"
			 << "F -> Send a file from a user to another user\n"
			 << "----------------------------\n"
			 << std::endl;
		std::cin >> op;
		std::vector<std::string> users;


		if (op == "I") {// protocolo for Print

			//Protocolo:
			msg = 	std::string("0000")+	// size of msg
				"I";			// P
				users.push_back(msg);

		} else if (op == "L"){//protocolo for Login

			std::string nickname = "";
			std::cout << "enter nickname: ";
			std::cin.ignore();
			getline(std::cin, nickname); // scann with spaces
			//Protocolo:
			msg=	fillZeros(nickname.size(),4)+	// size of Nickname(4)
				"L"+				// L
				nickname;			// nickname

				users.push_back(msg);

		} else if (op == "M")	{ //protocolo for Chat

			std::string nickname="";
			std::cout<<"enter nickname to chat: ";
			std::cin.ignore();
			getline(std::cin, nickname); //scan with spaces
			std::cout<<"enter message: ";
			//std::cin.ignore();
			getline(std::cin,msg); //scan with spaces
			//Protocolo:
			msg=	fillZeros(msg.size(),4)+	// size of msg(4)
				"M"+ 				// C
				fillZeros(nickname.size(),2)+	// nickname size(2)
				nickname+			// nickname
				msg;				// msg
				users.push_back(msg);


		} else if (op == "O"){ // protocolo for End

			//Protocolo:
			msg = 	std::string("0000") +		// size of msg(4)
				"O";				// E
				users.push_back(msg);


		} else if (op == "F"){ // protocolo for File

			std::string file="",filename="",nickname="";
			std::cout << "enter nickname to send file: ";
			std::cin.ignore();
			getline(std::cin, nickname); // scann with spaces
			std::cout << "enter filename: ";
			getline(std::cin, filename); // scann with spaces
			if(!existContent(filename,file)){
				continue;
			}
			if(file.size()>9999){
				std::cout << "the file is very large(>9999)\n";
				continue;
			}
			//Protocolo:
			msg=	fillZeros(filename.size(),4) +	// size of filename(4)
				"F" +				// F
				fillZeros(nickname.size(),2) +	// nickname size(2)
				nickname +			// nickaname
				filename +			// filename
				fillZeros(file.size(),4) +	// file's size(4)
				file;				// file
				users.push_back(msg);


		} else if (op == "M")	{ //protocolo for Chat
			std::string nm="";
			std::cout << "Numero de usuarios a mandar el mensaje:" << '\n';
			std::cin.ignore();
			getline(std::cin, nm); //scan with spaces
			int size_=stoi(nm);
			std::string nickname="";
			std::string temp="";
			std::cout<<"enter message: ";
			//std::cin.ignore();
			getline(std::cin,msg); //scan with spaces

			for (int i = 0; i < size_; i++) {
				std::cout<<"enter nickname to chat: ";
				getline(std::cin, nickname); //scan with spaces
				//Protocolo:
				temp=fillZeros(msg.size(),4)+	// size of msg(4)
					"M"+ 				// C
					fillZeros(nickname.size(),2)+	// nickname size(2)
					nickname+			// nickname
					msg;				// msg
					users.push_back(temp);
					std::cout << "/* message */"<<temp << '\n';
			}}
			else{ // this can be better =/
			msg = "0000P";
			std::cout << "error action no found, enter other\n ";
			continue;
		}
		int nwrite;
		for (int i = 0; i < users.size(); i++) {
			nwrite = write(SocketFD, users[i].c_str(), int(users[i].size()));
		}
		if(op=="O"){
			return;
		}
	}

}

int main(){
	int n;

	SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (-1 == SocketFD)
	{
		perror("cannot create socket");
		exit(EXIT_FAILURE);
	}

	memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

	stSockAddr.sin_family = AF_INET;
	stSockAddr.sin_port = htons(8888);
	Res = inet_pton(AF_INET, "192.168.1.8", &stSockAddr.sin_addr);

	if (0 > Res) {
		perror("error: first parameter is not a valid address family");
		close(SocketFD);
		exit(EXIT_FAILURE);
	} else if (0 == Res) {
		perror("char string (second parameter does not contain valid ipaddress");
		close(SocketFD);
		exit(EXIT_FAILURE);
	} if (-1 == connect(SocketFD, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in))) {
		perror("connect failed");
		close(SocketFD);
		exit(EXIT_FAILURE);
	}
	T.resize(2);
	T[0]=(std::thread(read2, SocketFD, buffer));
	T[1]=(std::thread(write2, SocketFD));
	//T[1].join();
	T[0].join();
	shutdown(SocketFD, SHUT_RDWR);
	close(SocketFD);
	if(T[1].joinable()){
		perror("server is close");
		exit(EXIT_FAILURE);
	}
	return 0;
}
