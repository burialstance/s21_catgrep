CC := gcc
CFLAGS := -Wall -Werror -Wextra

SRC := s21_grep.c
OUT := s21_grep


s21_grep:
build: 
	$(CC) $(SRC) $(CFLAGS) -o $(OUT)

test: s21_grep
	python3 tests.py -m

clean:
	rm $(OUT) text_1.txt text_2.txt text_empty.txt

