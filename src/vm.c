
struct {
  ptr stack_size;
  byte *stack;
  byte *stack_ptr;
  i64 reg;
  bool halt;
  bool error;
} typedef t_vm;

static void vm_init(t_vm *vm, ptr stack_size, void *stack) {
  vm->stack_size = stack_size;
  vm->stack = stack;
  vm->stack_ptr = vm->stack + vm->stack_size;
  vm->halt = false;
  vm->error = false;
  vm->reg = false;
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
// 05 ADD  REG,IMM8 (in bytes)
// 06 ADD  REG,STK8
// 07 SUB  REG,IMM8
// 08 SUB  REG,STK8
// 09 MUL  REG,IMM8
// 0A MUL  REG,STK8
// 0B DIV  REG,IMM8
// 0C DIV  REG,STK8
// 0D NEG  REG
// 0E CLR  REG
//
// 10 PUSH REG
// 11 POP  REG
// 12 MOV  REG,IMM8
static void vm_next_instruction(t_vm *vm, t_bytestream *stream) {
  if(!vm->halt) {
    byte instruction = *stream->p;
    stream->p += 1;
    switch(instruction) {
      case 0x00: {} break;
      case 0x01: {
        vm->halt = true;
      } break;
      case 0x02: {
        vm->error = true;
        vm->halt = true;
      } break;
      
      case 0x05: {
        i64 imm = *(i64*)stream->p;
        stream->p += sizeof imm;
        vm->reg += imm;
      } break;
      case 0x06: {
        vm->reg += vm_pop_i64(vm);
      } break;
      case 0x07: {
        i64 imm = *(i64*)stream->p;
        stream->p += sizeof imm;
        vm->reg -= imm;
      } break;
      case 0x08: {
        vm->reg -= vm_pop_i64(vm);
      } break;
      case 0x09: {
        i64 imm = *(i64*)stream->p;
        stream->p += sizeof imm;
        vm->reg *= imm;
      } break;
      case 0x0A: {
        vm->reg *= vm_pop_i64(vm);
      } break;
      case 0x0B: {
        i64 imm = *(i64*)stream->p;
        stream->p += sizeof imm;
        vm->reg /= imm;
      } break;
      case 0x0C: {
        vm->reg /= vm_pop_i64(vm);
      } break;
      case 0x0D: {
        vm->reg = -vm->reg;
      } break;
      case 0x0E: {
        vm->reg = 0;
      } break;
      
      case 0x10: {
        vm_push_i64(vm, vm->reg);
      } break;
      case 0x11: {
        vm->reg = vm_pop_i64(vm);
      } break;
      case 0x12: {
        i64 imm = *(i64*)stream->p;
        stream->p += sizeof imm;
        vm->reg = imm;
      } break;
    }
  }
}
