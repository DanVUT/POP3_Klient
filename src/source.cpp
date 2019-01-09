#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <ctime>
#include <vector>
#include <thread>
#include <signal.h>
#include <fstream>
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/x509_vfy.h>
#include <openssl/bio.h>
#include <openssl/conf.h>

#include "arguments.h"
#include "connection.h"
#include "globvars.h"
#include "errors.h"
#include "pop.h"


int main(int argc, char* argv[]){
    int err = 0;
    int emailsRecieved = 0;
    std::string response = "";
    
    err = getArguments(argc, argv);
    if(err){
        return err;
    }

    client_socket = bindSocket(server_hostname, port_number);
    if(client_socket == ERR_SOCKET){
        return ERR_SOCKET;
    }

    if(tls || (!tls && !pop3s)){
        
        if(tls){
            tls = false;
            err = getWelcomeMessage();
            if(err){
                std::cerr << "ERROR problem with welcome message\n";
                return ERR_WELCOME;
            }
            sendCommand("STLS");
            response = recieveResponse();
            if(isERR(response)){
                std::cerr << "ERROR problem with starting TLS negotiation\n";
                return ERR_TLSNEG;
            }
            tls = true;
        }
        else{
            err = getWelcomeMessage();
        }
    }
    if(err){
        std::cerr << "ERROR problem with welcome message\n";
        return ERR_WELCOME;
    }

    if(tls || pop3s){
        ssl = bindSSLSocket(client_socket, certfile, certaddr);
        if(ssl==NULL){
            return ERR_SSLBIND;
        }
        if(pop3s){
            err = getWelcomeMessage();
            if(err){
                std::cerr << "ERROR problem with welcome message\n";
                return ERR_WELCOME;
            }
        }
    }


    err = authorize(client_socket, authfile);
    if(err){
        return ERR_AUTH;
    }
    emailsRecieved = getMails(outdir);
    if(emailsRecieved < 0){
        return emailsRecieved;
    }
    if(emailsRecieved >= 0){
        std::cout << emailsRecieved << " messages were recieved.\n";
    }
    putInfo(usersFile);
    sendCommand("QUIT");
    close(client_socket);

    return 0;
    
}