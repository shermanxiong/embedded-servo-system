#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

/* baudrate settings are defined in <asm/termbits.h>, which is
 * included by <termios.h> */
#ifndef BAUDRATE
#define BAUDRATE B2400
#endif

#define _POSIX_SOURCE 1		/* POSIX compliant source */

static int fd;
static struct termios oldtio, newtio;
static char *device;

int serial_init(char *modemdevice, int canonical)
{
    /*
     * Open modem device for reading and writing and not as controlling tty
     * because we don't want to get killed if linenoise sends CTRL-C.
     **/
    device = modemdevice;
    fd = open (device, O_RDWR | O_NOCTTY );
    if (fd < 0)
      {
	  perror (device);
	  exit(-1);
      }

    tcgetattr (fd, &oldtio);	/* save current serial port settings */
    memcpy(&newtio, &oldtio, sizeof(newtio));

    cfsetispeed(&newtio, BAUDRATE);
    cfsetospeed(&newtio, BAUDRATE);

    /*
     *CRTSCTS : output hardware flow control (only used if the cable has
     *all necessary lines. )
     *CS8     : 8n1 (8bit,no parity,1 stopbit)
     *CLOCAL  : local connection, no modem contol
     *CREAD   : enable receiving characters
     **/
    newtio.c_cflag &= ~CSIZE;
    newtio.c_cflag &= ~PARENB;
    newtio.c_cflag &= ~CSTOPB;
    newtio.c_cflag &= ~CRTSCTS;
    newtio.c_cflag |=  CS8 | CLOCAL | CREAD;

    /*
     *ICRNL   : map CR to NL (otherwise a CR input on the other computer
     *          may not terminate input)
     *          otherwise make device raw (no other input processing)
     **/
    newtio.c_iflag |=  ICRNL;

#ifndef  SEND_RAW_NEWLINES
    /*
     * Map NL to CR NL in output.
     *                  */
    newtio.c_oflag |= ONLCR;
#else
    newtio.c_oflag &= ~ONLCR;
#endif

    /*
     * ICANON  : enable canonical input, read line-by-line
     **/
    if(canonical) {
	newtio.c_lflag |= ICANON;
#ifdef ECHO
    /* If you use canonical mode to use line editing, you may
     * want to turn on echo of characters to make the edits to show
     * in the sending terminal */
    newtio.c_lflag |= ECHO | ECHOE;
#endif
    } else {
	newtio.c_lflag &= ~ICANON;
    }

    /*
     * ISIG : enable SIGINTR, SIGSUSP, etc..
     **/
    newtio.c_lflag &= ~ISIG;

    /*
     * now clean the modem line and activate the settings for the port
     **/
    tcflush (fd, TCIFLUSH);
    tcsetattr (fd, TCSANOW, &newtio);

    /*
     * terminal settings done, return file descriptor
     **/

    return fd;
}

void serial_cleanup(int ifd){
    if(ifd != fd) {
	    fprintf(stderr, "WARNING! file descriptor != the one returned by serial_init()\n");
    }
#ifdef DRAIN_BEFORE_CLOSE
    /* wait until all chars in buffer have been sent */
    tcdrain(ifd);
#endif
    /* restore the old port settings */
    tcsetattr (ifd, TCSANOW, &oldtio);
    close(ifd);
}

void print_prompt(void) {
	printf("Set speed => s <value between 5 and 120>\n"
		   "Get speed => g\n"
		   "Exit with Ctrl-D (EOF)\n"
		   "> ");
}

int main(void) {
	fd = serial_init("/dev/ttyS0", 0 ); 
	FILE *avr = fdopen(fd, "r+");
	if(!avr) {
		perror("fdopen");
	}

	char cmd;
	unsigned char t_buff[4];
	unsigned char r_buff[4];
	char line[16];
	int input;
	int t_flag;
	int r_flag;
	
	// get user input
	print_prompt();
	fgets(line, sizeof(line), stdin);
	int args = sscanf(line, "%c %d", &cmd, &input);
	
	while(1) {
		switch(cmd) {
			case 'g': // get motor speed
				do {
					// sending 'z' to AVR tells micro-controller to send back speed
					*t_buff = 'z';
					t_flag = write(fd, t_buff, 1);
					printf("Reading speed\n");
					r_flag = read(fd, r_buff, 1);
				} while (r_flag != 1);
				printf("Speed: %d RPM\n", (int) *r_buff);
				break;
			case 's': // set motor speed to input value
				if (args != 2) {
					printf("Syntax error. Expected 2 arguments: <command> <value>\n");
					break;
				}
				if (input < 5 || input > 120) { // check input values
					printf("Input error. RPM value out of range\n");
					break;
				}
				// convert input (integer) value to char to send through serial port
				// this works because 5 <= input <= 120, well within ASCII value range 0-255
				*t_buff = (char) input;
				do {
					t_flag = write(fd, t_buff, 1);
					printf("Sent value: %d\n", (int) *t_buff);
				} while (t_flag < 0);
				printf("OK\n");
				break;
			default:
				printf("Syntax error. Invalid command.\n");
				break;
		}
		printf("> ");
		fgets(line, sizeof(line), stdin);
		args = sscanf(line, "%c %d", &cmd, &input);
	}
	
	fclose(avr);
	serial_cleanup(fd);
	return 0;
}