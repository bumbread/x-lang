
#define DEFAULT_ALIGNMENT 4

ptr ptr_check_is_power_of_two(ptr p) {
  return (p & (p-1)) == 0;
}

ptr ptr_align(ptr p, ptr align) {
  assert(ptr_check_is_power_of_two(align));
  ptr mod = p & (align - 1);
  if(mod) {
    return p + (align - mod);
  }
  return p;
}

ptr ptr_get_alignment_offset(ptr p, ptr align) {
  assert(ptr_check_is_power_of_two(align));
  ptr mod = p & (align - 1);
  if(mod) {
    return align - mod;
  }
  return 0;
}

// linear allocator
struct {
  byte *buffer;
  ptr buffer_size;
  ptr size; // allocation size
  // for resizing
  ptr last_alloc_size;
  byte *last_alloc_ptr;
} typedef t_arena;

static void arena_init(t_arena *arena, ptr buffer_size, void *buffer) {
  arena->buffer = buffer;
  arena->buffer_size = buffer_size;
  arena->size = 0;
  arena->last_alloc_ptr = null;
  arena->last_alloc_size = 0;
}

static void *arena_alloc(t_arena *arena, ptr size, ptr align) {
  ptr offset = ptr_get_alignment_offset((ptr)(arena->buffer + arena->size), align);
  if(arena->size + offset + size > arena->buffer_size) {
    return null;
  }
  arena->last_alloc_ptr = arena->buffer + arena->size + offset;
  arena->size += offset + size;
  arena->last_alloc_size = size;
  return arena->last_alloc_ptr;
}

static void *arena_realloc_last(t_arena *arena, ptr new_size, ptr align) {
  if(arena->last_alloc_ptr == null) {
    return arena_alloc(arena, new_size, align);
  }
  if(new_size > arena->last_alloc_size) {
    if(arena->size + new_size - arena->last_alloc_size > arena->buffer_size) {
      return null;
    }
  }
  arena->last_alloc_size = new_size;
  return arena->last_alloc_ptr;
}

static void *arena_resize_last(t_arena *arena, ptr new_size) {
  assert(arena->last_alloc_ptr != null);
  if(new_size > arena->last_alloc_size) {
    if(arena->size + new_size - arena->last_alloc_size > arena->buffer_size) {
      return null;
    }
  }
  arena->last_alloc_size = new_size;
  return arena->last_alloc_ptr;
}

static void arena_flush(t_arena *arena) {
  arena->last_alloc_ptr = null;
  arena->last_alloc_size = 0;
  arena->size = 0;
}

// stack of fixed-size elements
struct {
  byte *buffer;
  ptr buffer_size;
  ptr alloc_size;
  ptr element_size;
  ptr align;
} typedef t_stack;

static void stack_init(t_stack *stack, ptr buffer_size, void *buffer, ptr element_size, ptr align) {
  stack->buffer = buffer;
  stack->buffer_size = buffer_size;
  ptr offset = ptr_get_alignment_offset((ptr)buffer, align);
  stack->buffer += offset;
  stack->buffer_size -= offset;
  stack->element_size = ptr_align(element_size, align);
  stack->align = align;
  stack->alloc_size = 0;
}

static void *stack_push(t_stack *stack) {
  if(stack->alloc_size + stack->element_size > stack->buffer_size) {
    return null;
  }
  void *result = stack->buffer + stack->alloc_size;
  stack->alloc_size += stack->element_size;
  return result;
}

static void *stack_pop(t_stack *stack) {
  if(stack->alloc_size < stack->element_size) {
    return null;
  }
  stack->alloc_size -= stack->element_size;
  return stack->buffer + stack->alloc_size;
}

static void stack_free_all(t_stack *stack) {
  stack->alloc_size = ptr_get_alignment_offset((ptr)stack->buffer, stack->align);
}
