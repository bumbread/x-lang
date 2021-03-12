// TOKENS

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
    t_token_location loc;
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

enum {
    UNARY_FIRST,    // no touch
    UNARY_add,
    UNARY_sub,
    UNARY_addr,
    UNARY_deref,
    BINARY_FIRST,   // no touch
    BINARY_ass,
    BINARY_add,
    BINARY_sub,
    BINARY_mul,
    BINARY_div,
    ASS_FIRST,
    BINARY_add_ass,
    ARR_ASS_FIRST,
    BINARY_sub_ass,
    BINARY_div_ass,
    BINARY_mul_ass,
    ASS_LAST,
    BINARY_and,
    BINARY_or,
    BINARY_less,
    BINARY_greater,
    BINARY_leq,
    BINARY_geq,
    BINARY_eq,
    BINARY_neq,
    BINARY_function_call,
    BINARY_subscript,
    TERNARY_FIRST,
    TERNARY_slice,
    OPERATOR_LAST,
} typedef f_operation_cat;

enum {
    STMT_invalid,
    STMT_if,
    STMT_while,
    STMT_break,
    STMT_continue,
    STMT_return,
    STMT_print,
    STMT_decl,
    STMT_expr,
    STMT_block,
} typedef f_stmt_cat;

enum {
    EXPR_invlaid,
    EXPR_value,
    EXPR_variable,
    EXPR_unary,
    EXPR_binary,
    EXPR_ternary,
    EXPR_function_call,
} typedef f_expr_cat;

enum {
    EXPR_static=1,
    EXPR_lvalue=2,
    EXPR_side_effects=4,
} typedef f_expr_flags;

enum {
    VALUE_invalid,
    VALUE_int,
    VALUE_string,
    VALUE_float,
    // TODO(bumbread): for later
    //VALUE_slice_const,
    //VALUE_compound_literal,
} typedef f_value_cat;

enum {
    TYPE_invlaid,
    TYPE_int,
    TYPE_float,
    TYPE_string,
    TYPE_bool,
    TYPE_byte,
    TYPE_pointer,
    TYPE_slice,
    TYPE_function,
    // TODO(bumbread): for when I'll implement type decls.
    // TYPE_alias,
} typedef f_type_cat;

enum {
    // TODO(bumbread): for when I'll implement type decls
    //TYPE_distinct=1,
    //TYPE_const=2,
} typedef f_type_flags;

enum {
    DECL_no_value,
    DECL_expr_value,
    DECL_block_value,
} typedef f_decl_value_cat;

// ABSTRACT SYNTAX TREES
typedef struct t_expr_data_ t_expr_data;
typedef struct t_stmt_data_ t_stmt_data;
typedef struct t_type_data_ t_type_data;
typedef struct t_decl_data_ t_decl_data;

// lists declarations

typedef struct t_expr_list_node_ t_expr_list_node;
struct t_expr_list_node_ {
    t_expr_list_node *next;
    t_expr_list_node *prev;
    t_expr_data *data;
};

struct {
    t_expr_list_node *first;
    t_expr_list_node *last;
    i64 count;
} typedef t_expr_list;

typedef struct t_decl_list_node_ t_decl_list_node;
struct t_decl_list_node_ {
    t_decl_list_node *next;
    t_decl_list_node *prev;
    t_decl_data *data;
};

struct {
    t_decl_list_node *first;
    t_decl_list_node *last;
    i64 count;
} typedef t_decl_list;

typedef struct t_stmt_list_node_ t_stmt_list_node;
struct t_stmt_list_node_ {
    t_stmt_list_node *next;
    t_stmt_list_node *prev;
    t_stmt_data *data;
};

struct {
    t_stmt_list_node *first;
    t_stmt_list_node *last;
    i64 count;
} typedef t_stmt_list;

// expr data

struct {
    f_value_cat cat;
    union {
        i64 i;
        f64 f;
        t_intern const *s;
    };
} typedef t_value_data;

struct {
    f_operation_cat cat;
    t_expr_data *expr1;
    t_expr_data *expr2;
    t_expr_data *expr3;
} typedef t_operation_data;

struct {
    t_expr_data *callee;
    t_expr_list *parameters;
} typedef t_function_data;

// type data

// TODO(bumbread): should the parameter be stored as value?
struct {
    t_type_data *return_type;
    t_decl_list *parameters;
} typedef t_function_type_data;

// stmt data

struct {
    t_expr_data *condition;
    t_stmt_data *true_branch;
    t_stmt_data *false_branch;
} typedef t_if_data;

struct {
    t_expr_data *condition;
    t_stmt_data *block;
} typedef t_while_data;

struct t_decl_data_ {
    f_decl_value_cat cat;
    t_intern const *name;
    t_type_data *type;
    union {
        t_expr_data *value;
        t_stmt_list *block_data;
    };
};

// NODES.

struct t_type_data_ {
    f_type_cat cat;
    f_type_flags flags;
    union {
        t_type_data *pointer_base;
        t_type_data *slice_base;
        t_function_type_data func;
    };
};

struct t_expr_data_ {
    f_expr_cat cat;
    f_expr_flags flags;
    t_type_data *type;
    t_token_location loc;
    union {
        t_value_data value;
        t_operation_data operation;
        t_function_data func;
        t_intern const *var_name;
    };
};

struct t_stmt_data_ {
    f_stmt_cat cat;
    t_token_location loc;
    union {
        t_while_data while_data;
        t_if_data if_data;
        t_decl_data *decl_data;
        t_expr_data *return_expr;
        t_expr_data *print_expr;
        t_expr_data *expr;
        t_stmt_list block_data;
    };
};

static t_decl_list all_procs;

static t_intern const *main_name;
static t_intern const *result_name;
static t_intern const *empty_string;

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
KEYWORD_param(keyword_string,   "string")\
KEYWORD_param(keyword_true,      "true")\
KEYWORD_param(keyword_false,    "false")

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


static void init_compiler(void) {
    // intern keywords
#define KEYWORD_param(var, str) var = intern_cstring(str);
    KEYWORD_spec
#undef KEYWORD_param
    // fill the keyword array with interns
#define KEYWORD_param(var,str) keywords[keywords_num++] = var;
    KEYWORD_spec
#undef KEYWORD_param
    
    main_name = intern_cstring("main");
    result_name = intern_cstring("result");
    empty_string = intern_cstring("");
    
    all_procs.first = null;
    all_procs.last = null;
}

static char const *get_operator_string(f_operation_cat cat) {
    switch(cat) {
        case UNARY_add: return "+";
        case UNARY_sub: return "-";
        case UNARY_addr: return "$";
        case UNARY_deref: return "@";
        case BINARY_ass: return "=";
        case BINARY_add: return "+";
        case BINARY_sub: return "-";
        case BINARY_mul: return "*";
        case BINARY_div: return "/";
        case BINARY_add_ass: return "+=";
        case BINARY_sub_ass: return "-=";
        case BINARY_div_ass: return "/=";
        case BINARY_mul_ass: return "*=";
        case BINARY_and: return "and";
        case BINARY_or: return "or";
        case BINARY_less: return "<";
        case BINARY_greater: return ">";
        case BINARY_leq: return "<=";
        case BINARY_geq: return ">=";
        case BINARY_eq: return "==";
        case BINARY_neq: return "!=";
        case BINARY_function_call: return "call";
        case BINARY_subscript: return "subscript";
        case TERNARY_slice: return "slice";
    }
    return "<invalid unary operation>";
}

static bool op_is_unary(f_operation_cat cat) {
    return cat > UNARY_FIRST && cat < BINARY_FIRST;
}

static bool op_is_binary(f_operation_cat cat) {
    return cat > BINARY_FIRST && cat < TERNARY_FIRST;
}

static bool op_is_ternary(f_operation_cat cat) {
    return cat > TERNARY_FIRST && cat < OPERATOR_LAST;
}

static bool op_is_assignment(f_operation_cat cat) {
    return cat > ASS_FIRST && cat < ASS_LAST;
}

static bool op_is_arithmetic_assignment(f_operation_cat cat) {
    return cat > ARR_ASS_FIRST && cat < ASS_LAST;
}

