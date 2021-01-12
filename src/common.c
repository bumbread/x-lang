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

static bool error = false;
static char last_error[1024];
void set_errorf(char const *message, ...) {
  va_list args;
  va_start(args, message);
  vsprintf(last_error, message, args);
  va_end(args);
  error = true;
}

void check_errors(void) {
  if(error == true) {
    printf("error: %s\n", last_error);
    error = false;
  }
}
