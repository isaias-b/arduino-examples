#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>     //Used for UART
#include <fcntl.h>      //Used for UART
#include <termios.h>    //Used for UART

//At bootup, pins 8 and 10 are already set to UART0_TXD, UART0_RXD (ie the alt0 function) respectively
//The flags (defined in fcntl.h):
//  Access modes (use 1 of these):
//    O_RDONLY - Open for reading only.
//    O_RDWR - Open for reading and writing.
//    O_WRONLY - Open for writing only.
//
//  O_NDELAY / O_NONBLOCK (same function) - Enables nonblocking mode. When set read requests on the file can return immediately with a failure status
//                      if there is no input immediately available (instead of blocking). Likewise, write requests can also return
//                      immediately with a failure status if the output can't be written immediately.
//
//  O_NOCTTY - When set and path identifies a terminal device, open() shall not cause the terminal device to become the controlling terminal for the process.

//CONFIGURE THE UART
//The flags (defined in /usr/include/termios.h - see http://pubs.opengroup.org/onlinepubs/007908799/xsh/termios.h.html):
//  Baud rate:- B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B500000, B576000, B921600, B1000000, B1152000, B1500000, B2000000, B2500000, B3000000, B3500000, B4000000
//  CSIZE:- CS5, CS6, CS7, CS8
//  CLOCAL - Ignore modem status lines
//  CREAD - Enable receiver
//  IGNPAR = Ignore characters with parity errors
//  ICRNL - Map CR to NL on input (Use for ASCII comms where you want to auto correct end of line characters - don't use for bianry comms!)
//  PARENB - Parity enable
//  PARODD - Odd parity (else even)

int main(int argc, char *argv[]) {
  int fd = -1;
  long read_bytes = 5000;
  char *port = "/dev/ttyACM1";

  if (argc > 1) {
    printf("overwrite default port %s with %s\n", port, argv[1]);
    port = argv[1];
  }

  if (argc > 2) {
    char *end;
    long b2r = strtol(argv[2], &end, 10);
    if (errno == ERANGE) errno = 0;
    else {
      printf("overwrite default #bytes-to-read %ld with %ld\n", read_bytes, b2r);
      read_bytes = b2r;
    }
  }

  printf("try opening serial port %s...\n", port);
  fd = open(port, O_RDWR | O_NOCTTY);    // Open in non blocking read/write mode
  if (fd == -1) {
    printf("Error - Unable to open UART.  Ensure it is not in use by another application\n");
  }

  struct termios options;
  tcgetattr(fd, &options);
  options.c_cflag = B230400 | CS8 | CLOCAL | CREAD;   // Set baud rate
  options.c_iflag = IGNPAR;
  options.c_oflag = 0;
  options.c_lflag = 0;
  tcflush(fd, TCIFLUSH);
  tcsetattr(fd, TCSANOW, &options);

  printf("now reading %ld bytes...\n", read_bytes);
  for (long i = 0; i < read_bytes; i++) {
    if (fd != -1) {
      unsigned char rx_buffer[256];
      int rx_length = read(fd, (void*)rx_buffer, 255);
      if (rx_length < 0) {
        printf("no bytes to read (error)\r");
      } else if (rx_length == 0) {
        printf("zero bytes to read (empty)\r");
      } else {
        rx_buffer[rx_length] = '\0';
        printf("%s", rx_buffer);
      }
    } else {
      printf("nothing to read\n");
    }
  }

  printf("\nclosing port %s...\n", port);
  close(fd);

  return 0;
}
