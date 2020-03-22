#include <getopt.h>     // for getopt_long()
#include <sys/types.h>  // for opendir(), stat()
#include <sys/stat.h>   // for stat()
#include <dirent.h>     // for opendir(), readdir()
#include <unistd.h>     // for stat(), readlink()
#include <string.h>     // for strerror()
#include <fstream>      // for std::ifstream
#include <cstring>      // for strtok()
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;



typedef struct Connection_entry
{
    char *local_addr;
    char *foreign_addr;
    char *process;
    char *inode_num;
} Con_entry;




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

void read_net(vector<Con_entry> &data, string file){

    ifstream fin;
    fin.exceptions(ifstream::badbit);
    string path = "/proc/net/";
    char str[500];
    char *pch;
    Con_entry *p_cl;



    try{
        fin.open(path + file);

        for(int i=0 ; !fin.eof() && i<3 ; ++i){        // i for row-index in /proc/net/<file>, which starts from 0
        
            fin.getline(str, sizeof(str)/sizeof(char));

            // ignore the first row
            if(i == 0)  continue;
            
            cout << "str: " << str << endl;

            p_cl = new Con_entry();
            // cout << &p_cl << endl;
            p_cl->process = "(None)";


            pch = strtok(str, " ");
            for(int j=0 ; pch!=NULL ; ++j){     // j for column-index in /proc/net/<file>, which starts from 0
                
                // cout << "local_addr\n";
                // for(auto it=data.begin() ; it!=data.end() ; ++it){
                //     cout << it->local_addr << "\n";
                // }
                // cout << endl;

                switch (j)
                {
                case 1:     // local_address
                    // cout << pch << " ";
                    p_cl->local_addr = pch;
                    break;
                case 2:     // rem_address
                    // cout << pch << " ";
                    p_cl->foreign_addr = pch;
                    break;
                case 9:     // inode number
                    // cout << pch << " ";
                    p_cl->inode_num = pch;
                    break;
                }
                // cout << pch << " ";
                pch = strtok(NULL, " ");
            }
            cout << endl;


            cout << "local_addr\tremote_addr\t#inode\tpid\n";
            for(auto it=data.begin() ; it!=data.end() ; ++it){
                cout << it->local_addr << "\t" << it->foreign_addr << "\t" << it->inode_num << "\t" << it->process << "\n";
            }
            cout << endl;

            data.push_back(*p_cl);

            cout << "local_addr\tremote_addr\t#inode\tpid\n";
            for(auto it=data.begin() ; it!=data.end() ; ++it){
                cout << it->local_addr << "\t" << it->foreign_addr << "\t" << it->inode_num << "\t" << it->process << "\n";
            }
            cout << endl;


            // cout << data[0].inode_num << " ";
            // cout << "#entries of data: " << data.size() << endl;
        }
        // cout << "#entries of data: " << data.size() << endl;

        fin.close();
    }


    catch(ifstream::failure e){
        cerr << e.what() << endl;
        exit(EXIT_FAILURE);
    }



    // ifstream fin_udp("/proc/net/udp", ifstream::in);     // open file for reading
}

int main(int argc, char *argv[])
{   
    map<string, bool> show;     // use to determine display either tcp or udp connections
    vector<string> filter_strs;     // store string for filter function
    vector<Con_entry> tcp;     // store tcp connection info from /proc/net/tcp
    vector<Con_entry> udp;     // store udp connection info from /proc/net/udp



    // read /proc/net/tcp & udp and store in data structure (i.e. vector tcp, udp)
    read_net(tcp, "tcp");
    // read_net(udp, "udp");

    // cout << "local_addr\tremote_addr\t#inode\tpid\n";
    // for(auto it=tcp.begin() ; it!=tcp.end() ; ++it){
    //     cout << it->local_addr << "\t" << it->foreign_addr << "\t" << it->inode_num << "\t" << it->process << "\n";
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




    // // opendir & readdir & closedir
    // char dir[] = "/proc/20284/fd/";
    // DIR *p_dir;
    // struct dirent *p_dirent;

    // if((p_dir = opendir(dir)) == NULL){       // On error, opendir() function returns NULL 
    //     cerr << "opendir: " << dir << ": " << strerror(errno) << endl;
    //     exit(EXIT_FAILURE);
    // }
    // else{
    //     while((p_dirent = readdir(p_dir)) != NULL){

    //         switch (p_dirent->d_type)
    //         {
    //         case DT_CHR:
    //             cout << "C ";
    //             break;
    //         case DT_DIR:
    //             cout << "D ";
    //             break;
    //         case DT_LNK:
    //             cout << "L ";
    //             break;
    //         case DT_REG:
    //             cout << "- ";
    //             break;
    //         default:
    //             cout << "? ";
    //             break;
    //         }

    //         cout << p_dirent->d_name << endl;
    //     }
    //     cout << endl;
        
    //     closedir(p_dir);
    // }




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