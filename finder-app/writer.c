#include <stdio.h>
#include <syslog.h>

int main(int argc, char** argv) {

  FILE *f;

  openlog(NULL, 0, LOG_USER);

  if (argc < 3) {
    syslog(LOG_ERR, "String not specified");
    return 1;
  }

  if (argc < 2) {
    syslog(LOG_ERR, "Path not specified");
    return 1;
  }

  f = fopen(argv[1], "w");

  fprintf(f, "%s", argv[2]);
  syslog(LOG_DEBUG, "Writing %s to %s", argv[2], argv[1]);
  
  return 0;
}
