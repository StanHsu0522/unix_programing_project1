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


#endif  /* HW1_0866010_H_ */