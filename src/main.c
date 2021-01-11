
#include"common.c"
#include<stdio.h>
#include"memory.c"
#include"string.c"
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
