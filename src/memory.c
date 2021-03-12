
#define default_alignment 8
#define default_buffer_size 10*mb

static ptr ptr_check_is_power_of_two(ptr p) {
    return (p & (p-1)) == 0;
}

static ptr ptr_align(ptr p, ptr align) {
    assert(ptr_check_is_power_of_two(align));
    ptr mod = p & (align - 1);
    if(mod) {
        return p + (align - mod);
    }
    return p;
}

static ptr ptr_get_alignment_offset(ptr p, ptr align) {
    assert(ptr_check_is_power_of_two(align));
    ptr mod = p & (align - 1);
    if(mod) {
        return align - mod;
    }
    return 0;
}

static void *x_malloc(ptr size) {
    void *result = malloc(size);
    //printf("allocating %llu bytes, returning address at %p\n", size, result);
    return result;
}

struct {
    ptr buffer_size;
    byte *buffer;
    ptr alloc_size;
    ptr buffer_end;
} typedef t_arena;

struct {
    t_arena current;
} typedef t_global_allocator;

static t_global_allocator global_allocator;

static void arena_init(t_arena *arena, ptr size, void *buffer) {
    arena->buffer_size = default_buffer_size;
    arena->buffer = buffer;
    arena->alloc_size = 0;
    arena->buffer_end = (ptr)buffer + default_buffer_size;
}

static void initialize_global_allocator(void) {
    arena_init(&global_allocator.current, default_buffer_size, x_malloc(default_buffer_size));
}

static void request_another_buffer(void) {
    initialize_global_allocator();
}

static void *arena_alloc(t_arena *arena, ptr size, ptr alignment) {
    ptr alloc_start = (ptr)arena->buffer + arena->alloc_size;
    alloc_start = ptr_align(alloc_start, alignment);
    if(alloc_start + size <= arena->buffer_end) {
        arena->alloc_size += size;
        return (void *)alloc_start;
    }
    return null;
}

static void *global_alloc(ptr size) {
    void *result = arena_alloc(&global_allocator.current, size, default_alignment);
    if(null == result) {
        // not fit, request another buffer, try again
        assert(size < default_buffer_size);
        request_another_buffer();
        result = arena_alloc(&global_allocator.current, size, default_alignment);
    }
    return result;
}

// TODO(bumbread): make stack as linked list of megabuffers
struct {
    ptr buffer_size;
    byte *buffer;
    ptr element_size;
    ptr elements_count;
    ptr max_alloc_count;
} typedef t_stack;

static void stack_init(t_stack *stack, ptr buffer_size, void *buffer, ptr element_size) {
    ptr offset_buffer = ptr_get_alignment_offset((ptr)buffer, default_alignment);
    assert(buffer_size > offset_buffer);
    stack->buffer_size = buffer_size - offset_buffer;
    stack->buffer = (byte *)buffer + offset_buffer;
    stack->element_size = ptr_align(element_size, default_alignment);
    
    stack->elements_count = 0;
    stack->max_alloc_count = stack->buffer_size / stack->element_size;
}

static void *stack_push(t_stack *stack) {
    if(stack->elements_count >= stack->max_alloc_count) {
        return null;
    }
    void *result = stack->buffer + stack->elements_count*stack->element_size;
    stack->elements_count += 1;
    return result;
}

static void *stack_pop(t_stack *stack) {
    assert(stack->elements_count != 0);
    stack->elements_count -= 1;
    return stack->buffer + stack->elements_count*stack->element_size;
}
