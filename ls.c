#include <dirent.h>
#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <unistd.h>

int show_all = 0;

int main(int argc, char *argv[]) {
  int opt;
  while ((opt = getopt(argc, argv, "a")) != -1) {
    switch (opt) {
    case 'a':
      show_all = 1;
      break;
    default:
      fprintf(stderr, "Usage: %s [-a] [path]\n", argv[0]);
      return 1;
    }
  }

  const char *path = (optind < argc) ? argv[optind] : ".";

  DIR *dir = opendir(path);
  if (!dir) {
    perror("opendir");
    return 1;
  }

  errno = 0;

  struct dirent *entry;
  while ((entry = readdir(dir))) {
    if (!show_all && entry->d_name[0] == '.')
      continue;

    printf("%s\n", entry->d_name);
  }

  if (errno != 0) {
    perror("readdir");
    return 1;
  }

  closedir(dir);
  return 0;
}
