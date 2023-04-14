#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char* strsep(char** stringp, const char* delim) {
    char* rv = *stringp;
    if (rv) {
        *stringp += strcspn(*stringp, delim);
        if (**stringp)
            *(*stringp)++ = '\0';
        else
            *stringp = 0; }
    return rv;
}

char** parser(char* cmd, char* delim) {
    char** cmd_paire = malloc(2*sizeof(char*));
    char* copie_cmd = strdup(cmd); //pcq, cmd est altéré par strsep et donc copie pour comparaison
    char* subchain;

    if ((subchain = strsep(&cmd, delim)) != NULL) {
        if (strcmp(subchain, copie_cmd) != 0) {
            *(cmd_paire) = subchain;
            *(cmd_paire+1) = strsep(&cmd, "\0");
            free(copie_cmd);
            return cmd_paire;
        }
    }
    free(copie_cmd);
    free(cmd_paire);
    return NULL;
}

char** loop_parser(char** cmds, char* delim) {
    char** tmp = parser(*(cmds+1), delim);
    free(cmds);
    return tmp;
}
