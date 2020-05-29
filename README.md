# SimpleLinuxShell

*** HUONG DAN SU DUNG ***

- Ngon ngu C, moi truong Linux.
- Cac thu vien ho tro trong viec lap trinh
  * Cac thu vien lap trinh ngon ngu C: <stdio.h>, <string.h>, <stdlib.h>, <stdbool.h>, <stddef.h>.
  * Cac thu vien he thong cua Linux: <unistd.h>, <sys/types.h>, <sys/wait.h>, <fcntl.h>.
- Bien dien va chay chuong trinh:
  * Cai mot so thu vien can thiet truoc khi chay:
    -> Cai thu vien readline bang lenh Linux:
    
    1. sudo apt-get install libreadline-dev

    -> Cai trinh bien dich gcc bang lenh Linux:
    1. sudo apt update
    2. sudo apt install build-essential
    3. sudo apt-get install manpages-dev
    4. gcc --version (kiem tra trinh bien dich GCC da cai dat chua)

  * Bien dich va chay chuong trinh (filename.c) tren terminal bang cac dong lenh sau:
  a. gcc -Wall filename.c -lreadline -o filename
  b. ./filename
