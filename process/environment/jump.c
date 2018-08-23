#include <setjmp.h>
#include <stdio.h>

int a = 1;
static jmp_buf buf1, buf2;
void func1();
void func2();
void func3();

void func1() {
  a = 1;
  if (setjmp(buf1) != 0) {
    printf("func1: %d\n", a);
  } else {
    func2();
  }
}

void func2() {
  a = 2;
  if (setjmp(buf2) != 0) {
    printf("func2: %d\n", a);
    longjmp(buf1, 1);
  } else {
    func3();
  }
}

void func3() {
  a = 3;
  printf("func2: %d\n", a);
  longjmp(buf2, 1); 
}

int main() {
    func1();
}