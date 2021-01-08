
static void token_print(t_token token) {
  if(token.kind < 128 && isprint(token.kind)) {
    printf(" `%c` ", token.kind);
  }
  else if(token.kind == TOKEN_INT) {
    printf(" %lld ", token.int_value);
  }
  else if(token.kind == TOKEN_IDN) {
    printf(" [%.*s] ", (int)(token.end - token.start), token.start);
  }
  else {
    char *token_name = get_nonchar_token_kind_name(token.kind);
    printf(" `%s` ", token_name);
  }
}

static void test_lexing(void) {
  //char const *string = "+(!=+>>&& == 2)abndk*<<<18888__as2(2)*&";
  char const *string = ">> << >>= <<= >>> <<< >>>= <<<=";
  t_lexstate state;
  state_init(&state, string);
  do {
    state_parse_next_token(&state);
    token_print(state.last_token);
  } while(state.last_token.kind != TOKEN_EOF);
  printf("\n\n");
}

static i64 test_parse_expression(char const *expression) {
  t_lexstate state;
  state_init(&state, expression);
  state_parse_next_token(&state);
  i64 result = parse_expr(&state);
  check_errors();
  return result;
}

#define test(e) assert((e) == test_parse_expression(#e))
static void test_parsing(void) {
  test(1);
  test((1));
  test(-1);
  test(2+(2*2));
  test(2*-2+2);
  //test_parse_expression("2a");
  //test_parse_expression("a");
  check_errors();
}
#undef test

static void test_vm(void) {
  ptr size = 10*kb;
  t_vm vm;
  vm_init(&vm, size, malloc(size));
  
  byte code[] = {
    0x10, 0x69,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // LIT 69
    0x10, 0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // LIT 3
    0x05,                                          // ADD 
    0x10, 0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // LIT 5
    0x07,                                          // MUL
    0x10, 0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // LIT 7
    0x08,                                          // DIV
    0x00,                                          // HLT
  };
  vm_execute_code(&vm, sizeof code, code);
  check_errors();
}

static i64 test_vm_compile_code(t_vm *vm, char *expr) {
  vm_reset(vm);
  ptr bytecode_size;
  byte *bytecode = compile_for_vm(expr, &bytecode_size);
  vm_execute_code(vm, bytecode_size, bytecode);
  i64 result = vm_pop_i64(vm);
  check_errors();
  return result;
}

#define test(a) assert(test_vm_compile_code(&vm, #a) == (a))
static void test_vm_compiler(void) {
  ptr size = 10*kb;
  t_vm vm;
  vm_init(&vm, size, malloc(size));
  
  test(2+2*2);
  test(2+3);
  test(1+1);
  test(2+(2*2));
  test((1));
  test(-2);
  test(2*-1);
}
#undef test
