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

void panicf(char *message, ...) {
    va_list args;
    va_start(args, message);
    vprintf(message, args);
    va_end(args);
    exit(1);
}

static int max_errors = -1;
static int now_errors = 0;
static char (*errors)[1024];

void init_errors(int set_max_errors) {
    max_errors = set_max_errors;
    errors = malloc(set_max_errors * sizeof(char[1024]));
}

void push_errorf(char const *message, ...) {
    if(now_errors < max_errors) {
        va_list args;
        va_start(args, message);
        vsprintf(errors[now_errors], message, args);
        va_end(args);
        now_errors += 1;
    }
    else exit(1);
}

void check_errors(void) {
    for(int error_index = 0; error_index < now_errors; error_index += 1) {
        printf("%s\n", errors[error_index]);
    }
}
