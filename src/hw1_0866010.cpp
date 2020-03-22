#include <iostream>
#include <getopt.h>     // for getopt_long
#include <map>
#include <string>
#include <vector>

using namespace std;


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


int main(int argc, char *argv[])
{   
    map<string, bool> show;     // use to determine display either tcp or udp connections
    vector<string> filter_strs;     // store string for filter function


    // parse options passed by user
    parse_opt(argc, &argv[0], show, filter_strs);

    // map show traverse
    for(auto it = show.begin() ; it!=show.end() ; ++it){
        if(it->second){
            cout << "show " << it->first << endl;
        }
        else
        {
            cout << "don't show " << it->first << endl;
        }
    }

    // vector filter_strs traverse
    if(filter_strs.size() > 0){
        cout << "filter_str: ";
        for(auto it=filter_strs.begin() ; it!=filter_strs.end() ; ++it){
            cout << *it << " ";
        }
        cout << endl;
    }



    exit(EXIT_SUCCESS);
}