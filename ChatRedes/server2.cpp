  /* Server code in C */
 /*Compile
  *  g++ -o server2 server2.cpp -lpthread -std=c++11
  */
  
  #include <sys/types.h>
  #include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  //write()
  #include <iostream>
      
#include <thread>        
#include <chrono>         
#include <string>
#include <sstream>
#include <cstdlib>
#include <mutex>
#include <condition_variable>
#include <map>

#include <stdio.h>  //sprintf

#define port 1300

// int nbites =4;

using namespace std;

map<string,int> clients;  //string , socket
map<string,int>::iterator it;


void processChatClient_thread(int socketCliente){ 
    char buffer[1000];
    cout<<"Here2"<<endl;
    for(;;){
        //buffer[0] ="X";

        int n = read(socketCliente,buffer,1);
        if( n == 0){
            continue;
        }

        if(buffer[0] == 'L'){
            int size_nickname = atoi(buffer);
            int n = read(socketCliente,buffer,size_nickname);
            buffer[size_nickname] = '\0';
            clients[buffer]=socketCliente;        
            write(socketCliente,"A10",3);    // a10 ponerlo em la funcion
        }
        else if (buffer[0] == 'i'){   //send the nicknames

             write(socketCliente, "i",1);
             sprintf(buffer,"%02d",(int)clients.size());
             write(socketCliente,buffer,2);

            for(it = clients.begin(); it!= clients.end(); ++it){
                sprintf(buffer,"%02d",(int)clients.size());
                write(socketCliente, buffer,2);  
                cout<<it->first.size();
                //write(socketCliente, it->first, it->first.size());                         
            }        
        }   
        else if(buffer[0] == 'A'){
        
        }
    
    }
    //this_thread::sleep_for(std::chrono::seconds(n));
    //cout << "pause of " << n << " seconds ended\n";
}


string convertirHeader(string mensajeIngresado,int nroBytes)
{  int dif;
   string st="";
  string aux = to_string(mensajeIngresado.size());
     dif = nroBytes - int(aux.size());
     //st = aux + st;
     st=aux;
     for (int i = 0; i < dif; i++)
          {st = "0" + st;}
  
  return st;
}
void fillZeros(string & st, int nroBytes){ // complete number with zeross =)
  string aux = to_string(st.size());
  int dif = nroBytes - int(aux.size());
  st = aux + st;
  for (int i = 0; i < dif; i++)
    st = "0" + st;
}  
  void writeCC(int ConnectFD, string mssg,char act)
{

  if (act == 'P' or act == 'C' or act == 'L'  or act == 'E' or act == 'F' ) // L is when a nickname is repeat 
  {
    mssg = "R" +mssg;
    fillZeros(mssg,4);
    write(ConnectFD, mssg.c_str(), mssg.size());
  }
  else if ( act='D'){
  	cout<<"en download:"<<mssg<<endl;
  	write(ConnectFD, mssg.c_str(), mssg.size());


  }


}
 bool find_client(string name)
    {
        for (std::map<string,int>::iterator it=clients.begin(); it!=clients.end(); ++it){
            if(it->first == name){
                 return true;
             }
        
            else
                return false;
        }
}
string get_name_client(int socket){
        for (std::map<string,int>::iterator it=clients.begin(); it!=clients.end(); ++it)
            if(it->second == socket) return it->first;
        return "-";
}
void  ejecutar_operacion(char operation,int size_msg,int SocketFD)
  {   int n;
        char *message_buffer;
        string msgO;

        if(size_msg == 0){
            if(operation == 'P')
            {
                //return Messsage(source_socket, prepare_simple_response(get_clients()));
                string prnt="";
                map<string, int>::iterator it;

                for (it = clients.begin(); it != clients.end(); it++){
                     prnt+="username:  "+it->first+" value "+"\n";
                     cout<<"socket->"<<it->second<<endl;
                }
                if(prnt=="") prnt="not is conected";

                writeCC(SocketFD,prnt,operation); 
            }

            if(operation == 'E'){
              map<string, int>::iterator it,itTMP;
               for (it=clients.begin(); it!=clients.end(); ++it)
                      { if(it->second == SocketFD) itTMP=it;}
                             
              msgO=itTMP->first+"  left the chat";

              clients.erase(itTMP->first);
              //cout<<"se elimino "<<itTMP->first;
               for (it=clients.begin(); it!=clients.end(); ++it)
                  {    
                     writeCC(it->second,msgO,operation); 

                  }
              //writeCC(SocketFD,msgO,operation); 
          
            }
        }
        
        if(operation == 'L'){
            message_buffer = new char[size_msg];
            n = read(SocketFD, message_buffer, size_msg);
            //si este usuario solo se esta cambiando su nombre 
            string nickname=string(message_buffer);
            string currentName=get_name_client(SocketFD);

             if ((currentName=="-") or (currentName==nickname))
             {
              clients[message_buffer]=SocketFD;
              msgO="You are logged now";
              writeCC(SocketFD,msgO,operation); 
 
             }
             else{
               clients.erase(currentName);
               clients[message_buffer]=SocketFD;
               msgO="You change your nickname";
               writeCC(SocketFD,msgO,operation);  

             }
            
        }
        else if(operation == 'C'){

                    message_buffer = new char[2];//tamano del nickname 
                    n = read(SocketFD, message_buffer, 2);
                    int dest_name_size = atoi(message_buffer);//tamano del nickname 
                    //With that read for who is the message
                    message_buffer = new char[dest_name_size];//nickname
                    n = read(SocketFD, message_buffer, dest_name_size);
                    string dest_name = string(message_buffer);//nickname

                    //cout<<"nickname: "<<dest_name<<"|"<<endl;

                    int dest_socket = clients[dest_name]; //get te socket by the name
                    //cout<<"nickname false:"<<dest_socket<<endl;

                   //string name = get_name_client(SocketFD);
                    //if((clients.find(name)->second) == 0)
                    if(dest_socket==0)
                    {    msgO="Message not sent";
                         writeCC(SocketFD,msgO,operation);
                        //return Messsage(source_socket, prepare_simple_response("Message not sent"));
                    }
                    else{
                        message_buffer = new char[size_msg];
                    
                        //now read the message
                        n = read(SocketFD, message_buffer, size_msg);

                        string message = get_name_client(SocketFD) +"     says: \n" + string(message_buffer);
                        writeCC(dest_socket,message,operation);
                        //return Messsage(dest_socket, prepare_simple_response(message));
                    }
            
             }         
        else if(operation == 'F'){

                    cout<<"I'm reading the file\n";
                    //Read Two First Bytes el tamaño de nick del que envio
                    message_buffer = new char[2];
                    n = read(SocketFD, message_buffer, 2);
                    int dest_name_size = atoi(message_buffer);
                    //With that read for who is the message

                   //leemos el nombre o el nick
                    message_buffer = new char[dest_name_size];
                    n = read(SocketFD, message_buffer, dest_name_size);
					string dest_name = string(message_buffer);
					
					string name=get_name_client(SocketFD);//nombre del cliente quien envia 
                    cout<<"name->"<<name<<endl;


                   if((clients.find(dest_name)->second) == 0)
                    {   //le digo al que envia no se envio
                    	msgO="   Message not sent";
                        writeCC(SocketFD,msgO,operation);
                        //return Messsage(source_socket, prepare_simple_response("Message not sent"));
                    }
                   else{
                   	    string message;
                   	    //read file name 
                   	    message_buffer = new char[size_msg];
                        n = read(SocketFD, message_buffer,size_msg);
					    string fileName = string(message_buffer);

					    //Read size file 
                   		 message_buffer = new char[4];
                   		 n = read(SocketFD, message_buffer, 4);
                   		 int size_file = atoi(message_buffer);

                   		  //read file  
                   	    message_buffer = new char[size_file];
                        n = read(SocketFD, message_buffer,size_file);
					    string fileBinary= string(message_buffer);
                        
                        //SEND FRON DESTINET ENVIAR A SU DESTINO Y GUARDAR
                        
                        int sockDESTINO=clients.find(dest_name)->second;
                        //aqui le falta un chachito
                        //FILENAME size and action
                        message=convertirHeader(fileName,4)+"D"+convertirHeader(dest_name,2)+dest_name+fileName;
                        message=message+convertirHeader(fileBinary,4)+fileBinary;

                        writeCC(sockDESTINO,message,'D');

                        //DECIRLE AL QUE ENVIO QUE SE ENVIO 

                        message = get_name_client(SocketFD) + "    sent file ";
                        writeCC(SocketFD,message,operation);
                        //return Messsage(dest_socket, prepare_simple_response(message));
                    }

    }
  }
  void readC(int SocketP)//char buffer[]) //leer 
  { 
      char buffer[4];bzero(buffer,4);
      int n=0;
      n = read(SocketP, buffer, 4);
      if (n>0){
        int size_message = atoi(buffer);
        char buffer_op[1];bzero(buffer_op,1);
            //now read operationb
            n = read(SocketP, buffer_op, 1);
            ejecutar_operacion(buffer_op[0],size_message,SocketP);
      }

    if(n < 0)
    perror("Error de lectura del socket");
    
  }

  void writeC(int SocketP)
  {
    int n;
    string st, aux = "";
     int dif = 0;
     cout<<">> :";
     getline(cin, st);
     aux = to_string(st.size());
     dif = 4 - int(aux.size());
     st = aux + st;
     for (int i = 0; i < dif; i++)
          {st = "0" + st;}
   // cout<<"envia "<< st<<endl;*
    n = write(SocketP,st.c_str(),st.size());

    if(n < 0)
      perror("Error escribiendo a socket");
    /*char msg[256];
    int n=0;
    cin.getline(msg,256);
    n = write(SocketP,msg,256);

    if(n < 0)
      perror("Error escribiendo a socket");*/

  }

  void clientes(int SocketP, int i)
  {
    char buffer[256];// este tiene que ser dinamico;
    char *mensaje2;
    int n=0;
    char msg[256];

    do{
        bzero(buffer,256);
        thread rd(readC, SocketP);
        rd.join();
       /* cout<<endl;
        cout << "Cliente " <<i<<" dice: "<<buffer<< endl;
        
        thread wt(writeC, SocketP);
        cout << "Escribe servidor:  ";
        wt.join();*/
         //rd.join();
 
    }while (1);

    shutdown(SocketP, SHUT_RDWR);
    close(SocketP);
  }

  //*********************************************************************

  int main(void)
  {
    struct sockaddr_in stSockAddr, client_addr;
    int SocketFD = socket(PF_INET, SOCK_STREAM, 0);//IPPROTO_TCP);
    char buffer[256];
    char msg[256];
    int n;
 
    if(-1 == SocketFD)
    {
      perror("can not create socket");
      exit(EXIT_FAILURE);
    }
 
    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
 
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(port);
    stSockAddr.sin_addr.s_addr = INADDR_ANY; // aqui indica una sola tarjeta de red
 
    if(-1 == bind(SocketFD,(const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
    {
      perror("error bind failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
 
    if(-1 == listen(SocketFD, 10))
    {
      perror("error listen failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }


    socklen_t len;
    int i=1;
 
    for(;;){   
        len = sizeof(client_addr);
        cout<<"here1 "<<len<<endl;
        int ConnectFD = accept(SocketFD, (struct sockaddr *)&client_addr, &len);  //cuando el cliente se conecta se cnecta con el accept y el socket descriptor que me va a indicar la conexion, socket del servidor
        //cout<< ConnectFD <<endl;
//      for(;;){ 
          if(ConnectFD < 0){
              perror("error accept failed");
              close(SocketFD);
              exit(EXIT_FAILURE);              
          }               
          else{
              cout<<"KHA PASO"<<endl;
              printf("Connected client %d\n",i);
              //thread(clientes,ConnectFD,i).detach();
              thread(processChatClient_thread,clients.size()).detach();
              i++;
          } 
          //cout<<"s"<<endl;
        //thread(processChatClient_thread,clients.size()).detach();
    }
 
    close(SocketFD); //se cierra el socket del servidor
    return 0;
  }
