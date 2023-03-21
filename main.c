#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/input.h>
#include <fcntl.h>

#include "main.h"

#include "serveur.h"


int main(int argc, char** argv) {

   
	//test
    Connection();
    Sendmessage("AL1");
	Sendmessage("VL1");
	Receivemessage();
	
    //Salut
	
	Deconnection();
	
    exit(0);
}