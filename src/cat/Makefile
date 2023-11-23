CC := gcc
CFLAGS := -Wall -Werror -Wextra

SRC := "s21_cat.c"
OUT := s21_cat


s21_cat:
build: 
	$(CC) $(SRC) $(CFLAGS) -o $(OUT)

test: s21_cat
	python3 tests.py

clean:
	rm $(OUT) text_empty.txt text_en.txt text_ru.txt text_punc.txt