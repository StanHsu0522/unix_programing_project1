#include <getopt.h>     // for getopt_long()
#include <sys/types.h>  // for opendir(), stat()
#include <sys/stat.h>   // for stat()
#include <dirent.h>     // for opendir(), readdir()
#include <unistd.h>     // for stat(), readlink()
#include <string.h>     // for strerror()
#include <cstring>      // for strtok(), strchr()
#include <cstdio>       // for fopen(), fgets(), fclose()
#include <arpa/inet.h>  // for inet_ntop()
#include <utility>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;


#define BUFFER_SIZE 500


typedef struct Connection_entry
{
    string local_addr;
    string foreign_addr;
    string process;
    string inode_num;
    int proto_version;
} Con_entry;




bool is_number(char *cstr)
{
    string s(cstr);
    string::const_iterator it = s.begin();
    while (it != s.end() && isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

void parse_opt(int argc, char *argv[], map<string, bool> &show, vector<string> &filter_strs)
{
    const char *optstring = "tu";       // for short options
    int c ;
    struct option opts[] = {
        {"tcp", 0,  NULL,   't'},
        {"udp", 0,  NULL,   'u'}
    };

    for(unsigned int i=0 ; i<(sizeof(opts)/sizeof(option)) ; ++i){
        if(opts[i].name != 0)
            show.insert(pair<string, bool>(opts[i].name, false));
    }


    while((c = getopt_long(argc, argv, optstring, opts, NULL)) != -1){
        // cout << optopt << endl;      // By default, getopt() prints an error message on standard error,
                                        // and places the erroneous option character in optopt, and returns '?'.

        switch (c)
        {
        case 't':
            show["tcp"] = true;
            break;
        case 'u':
            show["udp"] = true;
            break;
        default:        // '?'
            cerr << " Usage: " << argv[0] << " [-t|--tcp] [-u|--udp] [filter-string]" << endl;
            exit(EXIT_FAILURE);
        }
    }

    // have more args?
    // optind is the index of the next element to be processed in argv[].
    while(argc > optind){
        filter_strs.push_back(argv[optind++]);
    }
}

string network_to_presentation(char *pch_bs, int ip_version){

    // Big-Endian puts highest bit in the lowwer memory adrress
    //
    //         0x 1  2  3  4  5  6  7  8    <--- Data
    //       +---------------------------+
    // low   | 0x12 | 0x34 | 0x56 | 0x78 |   high
    //       +---------------------------+
    //
    // Little-Endian
    //       +---------------------------+
    // low   | 0x78 | 0x56 | 0x34 | 0x12 |   high
    //       +---------------------------+
    //
    // //example test code for the machine endian
    // #include <stdio.h>
    // typedef union {
    //     unsigned long l;
    //     unsigned char c[4];
    // } EndianTest;
    // EndianTest et;
    // et.l = 0x12345678;
    // printf("本系統位元組順序為：");
    // if (et.c[0] == 0x78 && et.c[1] == 0x56 && et.c[2] == 0x34 && et.c[3] == 0x12) {
    //     printf("Little Endian\n");
    // } else if (et.c[0] == 0x12 && et.c[1] == 0x34 && et.c[2] == 0x56 && et.c[3] == 0x78) {
    //     printf("Big Endian\n");
    // } else {
    //     printf("Unknown Endian\n");
    // }

    struct in6_addr ipv6;
    struct in_addr ipv4;

    char *pch_colo;
    const char *ntop_result;
    char ip_addr_cstr[INET6_ADDRSTRLEN];



    pch_colo = strchr(pch_bs, ':');         // search the position of colon for seperating the address and port


    switch (ip_version)
    {
    case 4:     // IPv4
        // the data type of the field "s_addr" of struct in_addr is uint32_t

        ipv4.s_addr = stoul(string(pch_bs, pch_colo-pch_bs), NULL, 16);     // convert HEX string to unsighed long int
        ntop_result = inet_ntop(AF_INET, &(ipv4), ip_addr_cstr, sizeof(ip_addr_cstr)/sizeof(char));
        
        break;


    case 6:     // IPv6
        // Fill struct in6_addr and then use inet_ntop() to display human readable ipv6 address.
        // Note that the array in the struct in6_addr is filled in the network byte order
        // the data type of the array "s6.addr" is uint8_t


        // // This section is used to feed test data for ipv6 address conversion
        // string str = "BACD0120000000000000000052965732";        // have to come up with "2001:cdba::3257:9652"
        // pch_bs = (char*) str.c_str();


        // manually map each byte from little-endian to big-endian (bug!!)
        for(auto i=0 ; i<4 ; i++)
            for(auto j=0 ; j<4 ; j++)
                ipv6.s6_addr[( (i+1) * 4) - j - 1] = stoul(string(pch_bs + (8*i) + (2*j), 2), NULL, 16);

        ntop_result = inet_ntop(AF_INET6, &(ipv6), ip_addr_cstr, sizeof(ip_addr_cstr)/sizeof(char));
        
        break;


    default:
        ntop_result = NULL;
    }


    if(!ntop_result){       // NULL is returned if there was an error of inet_ntop()
        cerr << "inet_ntop: " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
    }

    return string(ip_addr_cstr) + ":" + to_string(stoul(string(pch_colo + 1), NULL, 16));
}

void read_net(vector<Con_entry> &data, string file, map<string, pair<char, vector<Con_entry>::iterator> > &ind_entry){

    FILE *fin;
    string path = "/home/stan/nctu/unix_programing/src/project1/";
    char str[BUFFER_SIZE];

    Con_entry *p_ce;

    const char *Token = " ";
    char *pch_bs;

    int ip_version = strchr(file.c_str(), '6') ? 6 : 4;


    fin = fopen((path + file).c_str(), "r");
    if(fin == NULL){
        cerr << "fopen: " << path + file << ": " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
    }


    for(int i=0 ; fgets(str, BUFFER_SIZE, fin) != NULL ; ++i){        // i for row-index in /proc/net/<file>, which starts from 0
        // cout << str << endl;
    
        // ignore the first row
        if(i == 0)  continue;
        

        p_ce = new Con_entry();
        p_ce->process = "";
        p_ce->proto_version = ip_version;

        // Parse each row for ip_address and inodenumber with Token
        pch_bs = strtok(str, Token);
        for(int j=0 ; pch_bs != NULL ; ++j){     // j for column-index in /proc/net/<file>, which starts from 0
            

            switch (j)
            {
            case 1:     // local_address
                // cout << "col1: " << pch_bs << " ";
                p_ce->local_addr = network_to_presentation(pch_bs, ip_version);
                break;
            case 2:     // rem_address
                // cout << "col2:" << pch_bs << " ";
                p_ce->foreign_addr = network_to_presentation(pch_bs, ip_version);
                break;
            case 9:     // inode number
                // cout << "col9: " << pch_bs << " ";
                p_ce->inode_num = pch_bs;
                break;
            }


            pch_bs = strtok(NULL, Token);
        }


        data.push_back(*p_ce);      // copy the content of previous struct
        ind_entry[data.back().inode_num] = make_pair((file.find("tcp")!=string::npos ? 't' : 'u'), data.end() - 1);     // maintain the mapping

        delete p_ce;        // free memory space for struct Con_entry
    }

    fclose(fin);

    Con_entry *a = new Con_entry();
    data.push_back(*a);
}

int main(int argc, char *argv[])
{   
    map<string, bool> show;     // use to determine display either tcp or udp connections
    map<string, pair<char, vector<Con_entry>::iterator> > ind_entry;      //mapping between inode number and entry number of tcp or udp vector
    vector<string> filter_strs;     // store string for filter function

    vector<Con_entry> tcp;     // store tcp connection info from /proc/net/tcp
    vector<Con_entry> udp;     // store udp connection info from /proc/net/udp



    // // read /proc/net/tcp & udp and store in data structure (i.e. vector tcp, udp)
    // // concatenate the IPv6 info to the same vector (i.e. tcp or udp) below IPv4 info
    // read_net(tcp, "tcp", ind_entry);
    // read_net(udp, "udp", ind_entry);
    // read_net(tcp, "tcp6", ind_entry);
    // read_net(udp, "udp6", ind_entry);

    // cout << "[TCP Links]\nlocal_addr\t\tremote_addr\t\t#inode\t\tpid\n";
    // for(auto it=tcp.begin() ; it!=tcp.end() ; ++it){
    //     cout << it->local_addr << ((it->local_addr.length() >15) ? "\t" : "\t\t") << it->foreign_addr << ((it->foreign_addr.length() >15) ? "\t" : "\t\t") << it->inode_num << "\t" << it->process << "\n";
    // }
    // cout << endl;

    // cout << "[UDP Links]\nlocal_addr\t\tremote_addr\t\t#inode\t\tpid\n";
    // for(auto it=udp.begin() ; it!=udp.end() ; ++it){
    //     cout << it->local_addr << ((it->local_addr.length() >15) ? "\t" : "\t\t") << it->foreign_addr << ((it->foreign_addr.length() >15) ? "\t" : "\t\t") << it->inode_num << "\t" << it->process << "\n";
    // }
    // cout << endl;






    // // readlink
    // char link_path[50] = "/proc/20284/fd/96";
    // char target_path[256];

    // memset(target_path, 0, sizeof(target_path));
    // if(readlink(link_path, target_path, sizeof(target_path)) == -1)      // On error, readlink returns -1 
    // {
    //     cerr << "readlink: " << link_path << ": " << strerror(errno) << endl;
    //     exit(EXIT_FAILURE);
    // }
    // else
    // {
    //     cout << "link path: " << target_path;
    // }




    // opendir & readdir & closedir
    DIR *p_dir;
    string dir, subdir;
    struct dirent *p_dirent;
    vector<string> proc_list;

    dir = "/proc/";



    if((p_dir = opendir(dir.c_str())) == NULL){       // On error, opendir() function returns NULL 
        cerr << "opendir: " << dir << ": " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
    }

    while((p_dirent = readdir(p_dir)) != NULL){
        if(is_number(p_dirent->d_name))
            proc_list.push_back(p_dirent->d_name);
    }        
    closedir(p_dir);

    // for(auto it = proc_list.begin() ; it!=proc_list.end() ;++it) cout << *it << "\t";
    // cout << endl;


    for(auto it=proc_list.begin() ; it!=proc_list.end() ; it++){
        subdir = dir + *it + "/fd/";
        cout << "dir: " << subdir << endl;


        if((p_dir = opendir(subdir.c_str())) == NULL){       // On error, opendir() function returns NULL 
            cerr << "opendir: " << subdir << ": " << strerror(errno) << endl;
            exit(EXIT_FAILURE);
        }

        while((p_dirent = readdir(p_dir)) != NULL){

            switch (p_dirent->d_type)
            {
            case DT_CHR:
                cout << "C ";
                break;
            case DT_DIR:
                cout << "D ";
                break;
            case DT_LNK:
                cout << "L ";
                break;
            case DT_REG:
                cout << "- ";
                break;
            default:
                cout << "? ";
                break;
            }
            cout << p_dirent->d_name << " ";
            
        }
        cout << "\n" << endl;

        closedir(p_dir);
        
    }





    // parse options passed by user
    // parse_opt(argc, &argv[0], show, filter_strs);


    // if(show.size() > 0){        // only show tcp or udp or both
    //     if(show["tcp"]){        // show tcp connections

    //     }
    //     else if(show["udp"]){       // show udp connections

    //     }
    // }
    // else{       // default show all connections

    // }

    exit(EXIT_SUCCESS);

    // // map show traverse
    // for(auto it = show.begin() ; it!=show.end() ; ++it){
    //     if(it->second){
    //         cout << "show " << it->first << endl;
    //     }
    //     else
    //     {
    //         cout << "don't show " << it->first << endl;
    //     }
    // }

    // // vector filter_strs traverse
    // if(filter_strs.size() > 0){
    //     cout << "filter_str: ";
    //     for(auto it=filter_strs.begin() ; it!=filter_strs.end() ; ++it){
    //         cout << *it << " ";
    //     }
    //     cout << endl;
    // }
}