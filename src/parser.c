
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
    BINARY_ass,
    BINARY_add_ass,
    BINARY_sub_ass,
    BINARY_mul_ass,
    BINARY_div_ass,
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
    t_type_node *type;
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
    t_ast_node *scope;
    union {
        t_expr_node expr;
        t_type_node type;
        t_stmt_node stmt;
        t_ast_list list;
    };
};

static t_ast_list all_procs;

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

static t_ast_node *type_float;
static t_ast_node *type_string;
static t_ast_node *type_bool;
static t_ast_node *type_int;
static t_ast_node *type_byte;

static t_ast_node *alloc_node(void) {
    t_ast_node *result = global_alloc(sizeof(t_ast_node));
    memset(result, 0, sizeof(t_ast_node));
    return result;
}

static t_ast_list *alloc_list(void) {
    t_ast_list *result = global_alloc(sizeof(t_ast_list));
    result->first = null;
    result->last = null;
    return result;
}

static t_ast_list_link *alloc_list_link(void) {
    t_ast_list_link *result = global_alloc(sizeof(t_ast_list_link));
    result->next = null;
    result->prev = null;
    return result;
}

static void init_primitive_type(t_ast_node *type, t_intern const *keyword) {
    type->cat = AST_type_node;
    type->type.cat = TYPE_alias;
    type->type.name = keyword;
}

static void parser_init_memory(void) {
    keyword_if       = intern_cstring("if");
    keyword_else     = intern_cstring("else");
    keyword_while    = intern_cstring("while");
    
    keyword_break    = intern_cstring("break");
    keyword_continue = intern_cstring("continue");
    keyword_return   = intern_cstring("return");
    keyword_print    = intern_cstring("print");
    
    keyword_and      = intern_cstring("and");
    keyword_or       = intern_cstring("or");
    
    keyword_bool     = intern_cstring("bool");
    keyword_byte     = intern_cstring("byte");
    keyword_int      = intern_cstring("int");
    keyword_float    = intern_cstring("float");
    keyword_string   = intern_cstring("string");
    
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

static inline bool unexpected_last_token(t_lexstate *state) {
    push_errorf("%s(%d, %d): unexpected token %s.",
                state->filename,
                state->line, state->offset,
                get_token_string(&state->last_token));
    return false;
}

static inline void parse_error(t_lexstate *state, char const *string) {
    push_errorf("%s(%d, %d): %s",
                state->filename,
                state->line, state->offset,
                string);
}

static void node_list_push(t_ast_list *list, t_ast_list_link *to_attach) {
    to_attach->next = null;
    to_attach->prev = list->last;
    if(list->last != null) {
        list->last->next = to_attach;
    }
    else {
        list->first = to_attach;
    }
    list->last = to_attach;
}

static void load_expr_node_data(t_ast_node *node, t_token *tok) {
    node->cat = AST_expr_node;
    switch(tok->kind) {
        case TOKEN_INT: {
            node->expr.cat = EXPR_int_value;
            node->expr.ivalue = tok->int_value;
            node->expr.type = &type_int->type;
        } break;
        case TOKEN_FLT: {
            node->expr.cat = EXPR_float_value;
            node->expr.fvalue = tok->flt_value;
            node->expr.type = &type_float->type;
        } break;
        case TOKEN_STR: {
            node->expr.cat = EXPR_string_value;
            node->expr.svalue = tok->str_value;
            node->expr.type = &type_string->type;
        } break;
        case TOKEN_IDN: {
            node->expr.cat = EXPR_variable;
            node->expr.var_name = tok->str_value;
            node->expr.type = null; // to be derived later;
        } break;
        default:
        assert(false);
    }
    
}

//--------------------||
// EXPRESSION PARSING ||
//--------------------||

static t_ast_node *parse_expr(t_lexstate *state);

static t_ast_node *parse_expr_list(t_lexstate *state,
                                   t_token_kind seq_separator,
                                   t_token_kind seq_terminator) {
    t_ast_node *node = alloc_node();
    node->cat = AST_list_node;
    if(token_is_kind(state, seq_terminator)) {
        return null;
    }
    while(true) {
        t_ast_node *value = parse_expr(state);
        assert(value);
        if(value == null) {
            parse_error(state, "error parsing expression in expr-list.");
            return node;
        }
        t_ast_list_link *link = alloc_list_link();
        link->p = value;
        node_list_push(&node->list, link);
        if(!token_match_kind(state, seq_separator)) {
            break;
        }
    }
    return node;
}

static t_ast_node *parse_expr1(t_lexstate *state) {
    t_ast_node *node;
    if(token_match_kind(state, '(')) {
        node = parse_expr(state);
        token_expect_kind(state, ')');
    }
    else if(state->last_token.kind == TOKEN_STR
            || state->last_token.kind == TOKEN_IDN
            || state->last_token.kind == TOKEN_INT
            || state->last_token.kind == TOKEN_FLT) {
        node = alloc_node();
        load_expr_node_data(node, &state->last_token);
        lex_next_token(state);
    }
    else {
        unexpected_last_token(state);
        return null;
    }
    
    if(token_match_kind(state, '(')) {
        
        t_ast_node *parameter_list = parse_expr_list(state, ',', ')');
        token_expect_kind(state, ')');
        
        t_ast_node *function_call = alloc_node();
        function_call->cat = AST_expr_node;
        function_call->expr.cat = EXPR_binary_op;
        function_call->expr.opr1 = node;
        function_call->expr.opr2 = parameter_list;
    }
    return node;
}

static t_ast_node *parse_expr2(t_lexstate *state) {
    // sub
    if(token_match_kind(state, '-')) {
        t_ast_node *expr = parse_expr2(state);
        if(expr == null) return null;
        t_ast_node *node = alloc_node();
        node->cat = AST_expr_node;
        node->expr.cat = EXPR_unary_op;
        node->expr.op = UNARY_sub;
        node->expr.opr1 = expr;
        return node;
    }
    
    // address-of
    else if(token_match_kind(state, '$')) {
        t_ast_node *expr = parse_expr2(state);
        if(expr == null) return null;
        t_ast_node *node = alloc_node();
        node->cat = AST_expr_node;
        node->expr.cat = EXPR_unary_op;
        node->expr.op = UNARY_addr;
        node->expr.opr1 = expr;
        return node;
    }
    
    // deref
    else if(token_match_kind(state, '@')) {
        t_ast_node *expr = parse_expr2(state);
        if(expr == null) return null;
        t_ast_node *node = alloc_node();
        node->cat = AST_expr_node;
        node->expr.cat = EXPR_unary_op;
        node->expr.op = UNARY_deref;
        node->expr.opr1 = expr;
        return node;
    }
    else {
        t_ast_node *result = parse_expr1(state);
        if(result == null) {
            return null;
        }
        
        while(true) {
            if(token_match_kind(state, '[')) {
                t_ast_node *first_index = null;
                if(!token_is_kind(state, ':')) {
                    first_index = parse_expr(state);
                    if(first_index == null) {
                        return null;
                    }
                }
                t_ast_node *node = alloc_node();
                
                if(token_match_kind(state, ':')) {
                    node->cat = AST_expr_node; //slicing operator
                    node->expr.cat = EXPR_ternary_op;
                    node->expr.op = TERNARY_slice;
                    node->expr.opr1 = result;
                    node->expr.opr2 = first_index;
                    
                    t_ast_node *second_index = null;
                    if(!token_is_kind(state, ']')) {
                        second_index = parse_expr(state);
                        if(second_index == null) {
                            return null;
                        }
                    }
                    node->expr.opr3 = second_index;
                    token_expect_kind(state, ']');
                }
                
                else {
                    node->cat = AST_expr_node; // array access operator
                    node->expr.cat = EXPR_binary_op;
                    node->expr.op = BINARY_subscript;
                    node->expr.opr1 = result;
                    node->expr.opr2 = first_index;
                    token_expect_kind(state, ']');
                }
                result = node;
            }
            else break;
        }
        return result;
    }
}

static t_ast_node *parse_expr3(t_lexstate *state) {
    t_ast_node *operand_left = parse_expr2(state);
    if(null == operand_left) return null;
    while(true) {
        
        if(token_is_kind(state, '*')) {
            t_token op_token = state->last_token;
            lex_next_token(state);
            t_ast_node *operand_right = parse_expr2(state);
            if(operand_right == null) return null;
            t_ast_node *node = alloc_node();
            node->cat = AST_expr_node;
            node->expr.cat = EXPR_binary_op;
            node->expr.op = BINARY_mul;
            node->expr.opr1 = operand_left;
            node->expr.opr2 = operand_right;
            operand_left = node;
        }
        
        else if(token_is_kind(state, '/')) {
            t_token op_token = state->last_token;
            lex_next_token(state);
            t_ast_node *operand_right = parse_expr2(state);
            if(operand_right == null) return null;
            t_ast_node *node = alloc_node();
            node->cat = AST_expr_node;
            node->expr.cat = EXPR_binary_op;
            node->expr.op = BINARY_div;
            node->expr.opr1 = operand_left;
            node->expr.opr2 = operand_right;
            operand_left = node;
        }
        else break;
    }
    return operand_left;
}

static t_ast_node *parse_expr4(t_lexstate *state) {
    t_ast_node *operand_left = parse_expr3(state);
    if(null != operand_left) {
        while(true) {
            
            if(token_is_kind(state, '+')) {
                t_token op_token = state->last_token;
                lex_next_token(state);
                t_ast_node *operand_right = parse_expr3(state);
                if(operand_right == null) return null;
                t_ast_node *node = alloc_node();
                node->cat = AST_expr_node;
                node->expr.cat = EXPR_binary_op;
                node->expr.op = BINARY_add;
                node->expr.opr1 = operand_left;
                node->expr.opr2 = operand_right;
                operand_left = node;
            }
            
            else if(token_is_kind(state, '-')) {
                t_token op_token = state->last_token;
                lex_next_token(state);
                t_ast_node *operand_right = parse_expr3(state);
                if(operand_right == null) return null;
                t_ast_node *node = alloc_node();
                node->cat = AST_expr_node;
                node->expr.cat = EXPR_binary_op;
                node->expr.op = BINARY_sub;
                node->expr.opr1 = operand_left;
                node->expr.opr2 = operand_right;
                operand_left = node;
            }
            else break;
        }
    }
    return operand_left;
}

static t_ast_node *parse_expr5(t_lexstate *state) {
    t_ast_node *operand_left = parse_expr4(state);
    if(null == operand_left) return null;
    while(true) {
        
        if(token_is_kind(state, '<')) {
            t_token op_token = state->last_token;
            lex_next_token(state);
            t_ast_node *operand_right = parse_expr4(state);
            if(operand_right == null) return null;
            t_ast_node *node = alloc_node();
            node->cat = AST_expr_node;
            node->expr.cat = EXPR_binary_op;
            node->expr.op = BINARY_less;
            node->expr.opr1 = operand_left;
            node->expr.opr2 = operand_right;
            operand_left = node;
        }
        
        else if(token_is_kind(state, '>')) {
            t_token op_token = state->last_token;
            lex_next_token(state);
            t_ast_node *operand_right = parse_expr4(state);
            if(operand_right == null) return null;
            t_ast_node *node = alloc_node();
            node->cat = AST_expr_node;
            node->expr.cat = EXPR_binary_op;
            node->expr.op = BINARY_greater;
            node->expr.opr1 = operand_left;
            node->expr.opr2 = operand_right;
            operand_left = node;
        }
        
        else if(token_is_kind(state, TOKEN_CMP_LEQ)) {
            t_token op_token = state->last_token;
            lex_next_token(state);
            t_ast_node *operand_right = parse_expr4(state);
            if(operand_right == null) return null;
            t_ast_node *node = alloc_node();
            node->cat = AST_expr_node;
            node->expr.cat = EXPR_binary_op;
            node->expr.op = BINARY_leq;
            node->expr.opr1 = operand_left;
            node->expr.opr2 = operand_right;
            operand_left = node;
        }
        
        else if(token_is_kind(state, TOKEN_CMP_GEQ)) {
            t_token op_token = state->last_token;
            lex_next_token(state);
            t_ast_node *operand_right = parse_expr4(state);
            if(operand_right == null) return null;
            t_ast_node *node = alloc_node();
            node->cat = AST_expr_node;
            node->expr.cat = EXPR_binary_op;
            node->expr.op = BINARY_geq;
            node->expr.opr1 = operand_left;
            node->expr.opr2 = operand_right;
            operand_left = node;
        }
        
        else if(token_is_kind(state, TOKEN_CMP_EQ)) {
            t_token op_token = state->last_token;
            lex_next_token(state);
            t_ast_node *operand_right = parse_expr4(state);
            if(operand_right == null) return null;
            t_ast_node *node = alloc_node();
            node->cat = AST_expr_node;
            node->expr.cat = EXPR_binary_op;
            node->expr.op = BINARY_eq;
            node->expr.opr1 = operand_left;
            node->expr.opr2 = operand_right;
            operand_left = node;
        }
        
        else if(token_is_kind(state, TOKEN_CMP_NEQ)) {
            t_token op_token = state->last_token;
            lex_next_token(state);
            t_ast_node *operand_right = parse_expr4(state);
            if(operand_right == null) return null;
            t_ast_node *node = alloc_node();
            node->cat = AST_expr_node;
            node->expr.cat = EXPR_binary_op;
            node->expr.op = BINARY_neq;
            node->expr.opr1 = operand_left;
            node->expr.opr2 = operand_right;
            operand_left = node;
        }
        else break;
    }
    return operand_left;
}

static t_ast_node *parse_expr6(t_lexstate *state) {
    t_ast_node *operand_left = parse_expr5(state);
    if(null == operand_left) return null;
    while(token_is_identifier(state, keyword_and)) {
        t_token op_token = state->last_token;
        lex_next_token(state);
        t_ast_node *operand_right = parse_expr5(state);
        if(operand_right == null) return null;
        t_ast_node *node = alloc_node();
        node->cat = AST_expr_node;
        node->expr.cat = EXPR_binary_op;
        node->expr.op = BINARY_and;
        node->expr.opr1 = operand_left;
        node->expr.opr2 = operand_right;
        operand_left = node;
    }
    return operand_left;
}

static t_ast_node *parse_expr7(t_lexstate *state) {
    t_ast_node *operand_left = parse_expr6(state);
    if(null == operand_left) return null;
    while(token_is_identifier(state, keyword_or)) {
        t_token op_token = state->last_token;
        lex_next_token(state);
        t_ast_node *operand_right = parse_expr6(state);
        if(operand_right == null) return null;
        t_ast_node *node = alloc_node();
        node->cat = AST_expr_node;
        node->expr.cat = EXPR_binary_op;
        node->expr.op = BINARY_or;
        node->expr.opr1 = operand_left;
        node->expr.opr2 = operand_right;
        operand_left = node;
    }
    return operand_left;
}

static t_ast_node *parse_expr(t_lexstate *state) {
    return parse_expr7(state);
}

/* STATEMENTS PARSER */


static t_ast_node *parse_assignment(t_lexstate *state) {
    t_ast_node *operand_left = parse_expr7(state);
    if(null != operand_left) {
        if(token_is_kind(state, TOKEN_ADD_ASS)) {
            
            t_token op_token = state->last_token;
            lex_next_token(state);
            t_ast_node *operand_right = parse_expr7(state);
            t_ast_node *node = alloc_node();
            node->cat = AST_expr_node;
            node->expr.cat = EXPR_binary_op;
            node->expr.op = BINARY_add_ass;
            node->expr.opr1 = operand_left;
            node->expr.opr2 = operand_right;
            operand_left = node;
        }
        else if(token_is_kind(state, TOKEN_SUB_ASS)) {
            
            t_token op_token = state->last_token;
            lex_next_token(state);
            t_ast_node *operand_right = parse_expr7(state);
            t_ast_node *node = alloc_node();
            node->cat = AST_expr_node;
            node->expr.cat = EXPR_binary_op;
            node->expr.op = BINARY_geq;
            node->expr.opr1 = operand_left;
            node->expr.opr2 = operand_right;
            operand_left = node;
        }
        else if(token_is_kind(state, TOKEN_MUL_ASS)) {
            
            t_token op_token = state->last_token;
            lex_next_token(state);
            t_ast_node *operand_right = parse_expr7(state);
            t_ast_node *node = alloc_node();
            node->cat = AST_expr_node;
            node->expr.cat = EXPR_binary_op;
            node->expr.op = BINARY_mul_ass;
            node->expr.opr1 = operand_left;
            node->expr.opr2 = operand_right;
            operand_left = node;
        }
        else if(token_is_kind(state, TOKEN_DIV_ASS)) {
            
            t_token op_token = state->last_token;
            lex_next_token(state);
            t_ast_node *operand_right = parse_expr7(state);
            t_ast_node *node = alloc_node();
            node->cat = AST_expr_node;
            node->expr.cat = EXPR_binary_op;
            node->expr.op = BINARY_div_ass;
            node->expr.opr1 = operand_left;
            node->expr.opr2 = operand_right;
            operand_left = node;
        }
        else if(token_is_kind(state, '=')) {
            
            t_token op_token = state->last_token;
            lex_next_token(state);
            t_ast_node *operand_right = parse_expr7(state);
            t_ast_node *node = alloc_node();
            node->cat = AST_expr_node;
            node->expr.cat = EXPR_binary_op;
            node->expr.op = BINARY_div_ass;
            node->expr.opr1 = operand_left;
            node->expr.opr2 = operand_right;
            operand_left = node;
        }
    }
    //token_expect(state, ';');
    return operand_left;
}


static t_ast_node *parse_stmts(t_lexstate *state);
static t_ast_node *parse_stmt(t_lexstate *state);

static t_ast_node *parse_if_stmt(t_lexstate *state) {
    token_expect_identifier(state, keyword_if);
    t_ast_node *node = alloc_node();
    t_ast_node *condition = parse_expr(state);
    t_ast_node *block = parse_stmts(state);
    if(token_match_identifier(state, keyword_else)) {
        t_ast_node *else_stmt;
        if(token_is_kind(state, '{')) {
            else_stmt = parse_stmts(state);
        }
        else {
            else_stmt = parse_stmts(state);
            token_expect_kind(state, ';');
        }
        node->stmt.if_false_block = else_stmt;
    }
    
    node->stmt.if_condition = condition;
    node->stmt.if_true_block = block;
    node->stmt.cat = STMT_if;
    node->cat = AST_stmt_node;
    return node;
}

static t_ast_node *parse_while_stmt(t_lexstate *state) {
    token_expect_identifier(state, keyword_while);
    t_ast_node *condition = parse_expr(state);
    t_ast_node *block = parse_stmts(state);
    t_ast_node *node = alloc_node();
    node->cat = AST_stmt_node;
    node->stmt.cat = STMT_while;
    node->stmt.while_condition = condition;
    node->stmt.while_block = block;
    return node;
}

static t_ast_node *parse_type(t_lexstate *state) {
    t_ast_node *node = alloc_node();
    node->cat = AST_type_node;
    
    t_token primitive = state->last_token;
    if(token_expect_kind(state, TOKEN_IDN)) {
        node->type.cat = TYPE_alias;
        node->type.name = primitive.str_value;
    }
    else return null;
    
    while(true) {
        
        if(token_match_kind(state, '$')) {
            t_ast_node *pnode = alloc_node();
            pnode->cat = AST_type_node;
            pnode->type.cat = TYPE_pointer;
            pnode->type.base_type = node;
            node = pnode;
        }
        else if(token_match_kind(state, '[')) {
            if(!token_expect_kind(state, ']')) {
                printf("  note: x-lang doesn't have constant array types.");
                return null;
            }
            t_ast_node *pnode = alloc_node();
            pnode->cat = AST_type_node;
            pnode->type.cat = TYPE_slice;
            pnode->type.base_type = node;
            node = pnode;
        }
        else if(token_match_kind(state, TOKEN_LEFT_ARROW)) {
            if(!token_expect_kind(state, '(')) {
                return null;
            }
            t_ast_node *decls = alloc_node();
            decls->cat = AST_list_node;
            t_ast_node *function_node = alloc_node();
            function_node->cat = AST_type_node;
            function_node->type.cat = TYPE_function;
            function_node->type.parameters = decls;
            function_node->type.return_type = node;
            node = function_node;
            while(true) {
                if(token_is_kind(state, ')')) {
                    break;
                }
                t_ast_node *parameter_type = parse_type(state);
                assert(parameter_type->cat == AST_type_node);
                if(parameter_type == null) {
                    parse_error(state, "no type in function declarator");
                }
                t_intern const *opt_param_name = null;
                t_token parameter_name = state->last_token;
                if(token_match_kind(state, TOKEN_IDN)) {
                    opt_param_name = parameter_name.str_value;
                }
                t_ast_node *param_decl = alloc_node();
                param_decl->cat = AST_stmt_node;
                param_decl->stmt.cat = STMT_declaration;
                param_decl->stmt.decl_type = parameter_type;
                param_decl->stmt.decl_name = opt_param_name;
                // TODO(bumbread): function default parameters ?
                param_decl->stmt.decl_value = null;
                
                t_ast_list_link *param_link = alloc_list_link();
                param_link->p = param_decl;
                node_list_push(&decls->list, param_link);
                
                if(!token_match_kind(state, ';')) {
                    break;
                }
            }
            if(!token_expect_kind(state, ')')) {
                return null;
            }
        }
    }
    
    return node;
}

static t_ast_node *parse_declaration(t_lexstate *state) {
    token_expect_kind(state, ':');
    
    t_ast_node *node = alloc_node();
    node->cat = AST_stmt_node;
    node->stmt.decl_type = parse_type(state);
    
    t_token name = state->last_token;
    if(token_expect_kind(state, TOKEN_IDN)) {
        node->stmt.decl_name = name.str_value;
    }
    
    if(token_match_kind(state, '=')) {
        node->stmt.decl_value = parse_expr(state);
        token_expect_kind(state, ';');
    }
    else if(token_is_kind(state, '{')) {
        node->stmt.decl_value = parse_stmts(state);
    }
    else {
        if(!token_expect_kind(state, ';')) {
            return null;
        }
    }
    
    assert(node->stmt.decl_type != null);
    t_ast_node *decl_type = node->stmt.decl_type;
    assert(decl_type->cat == AST_type_node);
    if(decl_type->type.cat == TYPE_function) {
        t_ast_list_link *func_link = alloc_list_link();
        func_link->p = node;
        node_list_push(&all_procs, func_link);
    }
    return node;
}

static t_ast_node *parse_stmt(t_lexstate *state) {
    t_ast_node *node = null;
    if(token_is_identifier(state, keyword_if)) {
        node = parse_if_stmt(state);
    }
    else if(token_is_identifier(state, keyword_while)) {
        node = parse_while_stmt(state);
    }
    else if(token_is_kind(state, ':')) {
        node = parse_declaration(state);
    }
    else if(token_match_identifier(state, keyword_return)) {
        node = alloc_node();
        node->cat = AST_stmt_node;
        node->stmt.cat = STMT_return;
        node->stmt.stmt_value = parse_expr(state);
        token_expect_kind(state, ';');
    }
    else if(token_match_identifier(state, keyword_break)) {
        node = alloc_node();
        node->cat = AST_stmt_node;
        node->stmt.cat = STMT_break;
        token_expect_kind(state, ';');
    }
    else if(token_match_identifier(state, keyword_continue)) {
        node = alloc_node();
        node->cat = AST_stmt_node;
        node->stmt.cat = STMT_continue;
        token_expect_kind(state, ';');
    }
    else if(token_match_identifier(state, keyword_print)) {
        node = alloc_node();
        node->cat = AST_stmt_node;
        node->stmt.cat = STMT_print;
        node->stmt.stmt_value = parse_expr(state);
        token_expect_kind(state, ';');
    }
    else if(token_is_kind(state, '{')) {
        node = parse_stmts(state);
    }
    else {
        node = parse_assignment(state);
        token_expect_kind(state, ';');
    }
    return node;
}

static t_ast_node *parse_stmts(t_lexstate *state) {
    if(!token_expect_kind(state, '{')) return null;
    
    t_ast_node *block = alloc_node();
    block->cat = AST_stmt_node;
    block->stmt.cat = STMT_block;
    while(true) {
        if(token_match_kind(state, '}')) {
            break;
        }
        else {
            t_ast_node *node = parse_stmt(state);
            t_ast_list_link *link = alloc_list_link();
            link->p = node;
            node_list_push(&block->stmt.statements, link);
        }
    }
    return block;
}

static t_ast_node *parse_global_scope(t_lexstate *state) {
    t_ast_node *block = alloc_node();
    block->cat = AST_stmt_node;
    block->stmt.cat = STMT_block;
    while(true) {
        if(token_match_kind(state, TOKEN_EOF)) {
            break;
        }
        else {
            t_ast_node *node = parse_declaration(state);
            if(node == null) {
                parse_error(state, "unable to parse declaration");
                return block;
            }
            t_ast_list_link *link = alloc_list_link();
            link->p = node;
            node_list_push(&block->stmt.statements, link);
        }
    }
    return block;
}

// debug functions
static t_ast_node *parse_ast_node_expr_level(char const *expr) {
    t_lexstate state;
    lex_init(&state, "@test", expr);
    lex_next_token(&state);
    t_ast_node *code = parse_expr(&state);
    check_errors();
    return code;
}
static t_ast_node *parse_ast_node_stmt_level(char const *stmt) {
    t_lexstate state;
    lex_init(&state, "@test", stmt);
    lex_next_token(&state);
    t_ast_node *code = parse_stmt(&state);
    check_errors();
    return code;
}
static t_ast_node *parse_ast_node_global_level(char const *code_str) {
    t_lexstate state;
    lex_init(&state, "@test", code_str);
    lex_next_token(&state);
    t_ast_node *code = parse_global_scope(&state);
    check_errors();
    return code;
}

static char const *get_operator_string(t_operator_cat cat) {
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
        case BINARY_div_ass: return "*=";
        case BINARY_mul_ass: return "/=";
        case BINARY_and: return "and";
        case BINARY_or: return "or";
        case BINARY_less: return "<";
        case BINARY_greater: return ">";
        case BINARY_leq: return "<=";
        case BINARY_geq: return ">=";
        case BINARY_eq: return "==";
        case BINARY_neq: return "!=";
        case BINARY_subscript: return "subscript";
        case TERNARY_slice: return "slice";
    }
    return "<invalid unary operation>";
}

static bool op_is_unary(t_operator_cat cat) {
    return cat > UNARY_FIRST_OPERATOR && cat < BINARY_FIRST_OPERATOR;
}

static bool op_is_binary(t_operator_cat cat) {
    return cat > BINARY_FIRST_OPERATOR && cat < TERNARY_FIRST_OPERATOR;
}

static bool op_is_ternary(t_operator_cat cat) {
    return cat > TERNARY_FIRST_OPERATOR && cat < LAST_OPERATOR;
}
