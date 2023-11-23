#include <getopt.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void show_help(char *process_name) {
  printf("Использование: %s [ПАРАМЕТР]... [ШАБЛОНЫ] [ФАЙЛ]...\n", process_name);
  printf("  %-12s %12s\n", "-e", "Шаблон");
  printf("  %-12s %12s\n", "-i", "Игнорирует различия регистра");
  printf("  %-12s %12s\n", "-v", "Инвертирует смысл поиска соответствий");
  printf("  %-12s %12s\n", "-c", "Выводит только количество совпадающих строк");
  printf("  %-12s %12s\n", "-l", "Выводит только совпадающие файлы");
  printf("  %-12s %12s\n", "-n",
         "Предваряет каждую строку вывода номером строки из файла ввода");
  printf("  %-12s %12s\n", "-h",
         "Выводит совпадающие строки, не предваряя их именами файлов");
  printf(
      "  %-12s %12s\n", "-s",
      "Подавляет сообщения об ошибках о несуществующих или нечитаемых файлах");
  printf("  %-12s %12s\n", "-f <file>",
         "Получает регулярные выражения из файла");
  printf("  %-12s %12s\n", "-o",
         "Печатает только совпадающие (непустые) части совпавшей строки");
};

struct Flags {
  int e;
  int i;
  int v;
  int c;
  int l;
  int n;
  int h;
  int s;
  int f;
  int o;
};

struct Settings {
  struct Flags flags;

  char **filenames;
  int filenames_count;

  regex_t *patterns;
  int patterns_count;
};

struct Settings build_settings() {
  struct Flags flags = {0};
  struct Settings settings = {.flags = flags,
                              .filenames = NULL,
                              .filenames_count = 0,
                              .patterns = NULL,
                              .patterns_count = 0};
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

  if (settings->patterns) {
    for (int i = 0; i < settings->patterns_count; i++) {
      regfree(&settings->patterns[i]);
    }

    free(settings->patterns);
    settings->patterns = NULL;
    settings->patterns_count = 0;
  }
}

void show_settings(const struct Settings *settings) {
  char SET[] = "\033[0;32mSET\033[0m";
  char UNSET[] = "\033[1;31mUNSET\033[0m";
  printf("e: %s, ", settings->flags.e ? SET : UNSET);
  printf("i: %s, ", settings->flags.i ? SET : UNSET);
  printf("v: %s, ", settings->flags.v ? SET : UNSET);
  printf("c: %s, ", settings->flags.c ? SET : UNSET);
  printf("l: %s, ", settings->flags.l ? SET : UNSET);
  printf("n: %s, ", settings->flags.n ? SET : UNSET);
  printf("h: %s, ", settings->flags.h ? SET : UNSET);
  printf("s: %s, ", settings->flags.s ? SET : UNSET);
  printf("o: %s, ", settings->flags.o ? SET : UNSET);
  printf("\n");

  if (settings->filenames) {
    printf("total files: %d\n", settings->filenames_count);
    for (int i = 0; i < settings->filenames_count; i++) {
      printf("  >> %s\n", settings->filenames[i]);
    }
  }

  if (settings->patterns) {
    printf("total patterns: %d\n", settings->patterns_count);
  }
}

void append_pattern_to_settings(struct Settings *settings, char *pattern) {
  settings->patterns = (regex_t *)realloc(
      settings->patterns, (settings->patterns_count + 1) * sizeof(regex_t));
  if (settings->patterns == NULL) {
    fprintf(stderr, "pattern memory allocate is failed");
    exit(EXIT_FAILURE);
  }

  int cflags = settings->flags.i ? REG_ICASE : REG_EXTENDED;
  if (regcomp(&settings->patterns[settings->patterns_count], pattern, cflags)) {
    fprintf(stderr, "Could not compile regex pattern: %s\n", pattern);
    exit(EXIT_FAILURE);
  }

  settings->patterns_count++;
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

void read_patterns_from_file(struct Settings *settigns, char *filename) {
  FILE *pFile = fopen(filename, "r");
  if (pFile == NULL) {
    fprintf(stderr, "%s Нет такого файла или у вас недостаточно прав.\n",
            filename);
    exit(EXIT_FAILURE);
  }

  char *row = NULL;
  ssize_t _read;
  size_t _len = 0;
  while ((_read = getline(&row, &_len, pFile)) != EOF) {
    int length = strlen(row);
    if (row[length - 1] == '\n') row[length - 1] = '\0';
    append_pattern_to_settings(settigns, row);
  }

  fclose(pFile);
  if (row) free(row);
}

void configure_settings_from_cmd_args(struct Settings *settings, int argc,
                                      char **argv) {
  char *process_name = argv[0];
  int c, _option_index = 0;
  const char *shortopts = "e:ivclnhsf:o";
  const struct option longopts[] = {{"help", no_argument, NULL, '_'},
                                    {0, 0, 0, 0}};

  char *patterns_filename = NULL;
  char **patterns = NULL;
  int patterns_count = 0;

  while ((c = getopt_long(argc, argv, shortopts, longopts, &_option_index)) !=
         -1) {
    switch (c) {
      case '_':
        show_help(process_name);
        exit(EXIT_SUCCESS);

      case 'e':
        settings->flags.e = 1;
        patterns =
            (char **)realloc(patterns, sizeof(char *) * (patterns_count + 1));
        patterns[patterns_count] = strdup(optarg);
        patterns_count++;
        break;

      case 'i':
        settings->flags.i = 1;
        break;
      case 'v':
        settings->flags.v = 1;
        break;
      case 'c':
        settings->flags.c = 1;
        break;
      case 'l':
        settings->flags.l = 1;
        break;
      case 'n':
        settings->flags.n = 1;
        break;
      case 'h':
        settings->flags.h = 1;
        break;
      case 's':
        settings->flags.s = 1;
        break;
      case 'f':
        settings->flags.f = 1;
        patterns_filename = strdup(optarg);
        break;
      case 'o':
        settings->flags.o = 1;
        break;
      case '?':
      default:
        fprintf(stderr,
                "По команде «%s --help» можно получить дополнительную "
                "информацию.\n",
                process_name);
        exit(EXIT_FAILURE);
    }
  }

  if (patterns) {
    for (int i = 0; i < patterns_count; i++) {
      append_pattern_to_settings(settings, patterns[i]);
      free(patterns[i]);
    }
    free(patterns);
  }

  if (patterns_filename) {
    read_patterns_from_file(settings, patterns_filename);
    free(patterns_filename);
  }

  if (optind < argc) {
    if (settings->patterns == NULL) {
      append_pattern_to_settings(settings, argv[optind]);
      optind++;
    }

    while (optind < argc) {
      append_filename_to_settings(settings, strdup(argv[optind]));
      optind++;
    }
  }
}

char *match(char *text, regex_t pattern) {
  regmatch_t pmatch[1];
  char *matched = NULL;
  if (regexec(&pattern, text, 1, pmatch, 0) == 0) {
    int start = pmatch[0].rm_so;
    int end = pmatch[0].rm_eo;
    matched = malloc(end - start + 1);
    strncpy(matched, text + start, end - start);
    matched[end - start] = '\0';
  }
  return matched;
}

int process_file(char *filename, const struct Settings settings) {
  FILE *pFile = fopen(filename, "r");
  if (pFile == NULL) {
    if (settings.flags.s) {
      return 0;
    }
    fprintf(stderr,
            "%s Нет такого файла или каталога или у вас недостаточно прав.\n",
            filename);
    exit(EXIT_FAILURE);
  }

  char *row = NULL;
  int row_num = 0;
  ssize_t _read;
  size_t _len = 0;

  int total_matched_lines = 0;

  while ((_read = getline(&row, &_len, pFile)) != EOF) {
    row_num++;
    char *matched = NULL;

    for (int i = 0; i < settings.patterns_count; i++) {
      matched = match(row, settings.patterns[i]);
      int condition = settings.flags.v ? (matched == NULL) : (matched != NULL);

      if (condition) {
        total_matched_lines++;

        if (settings.flags.l) {
          printf("%s\n", filename);
          fclose(pFile);
          if (row) free(row);
          if (matched) free(matched);
          return total_matched_lines;
        }

        if (settings.flags.c) {
          continue;
        }

        if (settings.flags.o) {
          if (matched) {
            if (settings.filenames_count > 1) {
              if (settings.flags.n) {
                if (settings.flags.h) {
                  printf("%d:%s\n", row_num, matched);
                } else {
                  printf("%s:%d:%s\n", filename, row_num, matched);
                }
              } else {
                if (settings.flags.h) {
                  printf("%s\n", matched);
                } else {
                  printf("%s:%s\n", filename, matched);
                }
              }
            } else {
              if (settings.flags.n) {
                printf("%d:%s\n", row_num, matched);
              } else {
                printf("%s\n", matched);
              }
            }
          }
          continue;
        }

        if (settings.filenames_count > 1 && (!settings.flags.h)) {
          printf("%s:", filename);
        }

        if (settings.flags.n) {
          printf("%d:", row_num);
        }

        printf("%s", row);
        if (row[strlen(row) - 1] != '\n' && row_num > 0) {
          printf("\n");
        }
      }
    }
    if (matched) free(matched);
  }

  if (settings.flags.c && (!settings.flags.l)) {
    if (settings.filenames_count > 1 && (!settings.flags.h)) {
      printf("%s:%d\n", filename, total_matched_lines);
    } else {
      printf("%d\n", total_matched_lines);
    }
  }

  fclose(pFile);
  if (row) free(row);
  return total_matched_lines;
}

int main(int argc, char **argv) {
  struct Settings settings = build_settings();
  configure_settings_from_cmd_args(&settings, argc, argv);
  // show_settings(&settings);

  int total_matched_lines = 0;
  for (int i = 0; i < settings.filenames_count; i++) {
    int matched_lines = process_file(settings.filenames[i], settings);
    total_matched_lines += matched_lines;
  }

  free_settings(&settings);
  return !(total_matched_lines != 0);
}