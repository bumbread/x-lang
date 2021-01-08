#include<ctype.h>
#include<assert.h>
#include<stdlib.h>

#include<stdint.h>
int8_t   typedef i8;
uint8_t  typedef u8;
int16_t  typedef i16;
uint16_t typedef u16;
int32_t  typedef i32;
uint32_t typedef u32;
int64_t  typedef i64;
uint64_t typedef u64;
float    typedef f32;
double   typedef f64;
u8       typedef byte;
u16      typedef word;
u32      typedef bool;
u64      typedef ptr;
#define  true    ((bool)1)
#define  false   ((bool)0)
#define  null    ((void *)0)
#define  kb      1024
#define  mb      1024*kb
#define  gb      1024*mb

#include<stdarg.h>
void panicf(char *message, ...) {
  va_list args;
  va_start(args, message);
  vprintf(message, args);
  va_end(args);
  exit(1);
}

static bool error = false;
static char last_error[1024];
void set_errorf(char const *message, ...) {
  va_list args;
  va_start(args, message);
  vsprintf(last_error, message, args);
  va_end(args);
  error = true;
}

void check_errors(void) {
  if(error == true) {
    printf("error: %s\n", last_error);
    error = false;
  }
}

#include"lexer.c"

static inline bool token_is(t_lexstate *state, t_token_kind kind) {
  return state->last_token.kind == kind;
}

static inline bool token_match(t_lexstate *state, t_token_kind kind) {
  if(state->last_token.kind == kind) {
    state_parse_next_token(state);
    return true;
  }
  return false;
}

// what about --3? 
// note: doesn't work

//
// expr3 = val | '(' expr0 ')'
// expr2 = expr3 | -expr3
// expr1 = expr2 [('*'|'/') expr2 ...]
// expr0 = expr1 [('+'|'-') expr1 ...]
// expr  = expr2
//

static i64 parse_expr0(t_lexstate *state);

static i64 parse_expr3(t_lexstate *state) {
  if(token_match(state, TOKEN_INT)) {
    return state->last_token.int_value;
  }
  else if(token_match(state, '(')) {
    i64 res = parse_expr0(state);
    if(false == token_match(state, ')')) {
      set_errorf("fatal: unmatched parenthesis");
    }
    return res;
  }
  else {
    set_errorf("fatal: unexpected token %s, expected INT or '('", 
               get_nonchar_token_kind_name(state->last_token.kind));
  }
  return 0;
}

static i64 parse_expr2(t_lexstate *state) {
  if(token_match(state, '-')) return -parse_expr3(state); 
  else return parse_expr3(state);
}

static i64 parse_expr1(t_lexstate *state) {
  i64 val = parse_expr2(state);
  while(token_is(state, '*') || token_is(state, '/')) {
    char op = state->last_token.kind;
    state_parse_next_token(state);
    i64 rval = parse_expr2(state);
    if(op == '*') val *= rval;
    if(op == '/') {
      if (rval == 0) {
        set_errorf("error: division by zero");
        return 0;
      }
      val /= rval;
    }
  }
  return val;
}

static i64 parse_expr0(t_lexstate *state) {
  i64 val = parse_expr1(state);
  while(token_is(state, '+') || token_is(state, '-')) {
    char op = state->last_token.kind;
    state_parse_next_token(state);
    i64 rval = parse_expr1(state);
    if(op == '+') val += rval;
    if(op == '-') val -= rval;
  }
  return val;
}

static i64 parse_expr(t_lexstate *state) {
  i64 result = parse_expr0(state);
  if(state->last_token.kind != TOKEN_EOF) {
    set_errorf("not an expression!\n");
  }
  return result;
}

#include<stdio.h>
#include"memory.c"
#include"vm.c"
#include"test.c"
int main(void) {
  test_lexing();
  test_parsing();
  test_vm();
  test_vm_compiler();
  
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
