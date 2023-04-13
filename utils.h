#ifndef UTULS
#define UTILS

char* strsep(char** stringp, const char* delim);
char** parser(char* cmd, char* delim);
char** loop_parser(char** cmds, char* delim);

#endif