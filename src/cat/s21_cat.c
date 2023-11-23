#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Flags {
  int b;
  int e;
  int n;
  int s;
  int t;
  int v;
};

struct Settings {
  struct Flags flags;
  char **filenames;
  int filenames_count;
};

void show_help() {
  printf("Использование: s21_cat [ПАРАМЕТР]... [ФАЙЛ]...\n\n");
  printf("  %-24s %30s\n", "-b, --number-nonblank",
         "нумерует только непустые строки");
  printf("  %-24s %30s\n", "-e, -E",
         "также отображает символы конца строки как $");
  printf("  %-24s %30s\n", "-n, --number", "нумерует все выходные строки");
  printf("  %-24s %30s\n", "-s, --squeeze-blank",
         "сжимает несколько смежных пустых строк");
  printf("  %-24s %30s\n", "-t,  -T", "также отображает табы как ^I");
  printf("  %-24s %30s\n", "--help", "показать эту справку и выйти");
  printf("\n");
  printf("Примеры:\n");
  printf("  s21_cat -n -e logins.txt\n");
  printf("    >> 1  burialstance$\n");
  printf("    >> 2  tyberora$\n");
};

struct Settings build_settings() {
  struct Flags flags = {0};
  struct Settings settings = {
      .flags = flags, .filenames = NULL, .filenames_count = 0};
  return settings;
}

void free_settings(struct Settings *settings) {
  if (settings->filenames) {
    for (int i = 0; i < settings->filenames_count; i++) {
      free(settings->filenames[i]);
    }

    free(settings->filenames);
    settings->filenames = NULL;
    settings->filenames_count = 0;
  }
}

void show_settings(struct Settings *settings) {
  char SET[] = "\033[0;32mSET\033[0m";
  char UNSET[] = "\033[1;31mUNSET\033[0m";
  printf("summary processed flags\tb: %s, e: %s, n: %s, s: %s, t: %s, v:%s\n",
         settings->flags.b ? SET : UNSET, settings->flags.e ? SET : UNSET,
         settings->flags.n ? SET : UNSET, settings->flags.s ? SET : UNSET,
         settings->flags.t ? SET : UNSET, settings->flags.v ? SET : UNSET);

  printf("total files: %d\n", settings->filenames_count);
  for (int i = 0; i < settings->filenames_count; i++) {
    printf("  >> %s\n", settings->filenames[i]);
  }
}

void append_filename_to_settings(struct Settings *settings, char *filename) {
  settings->filenames = (char **)realloc(
      settings->filenames, (settings->filenames_count + 1) * sizeof(char *));
  if (settings->filenames == NULL) {
    fprintf(stderr, "filenames memory allocate is failed");
    exit(EXIT_FAILURE);
  }

  settings->filenames[settings->filenames_count] = filename;
  settings->filenames_count++;
}

void configure_settings_from_args(struct Settings *settings, int argc,
                                  char **argv) {
  int c, _option_index = 0;
  const char *shortopts = "beEnstTv";
  const struct option longopts[] = {{"help", no_argument, NULL, 'h'},
                                    {"number-nonblank", no_argument, NULL, 'b'},
                                    {"number", no_argument, NULL, 'n'},
                                    {"squeeze-blank", no_argument, NULL, 's'},
                                    {0, 0, 0, 0}};

  while ((c = getopt_long(argc, argv, shortopts, longopts, &_option_index)) !=
         -1) {
    switch (c) {
      case 'h':
        show_help();
        exit(EXIT_SUCCESS);
      case 'b':
        settings->flags.b = 1;
        break;
      case 'e':
        settings->flags.e = 1;
        settings->flags.v = 1;
        break;
      case 'E':
        settings->flags.e = 1;
        break;
      case 'n':
        settings->flags.n = 1;
        break;
      case 's':
        settings->flags.s = 1;
        break;
      case 't':
        settings->flags.t = 1;
        settings->flags.v = 1;
        break;
      case 'T':
        settings->flags.t = 1;
        break;
      case 'v':
        settings->flags.v = 1;
        break;
      default:
        fprintf(stderr,
                "По команде «s21_cat --help» можно получить дополнительную "
                "информацию.\n");
        exit(EXIT_FAILURE);
    }
  }
  if (optind < argc) {
    while (optind < argc) {
      append_filename_to_settings(settings, strdup(argv[optind]));
      optind++;
    }
  }
}

void print_shifted_char(unsigned char c) {
  if (c == 9 || c == 10) {
    printf("%c", c);
  } else if (c >= 32 && c < 127) {
    printf("%c", c);
  } else if (c == 127) {
    printf("^?");
  } else if (c >= 128 + 32) {
    printf("M-");
    (c < 128 + 127) ? printf("%c", c - 128) : printf("^?");
  } else {
    (c > 32) ? printf("M-^%c", c - 128 + 64) : printf("^%c", c + 64);
  }
}

void process_file(char *filename, struct Flags flags) {
  FILE *pFile = fopen(filename, "r");
  if (pFile == NULL) {
    fprintf(stderr,
            "%s Нет такого файла или каталога или у вас недостаточно прав.\n",
            filename);
    exit(EXIT_FAILURE);
  }

  char *row = NULL;
  int row_index = 0;
  int is_current_row_empty = 0;
  int last_empty_row_index = 0;
  int total_empty_rows = 0;
  int is_prev_row_empty = 0;
  ssize_t _read;
  size_t _len = 0;

  while ((_read = getline(&row, &_len, pFile)) != EOF) {
    row_index++;
    is_current_row_empty = strlen(row) == 1;
    if (is_current_row_empty) {
      total_empty_rows++;
      last_empty_row_index = row_index;
    }
    is_prev_row_empty = last_empty_row_index + 1 == row_index && row_index != 1;

    if (flags.b) {
      if (!is_current_row_empty) {
        printf("%*d\t", 6, row_index - total_empty_rows);
      }
      printf("%s", row);
    } else if (flags.n) {
      printf("%*d\t", 6, row_index);
      printf("%s", row);
    } else if (flags.s) {
      if (!is_current_row_empty) {
        if (is_prev_row_empty) {
          printf("\n");
        }
        printf("%s", row);
      }
    } else if (flags.e) {
      for (size_t i = 0; i < strlen(row); i++) {
        if (row[i] == '\n') {
          printf("$");
        }
        flags.v ? print_shifted_char((unsigned char)row[i])
                : printf("%c", row[i]);
      }
    } else if (flags.t) {
      for (size_t i = 0; i < strlen(row); i++) {
        if (row[i] == '\t') {
          printf("^I");
        } else {
          flags.v ? print_shifted_char((unsigned char)row[i])
                  : printf("%c", row[i]);
        }
      }
    } else if (flags.v) {
      for (size_t i = 0; i < strlen(row); i++) {
        print_shifted_char((unsigned char)row[i]);
      }
    } else {
      printf("%s", row);
    }
  }

  fclose(pFile);
  if (row) free(row);
}

int main(int argc, char **argv) {
  struct Settings settings = build_settings();
  configure_settings_from_args(&settings, argc, argv);
  // show_settings(&settings);

  for (int i = 0; i < settings.filenames_count; i++) {
    process_file(settings.filenames[i], settings.flags);
  }

  free_settings(&settings);
  return 0;
}