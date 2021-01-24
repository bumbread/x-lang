
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
#include"checker.c"
#include"test.c"

int main(void) {
  test_lexing();
  test_interns();
  
  init_interns(malloc);
  parser_init_memory(10*mb, malloc(10*mb));
  
  byte *buf;
  char const *filename = "test.x";
  FILE *input = fopen(filename, "rb");
  if(null == input) {
    printf("filename '%s' not found\n", filename);
  }
  
  fseek(input, 0, SEEK_END);
  size_t input_size = ftell(input);
  fseek(input, 0, SEEK_SET);
  buf = malloc(input_size);
  fread(buf, input_size, 1, input);
  
  t_lexstate state;
  lex_init(&state, buf);
  lex_next_token(&state);
  t_ast_node *code = parse_stmts(&state);
  check_errors();
  ast_node_print_lisp(code, 0);
  
  return 0;
}
