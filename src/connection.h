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
#include <openssl/ssl.h>
#include "globvars.h"
#include "errors.h"
#include "pop.h"



int bindSocket(std::string server_hostname, int port_number);
SSL* bindSSLSocket(int client_socket, std::string certfile, std::string certaddr);
