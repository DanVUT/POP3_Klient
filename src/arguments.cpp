#include "arguments.h"
#include <unistd.h>


std::string tmpUsername("");
std::string path("");
std::string host("");
std::string userHost("");
std::string usersFile("");
std::string IDstring("");
int nonID = 1;
int opt;
int client_socket = 0;
int port_number = -1;
SSL*     ssl = NULL;
bool pop3s = false;
bool tls = false;
bool dlt = false;
bool nwm = false;
std::string certfile = "";
std::string certaddr = "";
std::string authfile = "";
std::string outdir = "";
std::string server_hostname = "";


int getArguments(int argc, char** argv){


    int opt = 0;
    while( (opt = getopt(argc, argv, "p:TSc:C:dna:o:")) != -1 ){
        switch(opt){
            case 'p':
                try{
                    port_number = std::stoi(optarg);
                }
                catch(std::invalid_argument){
                    std::cerr << "Zle zadany port\n";
                    return ERR_ARGS;
                }
                break;
            case 'T':
                if(tls){
                    std::cerr << "ERROR Switches T and S combined\n";
                    return ERR_ARGS;
                }
                if(pop3s){
                    std::cerr << "ERROR Multiple switch\n";
                    return ERR_ARGS;
                }
                pop3s = true;
                break;
            case 'S':
                if(pop3s){
                    std::cerr << "ERROR Switches T and S combined\n";
                    return ERR_ARGS;
                }
                if(tls){
                    std::cerr << "ERROR Switches T and S combined\n";
                    return ERR_ARGS;
                }
                tls = true;
                break;
            case 'c':
                if(certfile != ""){
                    std::cerr << "ERROR multiple switch\n";
                    return ERR_ARGS;
                }
                certfile = optarg;
                break;
            case 'C':
                if(certaddr != ""){
                    std::cerr << "ERROR multiple switch\n";
                    return ERR_ARGS;
                }
                certaddr = optarg;
                break;
            case 'd':
                if(dlt){
                    std::cerr << "ERROR multiple switch\n";
                    return ERR_ARGS;
                }
                dlt = true;
                break;
            case 'n':
                if(nwm){
                    std::cerr << "ERROR multiple switch\n";
                    return ERR_ARGS;
                }
                nwm = true;
                break;
            case 'a':
                if(authfile != ""){
                    std::cerr << "ERROR multiple switch\n";
                    return ERR_ARGS;
                }
                authfile = optarg;
                break;
            case 'o':
                if(outdir != ""){
                    std::cerr << "ERROR multiple switch\n";
                    return ERR_ARGS;
                }
                outdir = optarg;
                if(outdir[outdir.length()-1] != '/'){
                    outdir += '/';
                }
                break;
	     default:
		return ERR_ARGS;
        }
    }
    if(port_number == -1){
        if(pop3s){
            port_number = 995;
        }
        else{
            port_number = 110;
        }
    }

    for (int i = optind; i < argc; i++){
        if(i != argc - 1){
            std::cerr << "ERROR bad arguments\n";
            return ERR_ARGS;
        }
        server_hostname = argv[i];
    }

    if(server_hostname == "" || authfile == "" || outdir == ""){
        std::cerr << "ERROR missing required aruments\n";
        return ERR_ARGS;
    }
    if(certfile != "" || certaddr != ""){
        if(!pop3s && !tls){
            std::cerr << "ERROR certificate directory/file without -T or -S switch\n";
            return ERR_ARGS;
        }
    }


    return 0;
}