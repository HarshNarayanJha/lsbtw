#include <dirent.h>
#include <errno.h>
#include <error.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

int show_all = 0;
int show_long = 0;

struct stat *get_file_stat(const char *dir, const char *name, struct stat *sp) {
  char fullpath[4096];
  snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, name);

  if (lstat(fullpath, sp) < 0) {
    perror("lstat");
    return NULL;
  }

  return sp;
}

long get_dir_meta(const char *path, unsigned long *max_size_len,
                  unsigned long *total_size) {

  DIR *dir = opendir(path);
  if (!dir) {
    closedir(dir);
    perror("opendir");
    return -1;
  }

  errno = 0;
  long items = 0;

  struct dirent *entry;
  while ((entry = readdir(dir))) {
    if (!show_all && entry->d_name[0] == '.')
      continue;

    struct stat sp;
    if (!get_file_stat(path, entry->d_name, &sp))
      continue;

    unsigned long len = 0;
    unsigned long size = sp.st_size;
    while (size > 0) {
      size /= 10;
      len++;
    }

    *max_size_len = len > *max_size_len ? len : *max_size_len;
    *total_size += sp.st_size;

    items++;
  }

  if (errno != 0) {
    closedir(dir);
    perror("readdir");
    return -1;
  }

  closedir(dir);
  return items;
}

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

void print_long(const char *dir, const char *name, const int max_size_len) {
  struct stat sp;
  if (!get_file_stat(dir, name, &sp))
    return;

  char modes[11];
  mode_string(sp.st_mode, modes);

  struct passwd *pwd = getpwuid(sp.st_uid);
  struct group *grp = getgrgid(sp.st_gid);

  const char *user = pwd ? pwd->pw_name : "???";
  const char *group = grp ? grp->gr_name : "???";

  time_t now_t = time(NULL);
  struct tm now = *localtime(&now_t);

  char timebuf[32];
  struct tm tp = *localtime(&sp.st_mtim.tv_sec);

  if (tp.tm_year == now.tm_year) {
    strftime(timebuf, sizeof(timebuf), "%b %e %H:%M", &tp);
  } else {
    strftime(timebuf, sizeof(timebuf), "%b %e  %Y", &tp);
  }

  printf("%s %lu %s %s %*ld %s  %s\n", modes, sp.st_nlink, user, group,
         max_size_len, sp.st_size, timebuf, name);
}

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
      fprintf(stderr, "Usage: %s [-al] [path]\n", argv[0]);
      return 1;
    }
  }

  const char *path = (optind < argc) ? argv[optind] : ".";

  unsigned long max_size_len = 0, total_size = 0;
  long items = 0;
  if (show_long) {
    if ((items = get_dir_meta(path, &max_size_len, &total_size)) == -1) {
      return 1;
    }

    printf("items %lu total %lu\n", items, total_size);
  }

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
      print_long(path, entry->d_name, max_size_len);
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
