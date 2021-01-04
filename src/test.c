
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

static u64 test_parse_expression(char const *expression) {
  t_lexstate state;
  state_init(&state, expression);
  state_parse_next_token(&state);
  u64 result = parse_expr(&state);
  return result;
}

#define test(e) assert((e) == test_parse_expression(#e))
static void test_parsing(void) {
  test(1);
  test((1));
  test(-1);
  test(2+(2*2));
  test(2*-2+2);
  test_parse_expression("2a");
  test_parse_expression("a");
}
#undef test

static void test_lexing(void) {
  char const *string = "+()abndk*18888__as2(2)*&";
  t_lexstate state;
  state_init(&state, string);
  do {
    state_parse_next_token(&state);
    token_print(state.last_token);
  } while(state.last_token.kind != TOKEN_EOF);
  printf("\n\n");
}
