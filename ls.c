#include <dirent.h>
#include <errno.h>
#include <error.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  const char *path = argc > 1 ? argv[1] : ".";

  DIR *dir = opendir(path);
  if (!dir) {
    perror("opendir");
    return 1;
  }

  errno = 0;

  struct dirent *entry;
  while ((entry = readdir(dir))) {
    if (entry->d_name[0] == '.')
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
