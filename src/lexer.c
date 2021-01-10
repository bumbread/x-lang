
enum {
  TOKEN_EOF = 0,   // end of file
  // All ascii characters are a separate token
  // 0..127
  TOKEN_INT = 128, // integer numbers
  TOKEN_FLT,
  TOKEN_IDN, // identifiers/names
  // Operators having more than one symbol
  TOKEN_OP_LOG_OR,           // ||
  TOKEN_OP_LOG_AND,          // &&
  TOKEN_OP_REL_EQ,           // ==
  TOKEN_OP_REL_NEQ,          // !=
  TOKEN_OP_REL_GEQ,          // >=
  TOKEN_OP_REL_LEQ,          // <=
  TOKEN_OP_LSHIFTL,          // <<
  TOKEN_OP_LSHIFTL_ASSIGN,   // <<=
  TOKEN_OP_LSHIFTR,          // >>
  TOKEN_OP_LSHIFTR_ASSIGN,   // >>=
  TOKEN_OP_ASHIFTL,          // <<<
  TOKEN_OP_ASHIFTL_ASSIGN,   // <<<=
  TOKEN_OP_ASHIFTR,          // >>>
  TOKEN_OP_ASHIFTR_ASSIGN,   // >>>=
  TOKEN_OP_BIG_ARROW,        // =>
  TOKEN_OP_ARROW,            // ->
  TOKEN_OP_REVERSE_ARROW,    // <-
} typedef t_token_kind;

struct {
  t_token_kind kind;
  char const *start;
  char const *end;
  union {
    i64 int_value;
    f64 flt_value;
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

static inline void state_next_char(t_lexstate *state) {
  state->stream += 1;
}

static inline bool state_match_char(t_lexstate *state, char c) {
  if(*state->stream == c) {
    state_next_char(state);
    return true;
  }
  return false;
}

static u64 char_to_digit[256] = {
  ['0'] = 0,
  ['1'] = 1,
  ['2'] = 2,
  ['3'] = 3,
  ['4'] = 4,
  ['5'] = 5,
  ['6'] = 6,
  ['7'] = 7,
  ['8'] = 8,
  ['9'] = 9,
  ['a'] = 10,['A'] = 10,
  ['b'] = 11,['B'] = 11,
  ['c'] = 12,['C'] = 12,
  ['d'] = 13,['D'] = 13,
  ['e'] = 14,['E'] = 14,
  ['f'] = 15,['F'] = 15,
};

static u64 parse_integer(u64 base, char const *stream, char const **end) {
  u64 value = 0;
  u64 digit;
  while((digit = char_to_digit[*stream]) && (digit != 0 || *stream == '0')) {
    value = base*value + digit;
  }
  *end = stream;
  return value;
}

static void state_parse_next_token(t_lexstate *state) {
  while(isspace(*state->stream)) {
    state_next_char(state);
    continue;
  }
  char const *start = state->stream;
  while(true) {
    if(isalpha(*state->stream) || *state->stream == '_') {
      state->last_token.kind = TOKEN_IDN;
      while(isalnum(*state->stream) || *state->stream == '_') {
        state_next_char(state);
      }
    }
    else if(isdigit(*state->stream)) {
      while(isdigit(*state->stream)) state->stream+=1;
      
      if(*state->stream != '.') { //integer value
        state->stream = start;
        
        u64 base = 10;
        if(state->stream[0] == '0') {
          if(state->stream[1] == 'x') base = 16;
          else if(state->stream[1] == 'o') base = 8;
          else if(state->stream[1] == 'b') base = 2;
        }
        
        state->last_token.int_value = parse_integer(base, state->stream, &state->stream);
        state->last_token.kind = TOKEN_INT;
      }
      else { // floating point value
        state->stream = start;
        f64 val = strtod(start, (char **)&state->stream);
        state->last_token.flt_value = val;
        state->last_token.kind = TOKEN_FLT;
      }
    }
    else { // parse operators.
      //state_next_char(state);
      if(state_match_char(state, '|')) {
        if(state_match_char(state, '|')) state->last_token.kind = TOKEN_OP_LOG_OR;
        else state->last_token.kind = '|';
      }
      else if(state_match_char(state, '&')) {
        if(state_match_char(state, '&')) state->last_token.kind = TOKEN_OP_LOG_AND;
        else state->last_token.kind = '&';
      }
      else if(state_match_char(state, '=')) {
        if(state_match_char(state, '=')) state->last_token.kind = TOKEN_OP_REL_EQ;
        else if(state_match_char(state, '>')) state->last_token.kind = TOKEN_OP_BIG_ARROW;
        else state->last_token.kind = '=';
      }
      else if(state_match_char(state, '>')) {
        if(state_match_char(state, '=')) state->last_token.kind = TOKEN_OP_REL_GEQ;
        else if(state_match_char(state, '>')) {
          if(state_match_char(state, '>')) {
            if(state_match_char(state, '=')) state->last_token.kind = TOKEN_OP_ASHIFTR_ASSIGN;
            else state->last_token.kind = TOKEN_OP_ASHIFTR;
          }
          else if(state_match_char(state, '=')) state->last_token.kind = TOKEN_OP_LSHIFTR_ASSIGN;
          else state->last_token.kind = TOKEN_OP_LSHIFTR;
        }
        else state->last_token.kind = '>';
      }
      else if(state_match_char(state, '<')) {
        if(state_match_char(state, '=')) state->last_token.kind = TOKEN_OP_REL_LEQ;
        else if(state_match_char(state, '-')) state->last_token.kind = TOKEN_OP_REVERSE_ARROW;
        else if(state_match_char(state, '<')) {
          if(state_match_char(state, '<')) {
            if(state_match_char(state, '=')) state->last_token.kind = TOKEN_OP_ASHIFTL_ASSIGN;
            else state->last_token.kind = TOKEN_OP_ASHIFTL;
          }
          else if(state_match_char(state, '=')) state->last_token.kind = TOKEN_OP_LSHIFTL_ASSIGN;
          else state->last_token.kind = TOKEN_OP_LSHIFTL;
        }
        else state->last_token.kind = '<';
      }
      else if(state_match_char(state, '!')) {
        if(state_match_char(state, '=')) state->last_token.kind = TOKEN_OP_REL_NEQ;
        else state->last_token.kind = '!';
      }
      else {
        state->last_token.kind = *state->stream;
        state_next_char(state);
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
  else if(kind == TOKEN_OP_LOG_OR)  {return "||";}
  else if(kind == TOKEN_OP_LOG_AND) {return "&&";}
  else if(kind == TOKEN_OP_REL_EQ)  {return "==";}
  else if(kind == TOKEN_OP_REL_NEQ) {return "!=";}
  else if(kind == TOKEN_OP_REL_GEQ) {return ">=";}
  else if(kind == TOKEN_OP_REL_LEQ) {return "<=";}
  else if(kind == TOKEN_OP_LSHIFTL) {return "<<";}
  else if(kind == TOKEN_OP_LSHIFTR) {return ">>";}
  else if(kind == TOKEN_OP_ASHIFTL) {return "<<<";}
  else if(kind == TOKEN_OP_ASHIFTR) {return ">>>";}
  else if(kind == TOKEN_OP_LSHIFTL_ASSIGN) {return "<<=";}
  else if(kind == TOKEN_OP_LSHIFTR_ASSIGN) {return ">>=";}
  else if(kind == TOKEN_OP_ASHIFTL_ASSIGN) {return "<<<=";}
  else if(kind == TOKEN_OP_ASHIFTR_ASSIGN) {return ">>>=";}
  else if(kind == TOKEN_OP_BIG_ARROW) {return "=>";}
  else if(kind == TOKEN_OP_ARROW) {return "->";}
  else if(kind == 0) {return "EOF";}
  return "{unknown token}";
}
