
// TOKENS

struct {
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

enum {
  TYPE_none,
  TYPE_alias,
  TYPE_pointer,
  TYPE_slice,
  TYPE_function,
} typedef t_type_cat;

struct t_type_node_ {
  t_type_cat cat;
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

static t_intern const *keyword_if;
static t_intern const *keyword_else;
static t_intern const *keyword_while;

static t_intern const *keyword_break;
static t_intern const *keyword_continue;
static t_intern const *keyword_return;
static t_intern const *keyword_print;

static t_intern const *keyword_and;
static t_intern const *keyword_or;

static t_intern const *keyword_bool;;
static t_intern const *keyword_byte;
static t_intern const *keyword_int;
static t_intern const *keyword_float;
static t_intern const *keyword_string;

static t_intern const *main_name;

static t_ast_node *type_float;
static t_ast_node *type_string;
static t_ast_node *type_bool;
static t_ast_node *type_int;
static t_ast_node *type_byte;
