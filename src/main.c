
#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<stdarg.h>
#include<ctype.h>
#include<assert.h>
#include<stdlib.h>
#include<stdint.h>


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
  
  t_lexstate state;
  while(true) {
    char buf[1024];
    printf("x-shell$ "); fgets(buf, sizeof buf, stdin);
    state_init(&state, buf);
    
    state_parse_next_token(&state);
    t_ast_node *expr = parse_expr(&state);
    check_errors();
    ast_node_print_lisp(expr);
    u64 result = ast_node_evaluate(expr);
    
    if(error == false) {
      printf("\nresult: %lld\n", result);
    }
    else {
      printf("error: %s\n", last_error);
      error = false;
    }
  }
  
  return 0;
}
