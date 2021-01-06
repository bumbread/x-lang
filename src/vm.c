
struct {
  ptr stack_size;
  byte *stack;
  byte *stack_ptr;
  bool halt;
  bool error;
} typedef t_vm;

static void vm_init(t_vm *vm, ptr stack_size, void *stack) {
  vm->stack_size = stack_size;
  vm->stack = stack;
  vm->stack_ptr = vm->stack + vm->stack_size;
  vm->halt = false;
  vm->error = false;
}

static void vm_reset(t_vm *vm) {
  vm->stack_ptr = vm->stack + vm->stack_size;
  vm->halt = false;
  vm->error = false;
}

static void vm_push_i64(t_vm *vm, i64 value) {
  if(vm->error == false) {
    if(vm->stack_ptr >= vm->stack + sizeof value) {
      vm->stack_ptr -= sizeof value;
      *(i64*)vm->stack_ptr = value;
    }
    else  {
      vm->halt = true;
      vm->error = true;
    }
  }
}

static i64 vm_pop_i64(t_vm *vm) {
  if(vm->error == false) {
    if(vm->stack_ptr <= vm->stack + vm->stack_size - sizeof(i64)) {
      i64 result = *(i64*)vm->stack_ptr;
      vm->stack_ptr += sizeof result;
      return result;
    }
    else {
      vm->halt = true;
      vm->error = true;
    }
  }
  return 0;
}

struct {
  byte *p;
  byte *off;
  byte *end;
  bool error;
} typedef t_bytestream;

static void bytestream_init(t_bytestream *stream, ptr size, void *mem) {
  stream->p = mem;
  stream->end = stream->p + size;
  stream->off = stream->p;
  stream->error = false;
}

// intructions (codes in hex):
// 00 NOP
// 01 HALT
// 02 ERR
// 
// 05 ADD
// 06 SUB
// 07 MUL
// 08 DIV
// 09 NEG
//
// 10 LIT IMM64
static void vm_next_instruction(t_vm *vm, t_bytestream *stream) {
  if(!vm->halt) {
    byte instruction = *stream->p;
    stream->p += 1;
    switch(instruction) {
      case 0x00: {
        
      } break;
      case 0x01: {
        vm->halt = true;
      } break;
      case 0x02: {
        vm->error = true;
        vm->halt = true;
      } break;
      
      case 0x05: {
        i64 arg1 = vm_pop_i64(vm);
        i64 arg2 = vm_pop_i64(vm);
        i64 result = arg1 + arg2;
        vm_push_i64(vm, result);
      } break;
      case 0x06: {
        i64 arg1 = vm_pop_i64(vm);
        i64 arg2 = vm_pop_i64(vm);
        i64 result = arg1 - arg2;
        vm_push_i64(vm, result);
      } break;
      case 0x07: {
        i64 arg1 = vm_pop_i64(vm);
        i64 arg2 = vm_pop_i64(vm);
        i64 result = arg1 * arg2;
        vm_push_i64(vm, result);
      } break;
      case 0x08: {
        i64 arg1 = vm_pop_i64(vm);
        i64 arg2 = vm_pop_i64(vm);
        i64 result = arg1 / arg2;
        vm_push_i64(vm, result);
      } break;
      case 0x09: {
        i64 arg1 = vm_pop_i64(vm);
        i64 result = -arg1;
        vm_push_i64(vm, result);
      } break;
      case 0x10: {
        i64 imm = *(i64*)stream->p;
        stream->p += sizeof imm;
        vm_push_i64(vm, imm);
      } break;
    }
  }
}

static void vm_execute_code(t_vm *vm, byte *code) {
  t_bytestream stream;
  bytestream_init(&stream, 0, code);
  do {
    vm_next_instruction(vm, &stream);
  } while(!vm->halt);
}

static void bytestream_push_byte(t_bytestream *stream, byte val) {
  if(!stream->error) {
    if(stream->off + 1 > stream->end) {
      stream->error = true;
      return;
    }
    *stream->off = val;
    stream->off += 1;
  }
}

static void bytestream_push_i64(t_bytestream *stream, i64 val) {
  if(!stream->error) {
    if(stream->off + 8 > stream->end) {
      stream->error = true;
      return;
    }
    *(i64 *)stream->off = val;
    stream->off += 8;
  }
}

static void compile_expr0(t_lexstate *state, t_bytestream *code);

static void compile_expr3(t_lexstate *state, t_bytestream *code) {
  if(token_match(state, TOKEN_INT)) {
    bytestream_push_byte(code, 0x10); // LIT opcode
    bytestream_push_i64(code, state->last_token.int_value);
  }
  else if(token_match(state, '(')) {
    compile_expr0(state, code);
    if(false == token_match(state, ')')) {
      set_errorf("fatal: unmatched parenthesis");
    }
  }
  else {
    set_errorf("fatal: unexpected token %s, expected INT or '('", 
               get_nonchar_token_kind_name(state->last_token.kind));
  }
}

static void compile_expr2(t_lexstate *state, t_bytestream *code) {
  bool neg = false;
  if(token_match(state, '-')) neg = true;
  compile_expr3(state, code);
  if(neg) bytestream_push_byte(code, 0x09); // NEG opcode
}

static void compile_expr1(t_lexstate *state, t_bytestream *code) {
  compile_expr2(state, code);
  while(token_is(state, '*') || token_is(state, '/')) {
    char op = state->last_token.kind;
    state_parse_next_token(state);
    compile_expr2(state, code);
    if(op == '*') bytestream_push_byte(code, 0x07); // MUL opcode
    if(op == '/') bytestream_push_byte(code, 0x08); // DIV opcode
  }
}

static void compile_expr0(t_lexstate *state, t_bytestream *code) {
  compile_expr1(state, code);
  while(token_is(state, '+') || token_is(state, '-')) {
    char op = state->last_token.kind;
    state_parse_next_token(state);
    compile_expr1(state, code);
    if(op == '+') bytestream_push_byte(code, 0x05); // ADD opcode
    if(op == '-') bytestream_push_byte(code, 0x06); // SUB opcode
  }
}

static void compile_expr(t_lexstate *state, t_bytestream *code) {
  compile_expr0(state, code);
  if(state->last_token.kind != TOKEN_EOF) {
    set_errorf("not an expression!\n");
  }
  bytestream_push_byte(code, 0x01); // HLT opcode
}

byte *compile_for_vm(char *expression) {
  byte *result = malloc(1024);
  {
    t_lexstate state;
    state_init(&state, expression);
    state_parse_next_token(&state);
    t_bytestream stream;
    bytestream_init(&stream, 1024, result);
    compile_expr(&state, &stream);
  }
  return result;
}
