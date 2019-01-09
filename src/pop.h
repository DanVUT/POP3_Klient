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

int myRecv(int client_socket, char * buffer, int load, int flag);
int mySend(int client_socket, const char * command, int length, int flag);
std::string recieveResponse();
bool sendCommand(std::string command);
bool isERR(std::string response);
bool getWelcomeMessage();
int authorize(int client_socket, std::string authfile);
std::string getUsername(std::string authfile);
std::string getPassword(std::string authfile);
int getMails(std::string outdir);
bool getIDS();
int getAmount(std::string response);
int recieveAllMessages(int noMails, std::string outdir);
int recieveMessage(int number, std::string outdir);
int parseBytes(std::string bytesString);
std::string recieveMail(int bytes);
std::string getMessageID(std::string email);
void putInfo(std::string usersFile);