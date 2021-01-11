
#include"common.c"
#include<stdio.h>
#include"memory.c"
#include"string.c"
#include"lexer.c"
#include"parser.c"

#include"vm.c"
#include"test.c"

int main(void) {
  test_lexing();
  test_parsing();
  test_vm();
  test_vm_compiler();
  test_interns();
  
  while(true) {
    char buf[256];
    printf("x-shell$ "); fgets(buf, sizeof buf, stdin);
    u64 result = test_parse_expression(buf);
    if(error == false) {
      printf("%lld\n", result);
    }
    else {
      printf("error: %s\n", last_error);
      error = false;
    }
  }
  
  return 0;
}
