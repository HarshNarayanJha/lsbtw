#include <dirent.h>
#include <error.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  const char *path = argc > 1 ? argv[1] : ".";

  DIR *dir = opendir(path);
  if (!dir) {
    perror("opendir");
    return 1;
  }

  closedir(dir);
  return 0;
}
