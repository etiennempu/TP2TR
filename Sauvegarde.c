#include <stdlib.h>
#include <stdio.h>



void Sauvegarde(char* fileName,float data){
	
	
	FILE* fichier = NULL;

    fichier = fopen("test.txt", "w");
	
	if (fichier != NULL)
    {
		
		
		fprintf(fichier, "%d",data);

                
		fclose(fichier);
		

    }
    else
    {
        printf("Impossible d'ouvrir le fichier ");
    }
}
