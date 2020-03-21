#include <iostream>
#include <getopt.h>     // for getopt_long
#include <map>
#include <string>

using namespace std;


void parse_opt(int argc, char *argv[], map<string, bool> &show)
{
    const char *optstring = "tu";
    int c ;
    struct option opts[] = {
        {"tcp", 0,  NULL,   't'},
        {"udp", 0,  NULL,   'u'},
        {0,     0,  0,      0}      // unrecognized option
    };

    for(unsigned int i=0 ; i<(sizeof(opts)/sizeof(option)) ; ++i){
        if(opts[i].name != 0)
            show.insert(pair<string, bool>(opts[i].name, false));
    }


    while((c = getopt_long(argc, argv, optstring, opts, NULL)) != -1){
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
}


int main(int argc, char *argv[])
{   
    map<string, bool> show;     // use to determine display either tcp or udp connections

    // parse options passed by user
    parse_opt(argc, &argv[0], show);

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

    exit(EXIT_SUCCESS);
}