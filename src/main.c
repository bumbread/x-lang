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

enum {
  TOKEN_EOF = 0,   // end of file
  // all ascii characters are a separate token
  // 0..127
  TOKEN_INT = 128, // integer numbers
  TOKEN_IDN = 129, // identifiers/names
} typedef t_token_kind;

struct {
  t_token_kind kind;
  char const *start;
  char const *end;
  union {
    u64 int_value;
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
  }
  else {
    state->last_token.kind = *state->stream;
    state->offset += 1;
    state->stream += 1;
  }
  char const *end = state->stream;
  state->last_token.start = start;
  state->last_token.end = end;
}

static void token_print(t_token token) {
  if(token.kind < 128 && isprint(token.kind)) {
    printf("'%c'", token.kind);
  }
  else if(token.kind == TOKEN_INT) {
    printf("%d", token.int_value);
  }
  else if(token.kind == TOKEN_IDN) {
    printf("[%.*s]", (int)(token.end - token.start), token.start);
  }
  else if(token.kind == TOKEN_EOF) {
    printf("{EOF}");
  }
  else {
    printf("{invalid token}\n");
  }
}

#include<stdio.h>
#include"memory.c"
int main(void) {
  char const *string = "+()abndk*18888__as2(2)*&";
  t_lexstate state;
  state_init(&state, string);
  
  u64 tokens_count = 0;
  t_stack tokens;
  ptr size = 10*kb;
  stack_init(&tokens, size, malloc(size), sizeof(t_token), 0x100);
  
  do {
    state_parse_next_token(&state);
    
    t_token *next_token = stack_push(&tokens);
    *next_token = state.last_token;
    tokens_count += 1;
    
    token_print(next_token[0]);
    printf("\t\t%p\n", next_token);
  } while(state.last_token.kind != 0);
  
  for(u64 i = 0; i < tokens_count; i += 1) {
    
    printf(" ");
  }
  
  return 0;
}
