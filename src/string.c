#include<string.h>

struct {
  ptr len;
  char const *str;
} typedef t_intern_string;

static ptr intern_string_count = 0;
static ptr intern_string_cap = 0;
static t_intern_string *intern_strings = null;

static bool strings_equal(char *str1, char *str2) {
  while(true) {
    if(*str1++ != *str2++) return false;
    if(*str1 == 0) return true;
  }
  assert(false);
  return true;
}

static char *intern_cstring(char const *str) {
  for(ptr string_index = 0; string_index < intern_string_count; string_index += 1) {
    t_intern_string *interned_string = intern_strings + string_index;
    if(strings_equal(interned_string->str, str)) return interned_string->str;
  }
  // string not found, creating one
  if(1 + intern_string_count > intern_string_cap) {
    intern_string_cap = 2*intern_string_cap + 1;
    intern_strings = realloc(intern_strings, intern_string_cap);
  }
  ptr str_len = 0;
  {char *str1 = str; while(*str1++ != 0) str_len += 1;}
  char *str_copy = malloc(str_len + 1);
  strcpy(str_copy, str);
  
  t_intern_string interned_string = {
    .len = str_len,
    .str = str_copy,
  };
  intern_strings[intern_string_count] = interned_string;
  intern_string_count += 1;
}

static char *intern_string(char const *begin, char const *end) {
  ptr str_len = (ptr)(end - begin);
  for(ptr string_index = 0; string_index < intern_string_count; string_index += 1) {
    t_intern_string *interned_string = intern_strings + string_index;
    if(interned_string->len == str_len) {
      if(memcmp(interned_string->str, begin, str_len)) return interned_string;
    }
  }
  // string not found, creating one
  if(1 + intern_string_count > intern_string_cap) {
    intern_string_cap = 2*intern_string_cap + 1;
    intern_strings = realloc(intern_strings, intern_string_cap);
  }
  
  char *str_copy = malloc(str_len + 1);
  memcpy(str_copy, str, str_len);
  
  t_intern_string interned_string = {
    .len = str_len,
    .str = str_copy,
  };
  intern_strings[intern_string_count] = interned_string;
  intern_string_count += 1;
}
