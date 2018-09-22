#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <string> 
#include <thread>  
#include <sstream> 
#include <fstream>

using namespace std;

int port = 1300;

void open_file(string name , int size, char *buffer)
{
  std::ofstream outfile (name,std::ofstream::binary);

  outfile.write (buffer,size);

  delete[] buffer;

  outfile.close();
}

void readC(int SocketFD) //leer 
  { 
     char *message_buffer,*msg;

    char buffer[4],buffer_op[1];
    int n=0;
     
    message_buffer=new char[4];
		n = read(SocketFD,message_buffer, 4); // Reading first 4 bytes
		int size_msg=atoi(message_buffer);

    message_buffer=new char[1];
		n = read(SocketFD, message_buffer, 1); //reading 1 bytes
		string action=string(message_buffer);


		if (action == "R"){ // Responsive when is Printing or Chating or error in Login
			msg=new char[size_msg];
      n = read(SocketFD, msg,size_msg);
			cout<<"mensaje--->"<<msg<<endl;
		}
}

string convertirHeader(string mensajeIngresado,int nroBytes)
{  int dif;
   string st="";
   string aux = to_string(mensajeIngresado.size());
     dif = nroBytes - int(aux.size());
     st=aux;
     for (int i = 0; i < dif; i++)
          {st = "0" + st;}
  return st;
}

void fillZeros(string & st, int nroBytes){ // complete number with zeross 
	string aux = to_string(st.size());
	int dif = nroBytes - int(aux.size());
	st = aux + st;
	for (int i = 0; i < dif; i++)
		st = "0" + st;
}

string menu_acciones(int SocketP)
{
 	string msg = "", aux = "", op = "";
	int dif = 0;
	cout<<endl;
	cout<<"enter accion";
	cin >> op;
	  if (op == "L"){//protocolo for Login
           
			string username = "",port,ip;
			cout << "Please enter your nickname: ";
			cin.ignore(); 
			getline(cin, username);
            cout << "Please enter your port: ";
			cin.ignore(); 
			getline(cin, port);
		       cout << "Please enter your ip: ";
			cin.ignore(); 
			getline(cin,ip);

			msg="L"+username+port+ip;  // msg final
			fillZeros(msg,4);

		}
		else if (op == "C")	{ //protocolo for Chat

			string otherIP = "",port,mensaje;
			cout << "Enter the ip dest: ";
			cin.ignore();
			getline(cin,otherIP);
			//cin>>othername;
			cout << "Enter the PORT DEST: ";
			cin.ignore();
			getline(cin,port);

			cout<<"mensaje ingresado "<<mensaje<<endl;
			msg="C"+otherIP+port+mensaje;
			fillZeros(msg,4);
		}
		
	return msg;
}

void writeC(int SocketP)
{  
  int n=0;
  string st;
  st=menu_acciones(SocketP);

  //cout<<"After menu actions "<<endl;
  n = write(SocketP,st.c_str(),st.size());
  cout<<"sending ---"<<endl;

  if(n < 0)
    perror("Error writting to socket");
}
void checkReadC(int SocketFD){
    
    char *message_buffer;
    char buffer[4];
    bzero(buffer,4);
    int n = read(SocketFD,buffer,4);
    bool first_game = true;
    do{
        if (n>0){
                  int size_message = atoi(buffer);
                  char buffer_op[1];
                  //now read operation
                  n = read(SocketFD, buffer_op, 1);
                  if(buffer_op[0] == 'R'){
                      message_buffer = new char[size_message];
                      n = read(SocketFD, message_buffer, size_message);
                      cout << "Received  :" << message_buffer << endl;
                  }
            }

        bzero(buffer,4);
        n = read(SocketFD,buffer,4);
    }while(true);
    
}

int chat(int SocketFD)
{
    cout<< ".......................:D" << endl;
    //thread rdcheck(checkSend,SocketFD).detach();
  std::thread(checkReadC, SocketFD).detach();
  cout<< ".......................:D" << endl;
    do
    {
       thread write(writeC,SocketFD);
       write.join();

    }while (1);
    shutdown(SocketFD, SHUT_RDWR);
    close(SocketFD);
}
 
//*******************************************************************************************************************

  int main(void)
  {
    //cout<<"Hola";
    
    struct sockaddr_in stSockAddr;
    int Res;
    int SocketFD = socket(PF_INET, SOCK_STREAM,0); // IPPROTO_TCP);
    int n;

    if (-1 == SocketFD)
    {
      perror("cannot create socket");
      exit(EXIT_FAILURE);
      return 0;
    }
 
    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
 
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(port);
    //Res = inet_pton(AF_INET, "192.168.8.101", &stSockAddr.sin_addr);
    //Res = inet_pton(AF_INET, "127.0.0.1", &stSockAddr.sin_addr);
    Res = inet_pton(AF_INET,"192.168.1.2", &stSockAddr.sin_addr);
   // Res = inet_pton(AF_INET, "192.168.160.198", &stSockAddr.sin_addr);
 
    if (0 > Res)
    {
      perror("error: first parameter is not a valid address family");
      close(SocketFD);
      exit(EXIT_FAILURE);
      return 0;
    }
    else if (0 == Res)
    {
      perror("char string (second parameter does not contain valid ipaddress");
      close(SocketFD);
      exit(EXIT_FAILURE);
      return 0;
    }
 
    if (-1 == connect(SocketFD, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))  //conectarme con el servidor
    {
      perror("connecte failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
      return 0;
    }
   chat(SocketFD); //cierra el recurso 

    return 0;
  }
