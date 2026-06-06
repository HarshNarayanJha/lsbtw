run: build
    ./lsbtw

build:
    gcc -Wall -Wextra -o lsbtw ls.c
