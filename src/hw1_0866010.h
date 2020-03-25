#ifndef HW1_0866010_H_
#define HW1_0866010_H_


#include <getopt.h>     // for getopt_long()
#include <sys/types.h>  // for opendir(), stat()
#include <sys/stat.h>   // for stat()
#include <dirent.h>     // for opendir(), readdir()
#include <unistd.h>     // for stat(), readlink()
#include <string.h>     // for strerror()
#include <cstring>      // for strtok(), strchr()
#include <cstdio>       // for fopen(), fgets(), fclose()
#include <arpa/inet.h>  // for inet_ntop()
#include <iomanip>      // for setw()
#include <utility>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <vector>


#define BUFFER_SIZE 500
#define TMP_SIZE 150        // for buffering process args
#define OUTPUT_IP_ADDR_WIDTH 25
#define OUTPUT_PROTOCOL_WIDTH 6

using namespace std;


typedef struct Connection_entry
{
    string local_addr;
    string foreign_addr;
    string process;
    string inode_num;
    string protocol;
} Con_entry;


// function declaration
void parse_opt(int argc, char *argv[], map<string, bool> &show, vector<string> &filter_strs, bool &flag);
bool is_number(char *cstr);
string network_to_presentation(char *pch_bs, string proto);
void read_net(vector<pair<bool, Con_entry> > &data, string file, map<string, pair<char, int> > &ind_entry);
void lookup_proc(map<string, pair<char, int> > &ind_entry, vector<pair<bool, Con_entry> > &tcp, vector<pair<bool, Con_entry> > &udp);
void output(bool flag, map<string, bool> &show, const vector<pair<bool, Con_entry> > &tcp, const vector<pair<bool, Con_entry> > &udp);
void filter(const vector<string> &filter_strs, vector<pair<bool, Con_entry> > &tcp, vector<pair<bool, Con_entry> > &udp);


#endif  /* HW1_0866010_H_ */