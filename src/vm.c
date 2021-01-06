
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
} typedef t_bytestream;

static void bytestream_init(t_bytestream *stream, void *mem) {
  stream->p = mem;
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
