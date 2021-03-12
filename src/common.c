int8_t   typedef i8;
uint8_t  typedef u8;
int16_t  typedef i16;
uint16_t typedef u16;
int32_t  typedef i32;
uint32_t typedef u32;
int64_t  typedef i64;
uint64_t typedef u64;
float    typedef f32;
double   typedef f64;
u8       typedef byte;
u16      typedef word;
u32      typedef bool;
u64      typedef ptr;
#define  true    ((bool)1)
#define  false   ((bool)0)
#define  null    ((void *)0)
#define  kb      1024
#define  mb      1024*kb
#define  gb      1024*mb

#define array_count(arr) (sizeof(arr)/sizeof((arr)[0]))

struct {
    char const *filename;
    i64 line;
    i64 offset;
} typedef t_location;

struct {
    t_location loc;
    char *msg;
} typedef t_error;

void panicf(char *message, ...) {
    va_list args;
    va_start(args, message);
    vprintf(message, args);
    va_end(args);
    exit(1);
}

static int max_errors = -1;
static int now_errors = 0;
static t_error *errors;

void init_error_buffer(int set_max_errors) {
    max_errors = set_max_errors;
    errors = malloc(set_max_errors * sizeof(t_error));
    for(i64 i = 0; i < max_errors; i += 1) {
        errors[i].msg = malloc(sizeof(char)*1024);
    }
}

void print_error_buffer(void) {
    for(i64 error_index = 0; error_index < now_errors; error_index += 1) {
        t_error err = errors[error_index];
        printf("%s(%lld,%lld): %s\n", err.loc.filename, 
               err.loc.line, err.loc.offset, err.msg);
    }
}

void push_errorf(t_location loc, char const *message, ...) {
    if(now_errors < max_errors) {
        errors[now_errors].loc = loc;
        va_list args;
        va_start(args, message);
        vsprintf(errors[now_errors].msg, message, args);
        va_end(args);
        now_errors += 1;
    }
    else {
        print_error_buffer();
        exit(1);
    }
}

static void print_level(int level) {
    for(int i = 0; i < level; i += 1) {
        printf("  ");
    }
}

static u64 char_to_digit[256] = {
    ['0'] = 0,
    ['1'] = 1,
    ['2'] = 2,
    ['3'] = 3,
    ['4'] = 4,
    ['5'] = 5,
    ['6'] = 6,
    ['7'] = 7,
    ['8'] = 8,
    ['9'] = 9,
    ['a'] = 10,['A'] = 10,
    ['b'] = 11,['B'] = 11,
    ['c'] = 12,['C'] = 12,
    ['d'] = 13,['D'] = 13,
    ['e'] = 14,['E'] = 14,
    ['f'] = 15,['F'] = 15,
};

static u64 escape_char[256] = {
    ['n'] = '\n',
    ['t'] = '\t',
    ['r'] = '\r',
    ['a'] = '\a',
    ['b'] = '\b',
    ['v'] = '\v',
    ['\\'] = '\\',
    ['\''] = '\'',
    ['"'] = '"',
};

static bool is_digit_in_base(u64 base, char c) {
    u64 digit = char_to_digit[c];
    return (digit < base) && (digit != 0 || c == '0');
}
