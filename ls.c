#include <dirent.h>
#include <errno.h>
#include <error.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

void mode_string(mode_t mode, char *str) {
  if (S_ISDIR(mode))
    str[0] = 'd';
  else if (S_ISLNK(mode))
    str[0] = 'l';
  else if (S_ISBLK(mode))
    str[0] = 'b';
  else if (S_ISCHR(mode))
    str[0] = 'c';
  else if (S_ISFIFO(mode))
    str[0] = 'p';
  else if (S_ISSOCK(mode))
    str[0] = 's';
  else
    str[0] = '-';

  str[1] = (mode & S_IRUSR) ? 'r' : '-';
  str[2] = (mode & S_IWUSR) ? 'w' : '-';
  str[3] = (mode & S_IXUSR) ? 'x' : '-';

  str[4] = (mode & S_IRGRP) ? 'r' : '-';
  str[5] = (mode & S_IWGRP) ? 'w' : '-';
  str[6] = (mode & S_IXGRP) ? 'x' : '-';

  str[7] = (mode & S_IROTH) ? 'r' : '-';
  str[8] = (mode & S_IWOTH) ? 'w' : '-';
  str[9] = (mode & S_IXOTH) ? 'x' : '-';
  str[10] = '\0';
}

void print_long(const char *dir, const char *name) {
  char fullpath[4096];
  snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, name);

  struct stat sb;
  if (lstat(fullpath, &sb) < 0) {
    perror("lstat");
    return;
  };

  char mode[11];
  mode_string(sb.st_mode, mode);

  struct passwd *pwd = getpwuid(sb.st_uid);
  struct group *grp = getgrgid(sb.st_gid);

  const char *user = pwd ? pwd->pw_name : "???";
  const char *group = grp ? grp->gr_name : "???";

  long size = sb.st_size;

  printf("%s %s %s %ld %s\n", mode, user, group, size, name);
}

int show_all = 0;
int show_long = 0;

int main(int argc, char *argv[]) {
  int opt;
  while ((opt = getopt(argc, argv, "al")) != -1) {
    switch (opt) {
    case 'a':
      show_all = 1;
      break;
    case 'l':
      show_long = 1;
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

    if (show_long)
      print_long(path, entry->d_name);
    else
      printf("%s\n", entry->d_name);
  }

  if (errno != 0) {
    perror("readdir");
    return 1;
  }

  closedir(dir);
  return 0;
}
