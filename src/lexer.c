
struct {
  char const *filename;
  char const *stream;
  t_token last_token;
  t_location loc;
} typedef t_lexstate;

static void lex_init(t_lexstate *state, char const *filename, char const *stream) {
  state->filename = filename;
  state->stream = stream;
  state->loc.line = 1;
  state->loc.offset = 1;
  t_token eof_token; // to be removed later
  eof_token.start = eof_token.end = null;
  eof_token.kind = TOKEN_eof;
  state->last_token = eof_token;
}

static inline char const lex_next_char(t_lexstate *state) {
  char result = *state->stream;
  state->stream += 1;
  state->loc.offset += 1;
  if(result == '\n') {
    state->loc.line += 1;
    state->loc.offset = 0;
  }
  return result;
}

static inline bool lex_match_char(t_lexstate *state, char c) {
  if(*state->stream == c) {
    lex_next_char(state);
    return true;
  }
  return false;
}

static u64 lex_integer(u64 base, char const *stream, char const **end) {
  u64 value = 0;
  while(is_digit_in_base(base, *stream)) {
    value = base*value + char_to_digit[*stream];
    stream += 1;
  }
  *end = stream;
  return value;
}

static char lex_string_escape(char const *stream, char const **end) {
  char val;
  if(is_digit_in_base(16, *stream)) {
    u64 result = lex_integer(16, stream, &stream);
    if(result > 0xff) {
      push_errorf("value %x is unacceptable for a char literal", result);
    }
    val = (char)result;
  }
  else {
    val = (char)escape_char[*stream++];
  }
  *end = stream;
  return val;
}

static void lex_next_token(t_lexstate *state) {
  while(isspace(*state->stream)) {
    lex_next_char(state);
  }
  
  char const *start = state->stream;
  
  // lex identifiers
  if(isalpha(*state->stream) || *state->stream == '_') {
    state->last_token.kind = TOKEN_idn;
    while(isalnum(*state->stream) || *state->stream == '_') {
      lex_next_char(state);
    }
    char const *end = state->stream;
    t_intern const *string_value = intern_string(start, end);
    state->last_token.str_value = string_value;
  }
  
  // lex number literals
  else if(isdigit(*state->stream)) {
    
    // searching the '.' to find whether the number is flt
    while(isdigit(*state->stream)) state->stream+=1;
    if(*state->stream == '.') {
      state->stream = start;
      f64 val = strtod(start, (char **)&state->stream);
      state->last_token.flt_value = val;
      state->last_token.kind = TOKEN_flt;
    }
    else {
      state->stream = start;
      u64 base = 10;
      if(state->stream[0] == '0') {
        if(tolower(state->stream[1]) == 'x') base = 16;
        else if(tolower(state->stream[1] == 'o')) base = 8;
        else if(tolower(state->stream[1] == 'b')) base = 2;
      }
      state->last_token.int_value = lex_integer(base, state->stream, &state->stream);
      //state->last_token.subkind = SUBKIND_int;
      state->last_token.kind = TOKEN_int;
    }
  }
  else if(lex_match_char(state, '\'')) { // character literal
    u64 val;
    if(lex_match_char(state, '\\')) {
      val = lex_string_escape(state->stream, &state->stream);
    }
    else if(isprint(*state->stream)) {
      val = *state->stream;
      state->stream += 1;
    }
    else {
      push_errorf("value %x is unacceptable for a char literal", *state->stream);
    }
    state->last_token.kind = TOKEN_int;
    //state->last_token.subkind = SUBKIND_char;
    state->last_token.int_value = val;
    if(!lex_match_char(state, '\'')) {
      push_errorf("expected a char literal to close");
    }
  }
  else if(lex_match_char(state, '"')) { // string literal
    string_builder_start();
    while(true) {
      //char c = state_next_char(state);
      if(lex_match_char(state, '"')) break;
      else if(lex_match_char(state, '\\')) {
        string_builder_append_char(lex_string_escape(state->stream, &state->stream));
      }
      else {
        string_builder_append_char(*state->stream++);
      }
    }
    char *result = string_builder_finish();
    t_intern const *intern = intern_string(result, result + string_builder.len);
    state->last_token.str_value = intern;
    state->last_token.kind = TOKEN_str;
  }
  else { // parse operators.
    
    if(lex_match_char(state, '=')) {
      state->last_token.kind = '=';
      if(lex_match_char(state, '=')) {
        state->last_token.kind = TOKEN_cmp_eq;
      }
    }
    else if(lex_match_char(state, '!')) {
      state->last_token.kind = '!';
      if(lex_match_char(state, '=')) {
        state->last_token.kind = TOKEN_cmp_neq;
      }
    }
    else if(lex_match_char(state, '>')) {
      state->last_token.kind = '>';
      if(lex_match_char(state, '=')) {
        state->last_token.kind = TOKEN_cmp_geq;
      }
    }
    else if(lex_match_char(state, '<')) {
      state->last_token.kind = '<';
      if(lex_match_char(state, '=')) {
        state->last_token.kind = TOKEN_cmp_leq;
      }
      if(lex_match_char(state, '-')) {
        state->last_token.kind = TOKEN_left_arrow;
      }
    }
    else if(lex_match_char(state, '+')) {
      state->last_token.kind = '+';
      if(lex_match_char(state, '=')) {
        state->last_token.kind = TOKEN_add_ass;
      }
    }
    else if(lex_match_char(state, '-')) {
      state->last_token.kind = '-';
      if(lex_match_char(state, '=')) {
        state->last_token.kind = TOKEN_sub_ass;
      }
    }
    else if(lex_match_char(state, '*')) {
      state->last_token.kind = '*';
      if(lex_match_char(state, '=')) {
        state->last_token.kind = TOKEN_mul_ass;
      }
    }
    else if(lex_match_char(state, '/')) {
      state->last_token.kind = '/';
      if(lex_match_char(state, '=')) {
        state->last_token.kind = TOKEN_div_ass;
      }
    }
    else if(*state->stream == 0) {
      state->last_token.kind = TOKEN_eof;
    }
    else {
      state->last_token.kind = *state->stream;
      lex_next_char(state);
    }
  }
  char const *end = state->stream;
  state->last_token.start = start;
  state->last_token.end = end;
  state->last_token.loc = state->loc;
}



static char const *get_token_kind_name(t_token_kind kind) {
  if(kind == TOKEN_int) {return "INT";}
  else if(kind == TOKEN_idn) {return "NAME";}
  else if(kind == TOKEN_flt) {return "FLOAT";}
  else if(kind == TOKEN_str) {return "STRING";}
  else if(kind == '<') {return "<";}
  else if(kind == '>') {return ">";}
  else if(kind == '=') {return "=";}
  else if(kind == '!') {return "!";}
  else if(kind == '-') {return "-";}
  else if(kind == '+') {return "+";}
  else if(kind == '*') {return "*";}
  else if(kind == '/') {return "/";}
  else if(kind == '@') {return "@";}
  else if(kind == '$') {return "$";}
  else if(kind == '#') {return "#";}
  else if(kind == '%') {return "%";}
  else if(kind == '^') {return "^";}
  else if(kind == '&') {return "&";}
  else if(kind == '|') {return "|";}
  else if(kind == ':') {return ":";}
  else if(kind == ';') {return ";";}
  else if(kind == ',') {return ",";}
  else if(kind == '?') {return "?";}
  else if(kind == '(') {return "(";}
  else if(kind == ')') {return ")";}
  else if(kind == '[') {return "[";}
  else if(kind == ']') {return "]";}
  else if(kind == '{') {return "{";}
  else if(kind == '}') {return "}";}
  else if(kind == TOKEN_cmp_neq) {return "!=";}
  else if(kind == TOKEN_cmp_eq) {return "==";}
  else if(kind == TOKEN_cmp_leq) {return "<=";}
  else if(kind == TOKEN_cmp_geq) {return ">=";}
  else if(kind == TOKEN_left_arrow) {return "<-";}
  else if(kind == 0) {return "EOF";}
  return "{unknown token}";
}

static char const *get_token_string(t_token *token) {
  t_token_kind kind = token->kind;
  if(kind == TOKEN_idn) {
    return token->str_value->str;
  }
  return get_token_kind_name(kind);
}

static void print_token(t_token *token) {
  printf("%s", get_token_string(token));
}



static inline bool token_is_kind(t_lexstate *state, t_token_kind kind) {
  return state->last_token.kind == kind;
}

static inline bool token_match_kind(t_lexstate *state, t_token_kind kind) {
  if(state->last_token.kind == kind) {
    lex_next_token(state);
    return true;
  }
  return false;
}

static inline bool token_expect_kind(t_lexstate *state, t_token_kind kind) {
  if(state->last_token.kind == kind) {
    lex_next_token(state);
    return true;
  }
  push_errorf("%s(%d, %d): expected token %s, got %s",
              state->filename,
              state->loc.line, state->loc.offset,
              get_token_kind_name(kind),
              get_token_string(&state->last_token));
  return false;
}

static inline bool token_is_identifier(t_lexstate *state, t_intern const *str) {
  if(state->last_token.kind == TOKEN_idn) {
    return state->last_token.str_value == str;
  }
  return false;
}

static inline bool token_match_identifier(t_lexstate *state, t_intern const *str) {
  if(state->last_token.kind == TOKEN_idn && state->last_token.str_value == str) {
    lex_next_token(state);
    return true;
  }
  return false;
}

static inline bool token_expect_identifier(t_lexstate *state, t_intern const *str) {
  if(state->last_token.kind == TOKEN_idn && state->last_token.str_value == str) {
    lex_next_token(state);
    return true;
  }
  push_errorf("expected keyword %s, got %s", state->last_token.str_value->str, str->str);
  return false;
}

