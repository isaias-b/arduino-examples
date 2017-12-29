#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>     //Used for UART
#include <fcntl.h>      //Used for UART
#include <termios.h>    //Used for UART
#include <time.h>

#include <sys/time.h>
#include <sys/resource.h>

#define BUFFER_SIZE 2048
#define RUN_INFINITELY -1

static volatile int running = 1;
void process_step(int);
void int_signal_handler(int);

#ifdef USE_BPS

struct timeval rt1;
clock_t t1;
void setup_time() {
  t1 = clock();
  gettimeofday(&rt1, NULL);
}

void process_step(int fd) {
  static struct timeval rt2;
  static clock_t t2;
  static long bytes_read = 0;
  static long zeros_read = 0;
  static unsigned char rx_buffer[BUFFER_SIZE];
  if (fd != -1) {
    int rx_length = read(fd, (void*)rx_buffer, BUFFER_SIZE);
    if (rx_length < 0) {
      printf("ERROR<\r");
    } else if (rx_length == 0) {
      zeros_read++;
    } else {
      t2 = clock();
      gettimeofday(&rt2, NULL);
      float cpu_elapsed = ((float)(t2 - t1) / CLOCKS_PER_SEC );
      long elapsed = (long int)rt2.tv_sec - (long int)rt1.tv_sec;
      bytes_read += rx_length;
      float rate = (float) bytes_read / elapsed;
      float cpu_rate = (float) bytes_read / cpu_elapsed;
      rx_buffer[rx_length] = '\0';
      printf("stats:    rate = %2.1fkB/s    time = %2lds    bytes = %ldkB    zeros=%ld    cpurate = %2.1fkB/s    cputime = %2.0fs    \r",
        rate / 1000,
        elapsed,
        bytes_read / 1000,
        zeros_read,
        cpu_rate / 1000,
        cpu_elapsed
      );
    }
  } else {
    printf("nothing to read\r");
  }
}

#else

void process_step(int fd) {
  static unsigned char rx_buffer[BUFFER_SIZE];
  if (fd != -1) {
    int rx_length = read(fd, (void*)rx_buffer, BUFFER_SIZE);
    if (rx_length < 0) {
      printf("no bytes to read (error)\n");
    } else if (rx_length == 0) {
      printf("zero bytes to read (empty)\n");
    } else {
      rx_buffer[rx_length] = '\0';
      printf("%s", rx_buffer);
    }
  } else {
    printf("nothing to read\n");
  }
}

#endif

int main(int argc, char *argv[]) {
  int fd = -1;
  long read_bytes = RUN_INFINITELY;
  char *port = "/dev/ttyACM1";
  unsigned char rx_buffer[BUFFER_SIZE];
  setpriority(PRIO_PROCESS, 0, -20);

  if (argc > 1) {
    printf("overwrite default port %s with %s\n", port, argv[1]);
    port = argv[1];
  }

  if (argc > 2) {
    char *end;
    long b2r = strtol(argv[2], &end, 10);
    if (errno == ERANGE) errno = 0;
    else if(b2r < 0) {
      printf("overwrite default #bytes-to-read %ld with RUN_INFINITELY\n", read_bytes);
      read_bytes = RUN_INFINITELY;
    }
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

  #ifdef USE_BPS
  setup_time();
  #endif

  signal(SIGINT, int_signal_handler);
  if(read_bytes == RUN_INFINITELY) {
    printf("now reading bytes...\n");
    while (running) process_step(fd);
  } else {
    printf("now reading %ld bytes...\n", read_bytes);
    for (long i = 0; i < read_bytes && running; i++) process_step(fd);
  }
  printf("\nclosing port %s...\n", port);
  close(fd);

  return 0;
}

void int_signal_handler(int signal) {
  running = 0;
}
