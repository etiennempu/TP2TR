/*
 *  C code to demonstrate control of the LED matrix for the
 *  Raspberry Pi Sense HAT add-on board.
 *
 *  Uses the mmap method to map the led device into memory
 *
 *  Build with:  gcc -Wall -O2 led_matrix.c -o led_matrix
 *               or just 'make'
 *
 *  Tested with:  Sense HAT v1.0 / Raspberry Pi 3 B+ / Raspbian GNU/Linux 10 (buster)
 *
 */

#include <fcntl.h>
#include <linux/fb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define FILEPATH "/dev/fb0"
#define NUM_WORDS 64
#define FILESIZE (NUM_WORDS * sizeof(uint16_t))

#define RED 0xF800
#define GREEN 0x07E0
#define YELLOW 0xFFE0

int fbfd;
uint16_t *map;


void clear();

void LedUpdate(int index) {
    int i;
	int G1[16]={0,1,8,9,16,17,24,25,32,33,40,41,48,49,56,57};
	int G2[16]={3,4,11,12,19,20,27,28,35,36,43,44,51,52,59,60};
	int G3[16]={6,7,14,15,22,23,30,31,38,39,46,47,54,55,62,63};
    uint16_t *p;
	struct fb_fix_screeninfo fix_info;

    
	switch(index){
		
	case 0:
		
		
		/* open the led frame buffer device */
		fbfd = open(FILEPATH, O_RDWR);
		if (fbfd == -1) {
			perror("Error (call to 'open')");
			exit(EXIT_FAILURE);
		}

		/* read fixed screen info for the open device */
		if (ioctl(fbfd, FBIOGET_FSCREENINFO, &fix_info) == -1) {
			perror("Error (call to 'ioctl')");
			close(fbfd);
			exit(EXIT_FAILURE);
		}

		/* now check the correct device has been found */
		if (strcmp(fix_info.id, "RPi-Sense FB") != 0) {
			printf("%s\n", "Error: RPi-Sense FB not found");
			close(fbfd);
			exit(EXIT_FAILURE);
		}

		/* map the led frame buffer device into memory */
		map =
			mmap(NULL, FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
		if (map == MAP_FAILED) {
			close(fbfd);
			perror("Error mmapping the file");
			exit(EXIT_FAILURE);
		}

		/* set a pointer to the start of the memory area */
		p = map;

		/* clear the led matrix */
		memset(map, 0, FILESIZE);
		
		/* light it up! */
		for (i = 0; i < 16; i++) {
			*(p + G1[i]) = GREEN;
			
		}
		for (i = 0; i < 16; i++) {
			*(p + G2[i]) = GREEN;
			
		}
		for (i = 0; i < 16; i++) {
			*(p + G3[i]) = GREEN;
			
		}
		
		break;
	case 1 :
	
		p = map;
		
		for (i = 0; i < 16; i++) {
			*(p + G1[i]) = GREEN;
			
		}
		
		break;
		
	case 2 :
	
		p = map;
		
		for (i = 0; i < 16; i++) {
			*(p + G1[i]) = YELLOW;
			
		}
		
		break;
	
	case 3 :
	
		p = map;
		
		for (i = 0; i < 16; i++) {
			*(p + G1[i]) = RED;
			
		}
		
		break;
	case 4 :
	
		p = map;
		
		for (i = 0; i < 16; i++) {
			*(p + G2[i]) = GREEN;
			
		}
		
		break;
		
	case 5 :
	
		p = map;
		
		for (i = 0; i < 16; i++) {
			*(p + G2[i]) = YELLOW;
			
		}
		
		break;
	
	case 6 :
	
		p = map;
		
		for (i = 0; i < 16; i++) {
			*(p + G2[i]) = RED;
			
		}
		
		break;
	case 7 :
	
		p = map;
		
		for (i = 0; i < 16; i++) {
			*(p + G3[i]) = GREEN;
			
		}
		
		break;
		
	case 8 :
	
		p = map;
		
		for (i = 0; i < 16; i++) {
			*(p + G3[i]) = YELLOW;
			
		}
		
		break;
	
	case 9 :
	
		p = map;
		
		for (i = 0; i < 16; i++) {
			*(p + G3[i]) = RED;
			
		}
		
		break;
		
	case 10 :
		
		clear();
		
		break;
	
	}

    

}
void clear(){
    /* clear the led matrix */
    memset(map, 0, FILESIZE);

    /* un-map and close */
    if (munmap(map, FILESIZE) == -1) {
        perror("Error un-mmapping the file");
    }
    close(fbfd);
}