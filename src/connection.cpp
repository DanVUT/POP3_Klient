#include "connection.h"

int bindSocket(std::string server_hostname, int port_number){
    struct hostent *server;
    struct sockaddr_in server_address;
    struct sockaddr_in test;
    char buffer[512];
    bzero(buffer, 512);
    int client_socket;
    int result = 0;
    struct addrinfo hints, *servinfo, *p;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    

    result = getaddrinfo(server_hostname.c_str(), (std::to_string(port_number)).c_str(), &hints, &servinfo);
    if(result){
        std::cerr << "ERROR: no such host as " <<  server_hostname << "\n";
        return ERR_SOCKET; 
    }
    for (p=servinfo; p != NULL; p = p->ai_next) {
        if ((client_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        }
        
        if (connect(client_socket, p->ai_addr, p->ai_addrlen) == -1) {
            close(client_socket);
            continue;
        }
        

        break;
    }
    char tmp[256];    
    if(p == NULL){
        std::cerr << "ERROR connecting to the server\n";
        return ERR_SOCKET;
    }
    freeaddrinfo(servinfo);

    /*
    if ((server = gethostbyname(server_hostname.c_str())) == NULL) {
        std::cerr << "ERROR: no such host as " <<  server_hostname;
        return ERR_SOCKET;
    }


    bzero((char *) &server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&server_address.sin_addr.s_addr, server->h_length);
    server_address.sin_port = htons(port_number);
    */

    /*
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) <= 0)
    {
        std::cerr << "ERROR creating socket";
        return ERR_SOCKET;
    }

    if (connect(client_socket, (const struct sockaddr *) &server_address, sizeof(server_address)) != 0)
    {
        std::cerr << "ERROR connecting to the server";
        close(client_socket);
        return ERR_SOCKET;
    }
    host = server->h_name;
    */
    //socklen_t len = sizeof (test);
    //getpeername(client_socket, (struct sockaddr*) &test, &len);
    //std::cout << inet_ntoa(test.sin_addr) << "\n";
    return client_socket;
}

SSL* bindSSLSocket(int client_socket, std::string certfile, std::string certaddr){
    SSL_CTX* ctx;
    SSL*     ssl;
    const SSL_METHOD *meth;


    OpenSSL_add_ssl_algorithms();
    meth = SSLv23_client_method();
    ctx = SSL_CTX_new (meth); 
    
    if(certfile == "" && certaddr == ""){
        SSL_CTX_set_default_verify_paths(ctx);
    }
    else{
        if(certfile == "" && certaddr != ""){
            if(!(SSL_CTX_load_verify_locations(ctx, NULL, certaddr.c_str()))){
                std::cerr << "ERROR Certification directory not found\n";
                return NULL;
            }
        }
        else if(certfile != "" && certaddr == ""){
            if(!(SSL_CTX_load_verify_locations(ctx, certfile.c_str(), NULL))){
                std::cerr << "ERROR Certificate not found\n";
                return NULL;
            }
        }
        else{
            if(!(SSL_CTX_load_verify_locations(ctx, certfile.c_str(), certaddr.c_str()))){
                std::cerr << "ERROR Certificate and certificate directory not found\n";
                return NULL;
            }
        }
    }

    
    ssl = SSL_new (ctx);                    
    if(!(SSL_set_fd (ssl, client_socket))){
        std::cerr <<  "ERROR binding SSL to fd\n";
        return NULL;
    }
    if((SSL_connect (ssl)) <= 0){
        std::cerr << "ERROR SSL Handshake not successful\n";
        return NULL;
    }
    if((SSL_get_verify_result(ssl)) != X509_V_OK){
        std::cerr << "ERROR Could not verify server certificate with provided file/directory\n";
        return NULL;
    }
    return ssl;
}