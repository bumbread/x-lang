
static void token_print(t_token token) {
  if(token.kind < 128 && isprint(token.kind)) {
    printf("'%c'", token.kind);
  }
  else if(token.kind == TOKEN_INT) {
    printf("%lld", token.int_value);
  }
  else if(token.kind == TOKEN_IDN) {
    printf("[%.*s]", (int)(token.end - token.start), token.start);
  }
  else if(token.kind == TOKEN_OP_LOG_OR) {printf("||");}
  else if(token.kind == TOKEN_OP_LOG_AND) {printf("&&");}
  else if(token.kind == TOKEN_OP_REL_EQ) {printf("==");}
  else if(token.kind == TOKEN_OP_REL_NEQ) {printf("!=");}
  else if(token.kind == TOKEN_OP_REL_GEQ) {printf(">=");}
  else if(token.kind == TOKEN_OP_REL_LEQ) {printf("<=");}
  else if(token.kind == TOKEN_OP_LOG_SHIFTL) {printf("<<");}
  else if(token.kind == TOKEN_OP_LOG_SHIFTR) {printf(">>");}
  else if(token.kind == TOKEN_OP_ARITHM_SHIFTL) {printf("<<<");}
  else if(token.kind == TOKEN_OP_ARITHM_SHIFTR) {printf(">>>");}
  else if(token.kind == TOKEN_OP_BIG_ARROW) {printf("=>");}
  else if(token.kind == TOKEN_OP_ARROW) {printf("->");}
  else if(token.kind == TOKEN_EOF) {printf("{EOF}");}
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
  char const *string = "+(!=+>>&& == 2)abndk*<<<18888__as2(2)*&";
  t_lexstate state;
  state_init(&state, string);
  do {
    state_parse_next_token(&state);
    token_print(state.last_token);
  } while(state.last_token.kind != TOKEN_EOF);
  printf("\n\n");
}

static void test_vm(void) {
  ptr size = 10*kb;
  t_vm vm;
  vm_init(&vm, size, malloc(size));
  
  byte code[] = {
    0x00, // nop
    0x12, 0x69,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // mov reg,69
    0x0e, // clr reg
    0x05, 0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // add reg,5
    0x09, 0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // mul reg,3
    0x07, 0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // sub reg,3
    0x0B, 0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // div reg,4
    0x10, // push reg
    0x06, // add reg,stk
    0x10, // push reg
    0x0A, // mul reg,stk
    0x01, // halt
  };
  t_bytestream stream;
  bytestream_init(&stream, code);
  do {
    vm_next_instruction(&vm, &stream);
  } while(!vm.halt);
}
