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

void reset_error(void) {
  last_error[0] = 0;
}

enum {
  TOKEN_EOF = 0,   // end of file
  // All ascii characters are a separate token
  // 0..127
  TOKEN_INT = 128, // integer numbers
  TOKEN_IDN, // identifiers/names
  // Operators having more than one symbol
  TOKEN_OP_LOG_OR,        // ||
  TOKEN_OP_LOG_AND,       // &&
  TOKEN_OP_REL_EQ,        // ==
  TOKEN_OP_REL_NEQ,       // !=
  TOKEN_OP_REL_GEQ,       // >=
  TOKEN_OP_REL_LEQ,       // <=
  TOKEN_OP_LOG_SHIFTL,    // <<
  TOKEN_OP_LOG_SHIFTR,    // >>
  TOKEN_OP_ARITHM_SHIFTL, // <<<
  TOKEN_OP_ARITHM_SHIFTR, // >>>
  TOKEN_OP_BIG_ARROW,     // =>
  TOKEN_OP_ARROW,         // ->
} typedef t_token_kind;

struct {
  t_token_kind kind;
  char const *start;
  char const *end;
  union {
    i64 int_value;
  };
} typedef t_token;

struct {
  u64 line;
  u64 offset;
  char const *stream;
  t_token last_token;
} typedef t_lexstate;

static void state_init(t_lexstate *state, char const *stream) {
  state->stream = stream;
  t_token eof_token;
  eof_token.start = eof_token.end = null;
  eof_token.kind = TOKEN_EOF;
  state->last_token = eof_token;
  state->line = 0;
  state->offset = 0;
}

static void state_parse_next_token(t_lexstate *state) {
  char const *start = state->stream;
  
  while(true) {
    if(isalpha(*state->stream) || *state->stream == '_') {
      state->last_token.kind = TOKEN_IDN;
      while(isalnum(*state->stream) || *state->stream == '_') {
        state->stream += 1;
        state->offset += 1;
      }
    }
    else if(isdigit(*state->stream)) {
      u64 value = 0;
      state->last_token.kind = TOKEN_INT;
      while(isdigit(*state->stream)) {
        value *= 10;
        value += *state->stream - '0';
        state->stream += 1;
        state->offset += 1;
      }
      state->last_token.int_value = value;
    }
    else if(isspace(*state->stream)) {
      state->stream += 1;
      if(*state->stream == '\n') {
        state->offset = 0;
        state->line += 1;
      }
      continue;
    }
    else { // parse operators.
      
      char first_char = *state->stream;
      state->offset += 1;
      state->stream += 1;
      if(first_char == '|') {
        if(*state->stream == '|') { // ||
          state->offset += 1;
          state->stream += 1;
          state->last_token.kind = TOKEN_OP_LOG_OR;
        }
        else state->last_token.kind = first_char;
      }
      else if(first_char == '&') {
        if(*state->stream == '|') { // &&
          state->offset += 1;
          state->stream += 1;
          state->last_token.kind = TOKEN_OP_LOG_AND;
        }
        else state->last_token.kind = first_char;
      }
      else if(first_char == '=') {
        if(*state->stream == '=') { // ==
          state->offset += 1;
          state->stream += 1;
          state->last_token.kind = TOKEN_OP_REL_EQ;
        }
        else if(*state->stream == '>') { // =>
          state->offset += 1;
          state->stream += 1;
          state->last_token.kind = TOKEN_OP_BIG_ARROW;
        }
        else state->last_token.kind = first_char;
      }
      else if(first_char == '>') {
        if(*state->stream == '=') { // >=
          state->offset += 1;
          state->stream += 1;
          state->last_token.kind = TOKEN_OP_REL_GEQ;
        }
        else if(*state->stream == '>') { // >>
          state->offset += 1;
          state->stream += 1;
          if(*state->stream == '>') { // >>>
            state->last_token.kind = TOKEN_OP_ARITHM_SHIFTR;
          }
          else state->last_token.kind = TOKEN_OP_LOG_SHIFTR;
        }
        else state->last_token.kind = first_char;
      }
      else if(first_char == '<') {
        if(*state->stream == '=') { // <=
          state->offset += 1;
          state->stream += 1;
          state->last_token.kind = TOKEN_OP_REL_LEQ;
        }
        else if(*state->stream == '<') { // <<
          state->offset += 1;
          state->stream += 1;
          if(*state->stream == '<') { // >>>
            state->last_token.kind = TOKEN_OP_ARITHM_SHIFTL;
          }
          else state->last_token.kind = TOKEN_OP_LOG_SHIFTL;
        }
        else state->last_token.kind = first_char;
      }
      else if(first_char == '!') {
        if(*state->stream == '=') {
          state->offset += 1;
          state->stream += 1;
          state->last_token.kind = TOKEN_OP_REL_NEQ;
        }
        else state->last_token.kind = first_char;
      }
      else {
        state->last_token.kind = first_char;
      }
    }
    char const *end = state->stream;
    state->last_token.start = start;
    state->last_token.end = end;
    break;
  }
}

static char *get_nonchar_token_kind_name(t_token_kind kind) {
  if(kind == TOKEN_INT) {return "INT";}
  else if(kind == TOKEN_IDN) {return "NAME";}
  else if(kind == TOKEN_OP_LOG_OR) {return "||";}
  else if(kind == TOKEN_OP_LOG_AND) {return "&&";}
  else if(kind == TOKEN_OP_REL_EQ) {return "==";}
  else if(kind == TOKEN_OP_REL_NEQ) {return "!=";}
  else if(kind == TOKEN_OP_REL_GEQ) {return ">=";}
  else if(kind == TOKEN_OP_REL_LEQ) {return "<=";}
  else if(kind == TOKEN_OP_LOG_SHIFTL) {return "<<";}
  else if(kind == TOKEN_OP_LOG_SHIFTR) {return ">>";}
  else if(kind == TOKEN_OP_ARITHM_SHIFTL) {return "<<<";}
  else if(kind == TOKEN_OP_ARITHM_SHIFTR) {return ">>>";}
  else if(kind == TOKEN_OP_BIG_ARROW) {return "=>";}
  else if(kind == TOKEN_OP_ARROW) {return "->";}
  return "{unknown token}";
}

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
  
  error = false;
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
