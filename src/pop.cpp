#include "pop.h"

int myRecv(int client_socket, char *buffer, int load, int flag){
    if(tls || pop3s){
        return SSL_read(ssl, buffer, load);
    }
    else{
        return recv(client_socket, buffer, load, 0);
        
    }
}

int mySend(int client_socket, const char * command, int length, int flag){
    if(tls || pop3s){
        return SSL_write(ssl, command, length);
    }
    else{
        return send(client_socket, command, length, 0);
        
    }
}

std::string recieveResponse(){
    char letter[2];
    bzero(letter, 2);
    std::string response("");
    int res = 0;
    while (1){
        res = myRecv(client_socket, letter, 1,0);
        if (res <= 0)
            break;
        response = response + letter;
        if(letter[0] == '\r'){
            bzero(letter, 2);
            res = myRecv(client_socket, letter, 1,0);
            if (res <= 0)
                break;
            response = response + letter;
            if(letter[0] == '\n'){
                return response;
            }
        }
        bzero(letter, 2);
    }
    return response;
}

bool sendCommand(std::string command){
    command += "\r\n";
    int sent;
    sent = mySend(client_socket, command.c_str(), command.length(), 0);
    return sent == command.length();
}

bool isERR(std::string response){
    return response[0] != '+';
}

bool getWelcomeMessage(){
    std::string response("");
    response = recieveResponse();
    return isERR(response);
}

std::string getUsername(std::string authfile){
    std::ifstream pFile(authfile);
    std::string usernameLine("");
    if(!pFile.is_open()){
        std::cerr << "AUTH FILE NOT FOUND\n";
        return "";
    }
    std::getline(pFile, usernameLine);
    char tmp[255];
    std::sscanf(usernameLine.c_str(), "username = %s", tmp);
    std::string username("USER ");
    pFile.close();
    tmpUsername = tmp;
    return username + tmp;
}

std::string getPassword(std::string authfile){
    std::ifstream pFile(authfile);
    std::string passwordLine("");
    if(!pFile.is_open()){
        std::cerr << "AUTH FILE NOT FOUND\n";
        return "";
    }
    std::getline(pFile, passwordLine);
    std::getline(pFile, passwordLine);
    std::string password("PASS ");
    int i = 0;
    for(; passwordLine[i] != '='; i++){
    }
    i++;
    i++;
    for(; i<passwordLine.length(); i++){
        password += passwordLine[i];
    }
    return password;
}


int authorize(int client_socket, std::string authfile){
    std::string username("");
    std::string password("");
    std::string response("");
    username = getUsername(authfile);
    if(username == ""){
        std::cerr << "ERROR problem with getting a username from a file\n";
        return ERR_AUTH;
    }
    password = getPassword(authfile);
    if(password == ""){
        std::cerr << "ERROR problem with getting a password from a file\n";
        return ERR_AUTH;
    }



    sendCommand(username);                   //Odosle sa username
    response = recieveResponse();      //Prijem sa request na password od servera
    if(isERR(response)){
        std::cerr << response;
        return ERR_AUTH;
    }
    sendCommand(password);                   //Odosle sa heslo
    response = recieveResponse();                      //Prijme sa WELCOME response
    if(isERR(response)){
        std::cerr << response;
        return ERR_AUTH;
    }
    
    userHost = tmpUsername;
    usersFile = tmpUsername + ".cfg";
    return 0;
}    


int getMails(std::string outdir){
    int mails = 0;

    getIDS();
    std::string response("");
    sendCommand("STAT"); 
    response = recieveResponse();
    if(isERR(response)){
        std::cerr << response;
        return ERR_STAT;
    }
    mails = getAmount(response);                            //vyparsuje sa pocet sprav na serveri
    if(mails == -1){
        std::cerr << "ERROR STAT message in unexpected format\n";
        return ERR_STAT;
    }
    mails = recieveAllMessages(mails, outdir);
    return mails;
}

bool getIDS(){
    std::ifstream pFile;
    pFile.open(usersFile);
    if(!pFile.is_open()){
        return true;
    }
    std::string line("");
    while (std::getline(pFile, line)){
        if(line == userHost){
            std::getline(pFile, IDstring);
            pFile.close();
            return false;
        }
    }
    pFile.close();
    return true;
}

int getAmount(std::string response){
    int i=0;
    for(; response[i] != ' '; i++){}
    i++;

    std::string number("");
    for(; response[i] != ' '; i++){
        number += response[i];
    }
    try{
        i = std::stoi(number);
    }
    catch(std::invalid_argument){
        i = -1;
        std::cerr << "ERROR unsupported STAT message\n";
    }
    return i;
}

int recieveAllMessages(int noMails, std::string outdir){
    int recievedMails = 0;
    int tmp = 0;
    for(int i = 1; i <= noMails; i++){
        tmp = recieveMessage(i, outdir);
        if(tmp == 1){
            recievedMails++;
        }
        else if(tmp == -1){
            return ERR_RETR;
        }
        else{
            continue;
        }
    }
    return recievedMails;
}


int recieveMessage(int number, std::string outdir){
    int bytes = 0;
    std::string command = "RETR ";
    std::string response = "";
    std::string email = "";
    std::string messageID = "";

    bool willSave = true;
    int found = -1;
    std::ofstream pFile;
    
    sendCommand(command + std::to_string(number));
    response = recieveResponse();
    if(isERR(response)){
        std::cerr << response;
        return -1;
    }
    bytes = parseBytes(response);
    email = recieveMail(bytes);
    while(1){
        response = recieveResponse();
        if(response == ".\r\n" || response == "\n.\r\n"){
            if(response == "\n.\r\n"){
		email += "\n";
            }
            break;
        }
        else{
            email += response;
	}
    }
    found = -1;
    messageID = getMessageID(email);

    if(nwm && messageID != " "){
        found = IDstring.find(messageID);
        if((found > -1) || (messageID == IDstring)){

            willSave = false;
        }
    }
    if(willSave){
        if(messageID != " "){
            pFile.open(outdir + messageID + ".eml");
        }
        else{
            pFile.open(outdir + "noID" + std::to_string(nonID) + ".eml");
            nonID++;
        }
        if(!pFile.is_open()){
            std::cerr << "ERROR could not create file. output directory probably missing.\n";
            return -1;
        }
        pFile << email;
        pFile.close();
        if(nwm && messageID != " "){
            IDstring += messageID;
        }
        if(dlt){
            sendCommand(std::string("DELE ") + std::to_string(number));
            response = recieveResponse();
            if(isERR(response)){
                std::cerr << response;
                return -1;
            }
        }
    }
    return willSave;
}


int parseBytes(std::string bytesString){
    std::string intString = "";
    int index = 0;
    while (!std::isdigit(bytesString[index])){
        index += 1;
    }
    while (std::isdigit(bytesString[index])){
        intString += bytesString[index];
        index += 1;
    }
    try{
        index = std::stoi(intString);
    }
    catch(std::invalid_argument){
        index = -1;
    }
    return index;
}

std::string recieveMail(int bytes){
    char buffer[513];
    bzero(buffer, 513);
    int load;
    int total = 0;
    if(bytes > 512){
        load = 512;
    }
    else{
        load = bytes;
    }
    std::string email("");
    int recieved = 0;

    while (bytes > 0){
        recieved = myRecv(client_socket, buffer, load, 0);
        total += recieved;
        if (recieved <= 0){
            break;
        }
        email += buffer;
        bzero(buffer, 513);
        
        bytes -= recieved;
        
        if (bytes < 512){
            load = bytes;
        }
        
    }
    return email;

}

std::string getMessageID(std::string email){
    int index = 0;
    std::string messageID("");
    index = email.find("Message-Id: <");
    if(index < 0){
        index = email.find("Message-ID: <");
        if(index < 0){
            index = email.find("Message-Id:<");
            if(index < 0){
                index = email.find("Message-ID:<");
                if(index < 0){
                    return std::string(" ");
                }
                
            }
        }
    }
    while(email[index++] != '<'){}
    while(email[index] != '@') {
        messageID+=email[index];
        index++;
    }    

    return messageID + " ";
    
}

void putInfo(std::string usersFile){
    std::ofstream pFile;
    pFile.open(usersFile);
    if(!pFile.is_open()){
        return;
    }
    pFile << userHost << "\n" << IDstring;
    pFile.close();
}