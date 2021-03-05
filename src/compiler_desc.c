// TOKENS

struct {
  char const *filename;
  u64 line;
  u64 offset;
} typedef t_location;

enum {
  TOKEN_eof = 0,   // end of file
  // All ascii characters are a separate token
  // 0..127
  
  TOKEN_int = 128, // integer numbers
  TOKEN_flt,
  TOKEN_str,
  TOKEN_idn,
  TOKEN_cmp_neq,
  TOKEN_cmp_eq,
  TOKEN_cmp_leq,
  TOKEN_cmp_geq,
  
  TOKEN_add_ass,
  TOKEN_sub_ass,
  TOKEN_mul_ass,
  TOKEN_div_ass,
  
  TOKEN_left_arrow,
} typedef t_token_kind;

enum {
  FLAG_none,
  FLAG_keyword,
} typedef t_token_flags;

struct {
  t_token_kind kind;
  t_token_flags flags;
  char const *start;
  char const *end;
  t_location loc;
  union {
    i64 int_value;
    f64 flt_value;
    t_intern const *str_value;
  };
} typedef t_token;

static char const *get_token_kind_name(t_token_kind kind) {
  if(kind == TOKEN_int) {return "INT";}
  else if(kind == TOKEN_idn) {return "NAME";}
  else if(kind == TOKEN_flt) {return "FLOAT";}
  else if(kind == TOKEN_str) {return "STRING";}
  else if(kind == '<') {return "<";}
  else if(kind == '>') {return ">";}
  else if(kind == '=') {return "=";}
  else if(kind == '!') {return "!";}
  else if(kind == '-') {return "-";}
  else if(kind == '+') {return "+";}
  else if(kind == '*') {return "*";}
  else if(kind == '/') {return "/";}
  else if(kind == '@') {return "@";}
  else if(kind == '$') {return "$";}
  else if(kind == '#') {return "#";}
  else if(kind == '%') {return "%";}
  else if(kind == '^') {return "^";}
  else if(kind == '&') {return "&";}
  else if(kind == '|') {return "|";}
  else if(kind == ':') {return ":";}
  else if(kind == ';') {return ";";}
  else if(kind == ',') {return ",";}
  else if(kind == '?') {return "?";}
  else if(kind == '(') {return "(";}
  else if(kind == ')') {return ")";}
  else if(kind == '[') {return "[";}
  else if(kind == ']') {return "]";}
  else if(kind == '{') {return "{";}
  else if(kind == '}') {return "}";}
  else if(kind == TOKEN_cmp_neq) {return "!=";}
  else if(kind == TOKEN_cmp_eq) {return "==";}
  else if(kind == TOKEN_cmp_leq) {return "<=";}
  else if(kind == TOKEN_cmp_geq) {return ">=";}
  else if(kind == TOKEN_left_arrow) {return "<-";}
  else if(kind == 0) {return "EOF";}
  return "{unknown token}";
}

static char const *get_token_string(t_token *token) {
  t_token_kind kind = token->kind;
  if(kind == TOKEN_idn) {
    return token->str_value->str;
  }
  return get_token_kind_name(kind);
}

static void print_token(t_token *token) {
  printf("%s", get_token_string(token));
}

// ABSTRACT SYNTAX TREES

struct t_ast_node_ typedef t_ast_node;
struct t_type_node_ typedef t_type_node;
struct t_value_node_ typedef t_value_node;
struct t_stmt_node_ typedef t_stmt_node;

enum {
  AST_wtf,
  AST_list_node,
  AST_expr_node,
  AST_type_node,
  AST_stmt_node,
} typedef t_ast_node_cat;

struct t_ast_list_link_ typedef t_ast_list_link;

struct {
  t_ast_list_link *first;
  t_ast_list_link *last;
} typedef t_ast_list;

struct t_ast_list_link_ {
  t_ast_list_link *next;
  t_ast_list_link *prev;
  t_ast_node *p;
};

struct t_ast_stack_link_ typedef t_ast_stack_link;

struct {
  t_ast_stack_link *first;
  t_ast_stack_link *last;
} typedef t_ast_stack_list;

struct t_ast_stack_link_ {
  t_ast_stack_link *next;
  t_ast_stack_link *prev;
  t_ast_node *p;
};


enum {
  TYPE_none,
  TYPE_alias,
  TYPE_pointer,
  TYPE_slice,
  TYPE_function,
} typedef t_type_cat;

enum {
  FLAG_is_lvalue=1
} typedef t_type_flags;

struct t_type_node_ {
  t_type_cat cat;
  t_type_flags flags;
  union {
    // aliases
    t_intern const *name;
    // pointer, slice
    t_ast_node *base_type;
    // functions
    struct {
      t_ast_node *parameters;
      t_ast_node *return_type;
    };
  };
};

enum {
  op_wtf,
  UNARY_FIRST_OPERATOR, // no touching
  UNARY_add,
  UNARY_sub,
  UNARY_deref,
  UNARY_addr,
  BINARY_FIRST_OPERATOR, // no touching
  BINARY_add,
  BINARY_sub,
  BINARY_mul,
  BINARY_div,
  BINARY_less,
  BINARY_greater,
  BINARY_leq,
  BINARY_geq,
  BINARY_eq,
  BINARY_neq,
  BINARY_and,
  BINARY_or,
  BINARY_FIRST_ASS, // no touching
  BINARY_ass,
  BINARY_add_ass,
  BINARY_sub_ass,
  BINARY_mul_ass,
  BINARY_div_ass,
  BINARY_LAST_ASS, // no touching
  BINARY_function_call,
  BINARY_subscript,
  TERNARY_FIRST_OPERATOR, // no touching
  TERNARY_slice,
  LAST_OPERATOR, // no touching
} typedef t_operator_cat;

enum {
  EXPR_wtf,
  EXPR_variable,
  EXPR_int_value,
  EXPR_float_value,
  EXPR_string_value,
  EXPR_unary_op,
  EXPR_binary_op,
  EXPR_ternary_op,
} typedef t_expr_cat;

typedef struct t_expr_node_ t_expr_node;
struct t_expr_node_ {
  t_expr_cat cat;
  t_ast_node *type;
  union {
    // operators
    struct {
      t_operator_cat op;
      t_ast_node *opr1;
      t_ast_node *opr2;
      t_ast_node *opr3;
    };
    // values
    i64 ivalue;
    f64 fvalue;
    t_intern const *svalue;
    // ... (TODO: array values &c.)
    
    // variable
    t_intern const *var_name;
  };
};

enum {
  STMT_wtf,
  STMT_assignment,
  STMT_if,
  STMT_while,
  STMT_return,
  STMT_break,
  STMT_continue,
  STMT_declaration,
  STMT_block,
  STMT_print, // temporary (? TODO)
} typedef t_ast_stmt_category;

struct t_stmt_node_ {
  t_ast_stmt_category cat;
  union {
    // assignment
    struct {
      // has to be an assignment operator
      t_operator_cat ass_op;
      t_ast_node *lvalue;
      t_ast_node *rvalue;
    };
    // declarations
    struct {
      t_intern const *decl_name;
      t_ast_node *decl_type;
      t_ast_node *decl_value;
    };
    // if
    struct {
      t_ast_node *if_condition;
      t_ast_node *if_true_block;
      t_ast_node *if_false_block;
    };
    // while
    struct {
      t_ast_node *while_condition;
      t_ast_node *while_block;
    };
    // return, print
    t_ast_node *stmt_value;
    // block
    t_ast_list statements;
  };
};

struct t_ast_node_ {
  t_ast_node_cat cat;
  union {
    t_expr_node expr;
    t_type_node type;
    t_stmt_node stmt;
    t_ast_list list;
  };
};

static t_ast_list all_procs;
static t_ast_node *parser_scope = null;

static t_intern const *main_name;

static t_ast_node *type_float;
static t_ast_node *type_string;
static t_ast_node *type_bool;
static t_ast_node *type_int;
static t_ast_node *type_byte;

static void init_primitive_type(t_ast_node *type, t_intern const *keyword) {
  type->cat = AST_type_node;
  type->type.cat = TYPE_alias;
  type->type.name = keyword;
}


#define KEYWORD_spec \
KEYWORD_param(keyword_if,       "if")\
KEYWORD_param(keyword_else,     "else")\
KEYWORD_param(keyword_while,    "while")\
KEYWORD_param(keyword_break,    "break")\
KEYWORD_param(keyword_continue, "continue")\
KEYWORD_param(keyword_return,   "return")\
KEYWORD_param(keyword_print,    "print")\
KEYWORD_param(keyword_and,      "and")\
KEYWORD_param(keyword_or,       "or")\
KEYWORD_param(keyword_bool,     "bool")\
KEYWORD_param(keyword_byte,     "byte")\
KEYWORD_param(keyword_int,      "int")\
KEYWORD_param(keyword_float,    "float")\
KEYWORD_param(keyword_string,   "string")

// array of all keywords for looping
static t_intern const *keywords[] = {
#define KEYWORD_param(var, str) 0,
  KEYWORD_spec
#undef KEYWORD_param
};
static u64 keywords_num = 0;

// static keyword definitions
#define KEYWORD_param(var, str) static t_intern const *var;
KEYWORD_spec
#undef KEYWORD_param


static t_ast_node *alloc_node(void) {
  t_ast_node *result = global_alloc(sizeof(t_ast_node));
  memset(result, 0, sizeof(t_ast_node));
  return result;
}

static void init_compiler(void) {
  // intern keywords
#define KEYWORD_param(var, str) var = intern_cstring(str);
  KEYWORD_spec
#undef KEYWORD_param
  // fill the keyword array with interns
#define KEYWORD_param(var,str) keywords[keywords_num++] = var;
  KEYWORD_spec
#undef KEYWORD_param
  
  keyword_string   = intern_cstring("main");
  
  type_int = alloc_node();
  type_bool = alloc_node();
  type_byte = alloc_node();
  type_float = alloc_node();
  type_string = alloc_node();
  
  init_primitive_type(type_int, keyword_int);
  init_primitive_type(type_bool, keyword_bool);
  init_primitive_type(type_byte, keyword_byte);
  init_primitive_type(type_float, keyword_float);
  init_primitive_type(type_string, keyword_string);
  
  all_procs.first = null;
  all_procs.last = null;
}
