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

#define FILEPATH "/dev/fb1"
#define NUM_WORDS 64
#define FILESIZE (NUM_WORDS * sizeof(uint16_t))

#define RED 0xF800
#define GREEN 0x07E0
#define YELLOW 0xFFE0

void delay(int);

int main(void) {
    int i;
    int fbfd;
    uint16_t *map;
    uint16_t *p;
    struct fb_fix_screeninfo fix_info;

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
    for (i = 0; i < 8; i++) {
        *(p + i) = RED;
        delay(25);
    }
	for (i = 8; i < 16; i++) {
        *(p + i) = GREEN;
        delay(25);
    }
	for (i = 16; i < 24; i++) {
        *(p + i) = YELLOW;
        delay(25);
    }

    
    /* clear the led matrix */
    memset(map, 0, FILESIZE);

    /* un-map and close */
    if (munmap(map, FILESIZE) == -1) {
        perror("Error un-mmapping the file");
    }
    close(fbfd);

    return 0;
}

void delay(int t) {
    usleep(t * 1000);
}