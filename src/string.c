#include<string.h>

#define string_buffer_size 16*mb
#define string_builder_cap 1024

struct {
    ptr len;
    char str[0];
} typedef t_intern;

struct t_string_buffer_ {
    struct t_string_buffer_ *next;
    t_arena arena;
    ptr string_count;
    byte start[0];
} typedef t_string_buffer;

static t_string_buffer *first_buffer;

static t_string_buffer *buffer_create_and_init(void) {
    t_string_buffer *buffer = x_malloc(string_buffer_size);
    arena_init(&buffer->arena, string_buffer_size, buffer);
    void *test = arena_alloc(&buffer->arena, sizeof(t_string_buffer), 1);
    assert(test == buffer);
    buffer->string_count = 0;
    buffer->next = null;
    return buffer;
}

static void init_interns() {
    first_buffer = buffer_create_and_init();
}

static bool cstrings_equal(char const *str1, char const *str2) {
    while(true) {
        if(*str1++ != *str2++) return false;
        if(*str1 == 0) return true;
    }
    assert(false);
    return true;
}

static inline bool interns_equal(t_intern const *i1, t_intern const *i2) {
    return i1->len == i2->len && i1->str == i2->str;
}

static void copy_string_to_intern(t_intern *intern, ptr str_len, char const *str) {
    intern->len = str_len;
    for(ptr i = 0; i < intern->len; i += 1) {
        intern->str[i] = str[i];
    }
    intern->str[intern->len] = 0;
}

static t_intern const *intern_string(char const *begin, char const *end) {
    ptr string_length = (ptr)(end - begin);
    assert(string_length < string_buffer_size);
    
    // search the string in the chain of buffers
    for(t_string_buffer *buffer = first_buffer; buffer != null; buffer = buffer->next) {
        byte *intern_ptr = buffer->start;
        for(ptr str_index = 0; str_index < buffer->string_count; str_index += 1) {
            t_intern const *intern = (t_intern const *)intern_ptr;
            if(intern->len == string_length) {
                bool equal = true;
                for(ptr i = 0; i < intern->len; i += 1) {
                    if(intern->str[i] != begin[i]) {
                        equal = false;
                        break;
                    }
                }
                
                if(equal) {
                    return intern;
                }
            }
            intern_ptr += sizeof(ptr) + intern->len + 1; // 0-terminator isn't included in the length
        }
    }
    
    // return wasn't returned: add a new one
    t_string_buffer *buffer = first_buffer;
    while(true) {
        t_intern *intern = arena_alloc(&buffer->arena, sizeof(ptr) + string_length + 1, 1);
        if(intern != null) {
            copy_string_to_intern(intern, string_length, begin);
            buffer->string_count += 1;
            return intern;
        }
        if(buffer->next == null) break;
        buffer = buffer->next;
    }
    
    // intern wasn't allocated, new buffer
    t_string_buffer *new_buffer = buffer_create_and_init();
    assert(buffer->next == null);
    buffer->next = new_buffer;
    t_intern *intern = arena_alloc(&new_buffer->arena, sizeof(ptr) + string_length + 1, 1);
    assert(intern != null);
    copy_string_to_intern(intern, string_length, begin);
    new_buffer->string_count += 1;
    return intern;
}

t_intern const *intern_cstring(char const *str) {
    char const *end = str;
    while(*end != 0) end += 1;
    return intern_string(str, end);
}

struct {
    ptr len;
    byte *string;
    ptr cap;
} typedef t_string_builder;

static t_string_builder string_builder;

static void string_builder_init(void) {
    string_builder.string = malloc(string_builder_cap);
    string_builder.cap = string_builder_cap;
}

static void *string_builder_start(void) {
    string_builder.len = 0;
    return string_builder.string;
}

static void string_builder_append_char(char c) {
    string_builder.string[string_builder.len] = c;
    string_builder.len += 1;
}

static void *string_builder_finish(void) {
    string_builder.string[string_builder.len] = 0;
    return string_builder.string;
}
