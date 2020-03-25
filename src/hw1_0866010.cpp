#include "hw1_0866010.h"

bool is_number(char *cstr)
{
    string s(cstr);
    string::const_iterator it = s.begin();
    while (it != s.end() && isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

void parse_opt(int argc, char *argv[], map<string, bool> &show, vector<string> &filter_strs, bool &flag)
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

        flag = true;

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

string network_to_presentation(char *pch_bs, string proto)
{

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
    string port;
    int ip_version;


    ip_version = (strchr(proto.c_str(), '6')==NULL) ? 4 : 6;

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

    port = to_string(stoul(string(pch_colo + 1), NULL, 16));

    return string(ip_addr_cstr) + ":" + ((port == "0") ? "*" : port);
}

void read_net(vector<pair<bool, Con_entry> > &data, string file, map<string, pair<char, int> > &ind_entry)
{
    Con_entry *p_ce;

    FILE *fin;
    string path = "/proc/net/";
    char str[BUFFER_SIZE];

    const char *Token = " ";
    char *pch_bs;



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
        p_ce->process = "-";
        p_ce->protocol = file;

        // Parse each row for ip_address and inodenumber with Token
        pch_bs = strtok(str, Token);
        for(int j=0 ; pch_bs != NULL ; ++j){     // j for column-index in /proc/net/<file>, which starts from 0
            

            switch (j)
            {
            case 1:     // local_address
                // cout << "col1: " << pch_bs << " ";
                p_ce->local_addr = network_to_presentation(pch_bs, file);
                break;
            case 2:     // rem_address
                // cout << "col2:" << pch_bs << " ";
                p_ce->foreign_addr = network_to_presentation(pch_bs, file);
                break;
            case 9:     // inode number
                // cout << "col9: " << pch_bs << " ";
                p_ce->inode_num = pch_bs;
                break;
            }


            pch_bs = strtok(NULL, Token);
        }


        data.push_back(make_pair(true, *p_ce));      // copy the content of previous struct
        ind_entry[data.back().second.inode_num] = make_pair((file.find("tcp")!=string::npos ? 't' : 'u'), data.size() - 1);     // maintain the mapping

        delete p_ce;        // free memory space for struct Con_entry
    }

    fclose(fin);
}

void lookup_proc(map<string, pair<char, int> > &ind_entry, vector<pair<bool, Con_entry> > &tcp, vector<pair<bool, Con_entry> > &udp)
{
    DIR *p_dir;
    FILE *fin;

    const string dir = "/proc/";
    string subdir;

    char *r_branket, *l_branket;
    char target_path[256];
    char tmp[TMP_SIZE];
    char *pch_hyphen;
    struct dirent *p_dirent;

    map<string, pair<char, int> >::iterator itind;
    vector<pair<bool, Con_entry> >::iterator itvec;
    vector<string> proc_list;




    // first build up a list storing running process's PID
    if((p_dir = opendir(dir.c_str())) == NULL){       // On error, opendir() function returns NULL
        cerr << "opendir: (/proc/)" << dir << ": " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
    }

    while((p_dirent = readdir(p_dir)) != NULL){
        if(is_number(p_dirent->d_name))
            proc_list.push_back(p_dirent->d_name);
    }        
    closedir(p_dir);




    // use previous-created proc_list to traverse all the process's fd dir
    for(auto it=proc_list.begin() ; it!=proc_list.end() ; it++){
        subdir = dir + *it + "/fd/";        // (i.e. /proc/<PID>/fd/)

        // opendir error
        if((p_dir = opendir(subdir.c_str())) == NULL){
            
            // usually is permission denied, so skip
            if(errno == EACCES){
                continue;
            }
            else{
                cerr << "opendir: " << subdir << ": " << strerror(errno) << endl;
                exit(EXIT_FAILURE);
            }
        }


        // read one file info one iteration
        while((p_dirent = readdir(p_dir)) != NULL){

            // check if the file is symbolic link
            if(p_dirent->d_type == DT_LNK){

                // readlink
                if(readlink((subdir + p_dirent->d_name).c_str(), target_path, sizeof(target_path)) == -1){      // On error, readlink returns -1 
                    cerr << "readlink: " << strerror(errno) << endl;
                    exit(EXIT_FAILURE);
                }
                
                if(strstr(target_path, "socket:")){
                    // cout << target_path << "\n";

                    // locate '[' & ']' for extacting PID (e.g. socket:[546215])
                    l_branket = strchr(target_path, '[');
                    r_branket = strchr(target_path, ']');


                    // check if the entry for this PID exists
                    if((itind = ind_entry.find(string(l_branket + 1, r_branket - l_branket - 1))) != ind_entry.end()){

                        // switch to tcp vector or udp for filling up the process infomations
                        if(itind->second.first == 't'){
                            itvec = tcp.begin();
                        }
                        else{
                            itvec = udp.begin();
                        }
                        
                        // append PID
                        (itvec + itind->second.second)->second.process.clear();
                        (itvec + itind->second.second)->second.process += *it;
                        (itvec + itind->second.second)->second.process += "/";
                        

                        // get command (i.e. process name)
                        fin = fopen((dir + *it + "/comm").c_str(), "r");
                        if(fin == NULL){
                            cerr << "fopen: (process file)" << strerror(errno) << endl;
                            exit(EXIT_FAILURE);
                        }
                        fgets(tmp, TMP_SIZE, fin);
                        if(*(tmp + strlen(tmp) - 1) == '\n')   *(tmp + strlen(tmp) - 1) = '\0';
                        (itvec + itind->second.second)->second.process += tmp;
                        fclose(fin);
                

                        // get cmdline arguments
                        fin = fopen((dir + *it + "/cmdline").c_str(), "r");
                        if(fin == NULL){
                            cerr << "fopen: (process file)" << strerror(errno) << endl;
                            exit(EXIT_FAILURE);
                        }
                        (itvec + itind->second.second)->second.process += ((pch_hyphen = strchr(fgets(tmp, TMP_SIZE, fin), ' ')) == NULL ? "" : pch_hyphen);
                        fclose(fin);
                    }
                }
            }
        }
        closedir(p_dir);
    }
}

void output(bool flag, map<string, bool> &show, const vector<pair<bool, Con_entry> > &tcp, const vector<pair<bool, Con_entry> > &udp)
{
    if(flag){
        if(show["tcp"]){
            cout << "List of TCP connections:\n";
            cout << setw(OUTPUT_PROTOCOL_WIDTH) << left << "Proto" << setw(OUTPUT_IP_ADDR_WIDTH) << left << "Local Address" << setw(OUTPUT_IP_ADDR_WIDTH) << left << "Forign Address" << setw(OUTPUT_IP_ADDR_WIDTH) << left << "PID/Program name and arguments" << endl;
            for(auto it=tcp.begin() ; it!=tcp.end() ; ++it){
                if(it->first)
                    cout << setw(OUTPUT_PROTOCOL_WIDTH) << left << it->second.protocol << setw(OUTPUT_IP_ADDR_WIDTH) << left << it->second.local_addr << setw(OUTPUT_IP_ADDR_WIDTH) << left << it->second.foreign_addr << setw(OUTPUT_IP_ADDR_WIDTH) << left << it->second.process << "\n";
            }
            cout << endl;
        }
        if(show["udp"]){
            cout << "List of UDP connections:\n";
            cout << setw(OUTPUT_PROTOCOL_WIDTH) << left << "Proto" << setw(OUTPUT_IP_ADDR_WIDTH) << left << "Local Address" << setw(OUTPUT_IP_ADDR_WIDTH) << left << "Forign Address" << setw(OUTPUT_IP_ADDR_WIDTH) << left << "PID/Program name and arguments" << endl;
            for(auto it=udp.begin() ; it!=udp.end() ; ++it){
                if(it->first)
                    cout << setw(OUTPUT_PROTOCOL_WIDTH) << left << it->second.protocol << setw(OUTPUT_IP_ADDR_WIDTH) << left << it->second.local_addr << setw(OUTPUT_IP_ADDR_WIDTH) << left << it->second.foreign_addr << setw(OUTPUT_IP_ADDR_WIDTH) << left << it->second.process << "\n";
            }
            cout << endl;
        }
    }

    // default display (show both tcp & udp)
    else{
        for(int i=0 ; i<2 ;++i){
            cout << "List of "<< (i==0 ? "TCP" : "UDP") << " connections:\n";
            cout << setw(OUTPUT_PROTOCOL_WIDTH) << left << "Proto" << setw(OUTPUT_IP_ADDR_WIDTH) << left << "Local Address" << setw(OUTPUT_IP_ADDR_WIDTH) << left << "Forign Address" << setw(OUTPUT_IP_ADDR_WIDTH) << left << "PID/Program name and arguments" << endl;
            for(auto it=(i==0 ? tcp.begin() : udp.begin()); it!=(i==0 ? tcp.end() : udp.end()) ; ++it){
                if(it->first)
                    cout << setw(OUTPUT_PROTOCOL_WIDTH) << left << it->second.protocol << setw(OUTPUT_IP_ADDR_WIDTH) << left << it->second.local_addr << setw(OUTPUT_IP_ADDR_WIDTH) << left << it->second.foreign_addr << setw(OUTPUT_IP_ADDR_WIDTH) << left << it->second.process << "\n";
            }
            cout << endl;
        }
    }
}

void filter(const vector<string> &filter_strs, vector<pair<bool, Con_entry> > &tcp, vector<pair<bool, Con_entry> > &udp)
{
    if(filter_strs.size() > 0){
        for(auto i=0 ; i<2 ; ++i){

            // traverse both tcp and udp vector to match filter
            for(auto it_entry=(i==0 ? tcp.begin() : udp.begin()); it_entry!=(i==0 ? tcp.end() : udp.end()) ; ++it_entry){
                
                // In default, turn off visability of each entry
                it_entry->first = false;

                for(auto it_filter_str=filter_strs.begin() ; it_filter_str!=filter_strs.end() ; ++it_filter_str){
                    if(it_entry->second.protocol.find(*it_filter_str) != string::npos){
                        it_entry->first = true;
                        goto MATCH;
                    }
                    if(it_entry->second.local_addr.find(*it_filter_str) != string::npos){
                        it_entry->first = true;
                        goto MATCH;
                    }
                    if(it_entry->second.foreign_addr.find(*it_filter_str) != string::npos){
                        it_entry->first = true;
                        goto MATCH;
                    }
                    if(it_entry->second.process.find(*it_filter_str) != string::npos){
                        it_entry->first = true;
                        goto MATCH;
                    }

                    MATCH:
                        break;
                }
            }
        }
    }
}

int main(int argc, char *argv[])
{   
    bool flag = false;
    map<string, bool> show;                         // use to determine display either tcp or udp connections
    map<string, pair<char, int> > ind_entry;        //mapping between inode number and entry number of tcp or udp vector
    vector<string> filter_strs;                     // store string for filter function

    vector<pair<bool, Con_entry> > tcp;     // store tcp connection info from /proc/net/tcp
    vector<pair<bool, Con_entry> > udp;     // store udp connection info from /proc/net/udp



    // parse options passed by user
    parse_opt(argc, &argv[0], show, filter_strs, flag);

    // read /proc/net/tcp & udp and store in data structure (i.e. vector tcp, udp)
    // concatenate the IPv6 info to the same vector (i.e. tcp or udp) below IPv4 info
    read_net(tcp, "tcp", ind_entry);
    read_net(udp, "udp", ind_entry);
    read_net(tcp, "tcp6", ind_entry);
    read_net(udp, "udp6", ind_entry);

    // use inode unmber to map process
    lookup_proc(ind_entry, tcp, udp);

    // filter out result
    filter(filter_strs, tcp, udp);

    // display results in std_out
    output(flag, show, tcp, udp);

    exit(EXIT_SUCCESS);
}