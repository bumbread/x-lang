
enum {
    AST_value_node,
    AST_type_node,
    AST_ternary_expr_node,
    AST_binary_expr_node,
    AST_unary_expr_node,
    AST_decl_node,
    AST_decl_list_node,
    AST_expr_list_node,
    AST_stmt_node,
    AST_stmt_block,
} typedef t_ast_node_type;

enum {
    TYPE_none,
    TYPE_primitive,
    TYPE_pointer,
    TYPE_slice,
    TYPE_function,
} typedef t_type_category;

enum {
    STMT_if,
    STMT_while,
    STMT_return,
    STMT_break,
    STMT_continue,
    STMT_print, // temporary (TODO)
} typedef t_ast_stmt_category;

u32 typedef t_unary_op_cat;

enum {
    BINARY_subscript = 128,
    BINARY_leq,
    BINARY_geq,
    BINARY_eq,
    BINARY_neq,
    BINARY_and,
    BINARY_or,
    BINARY_add_ass,
    BINARY_sub_ass,
    BINARY_mul_ass,
    BINARY_div_ass,
} typedef t_binary_op_cat;

enum {
    TERNARY_slice = 128,
} typedef t_ternary_op_cat;

//struct t_ast_node_;
typedef struct t_ast_node_ t_ast_node;
struct t_ast_node_ {
    t_ast_node_type type;
    union {
        //AST_value_node
        struct {
            t_token value_token;
        };
        //AST_ternary_expr_node
        struct {
            t_ast_node *ternary_opr1;
            t_ast_node *ternary_opr2;
            t_ast_node *ternary_opr3;
            t_ternary_op_cat ternary_op;
        };
        //AST_binary_expr_node
        struct {
            t_ast_node *binary_opr1;
            t_ast_node *binary_opr2;
            t_binary_op_cat binary_op;
        };
        //AST_unary_expr_node
        struct {
            t_ast_node *unary_opr1;
            t_unary_op_cat unary_op;
        };
        //AST_type_node
        struct {
            t_type_category type_cat;
            union {
                // primitive
                t_intern const *primitive_typename;
                // pointer, slice
                t_ast_node *base_type;
                // functions
                struct {
                    t_ast_node *function_parameters;
                    t_ast_node *function_return_type;
                };
            };
        };
        // AST_decl_node
        struct {
            t_intern const *decl_name;
            t_ast_node *decl_type;
            t_ast_node *decl_value;
        };
        // AST_stmt_node,
        struct {
            t_ast_stmt_category stmt_cat;
            union {
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
                t_ast_node *stmt_value;
            };
        };
    };
    t_ast_node *value_type;
    t_ast_node *scope;
    t_ast_node *first;
    t_ast_node *prev;
    t_ast_node *next;
    t_ast_node *last;
};

static t_ast_node *all_procs;

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

static t_ast_node *alloc_ast_node(void) {
    t_ast_node *result = global_alloc(sizeof(t_ast_node));
    result->first = null;
    result->last = null;
    result->next = null;
    result->prev = null;
    result->scope = null;
    result->value_type = null;
    return result;
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
    
    all_procs = alloc_ast_node();
}

static inline bool token_is(t_lexstate *state, t_token_kind kind) {
    return state->last_token.kind == kind;
}

static inline bool token_match(t_lexstate *state, t_token_kind kind) {
    if(state->last_token.kind == kind) {
        lex_next_token(state);
        return true;
    }
    return false;
}

static inline void parse_error(t_lexstate *state, char const *string) {
    push_errorf("%s(%d, %d): %s",
                state->filename,
                state->line, state->offset,
                string);
}

static inline bool token_expect(t_lexstate *state, t_token_kind kind) {
    if(state->last_token.kind == kind) {
        lex_next_token(state);
        return true;
    }
    push_errorf("%s(%d, %d): expected token %s, got %s",
                state->filename,
                state->line, state->offset,
                get_token_kind_name(kind),
                get_token_string(&state->last_token));
    return false;
}


static inline bool token_is_identifier(t_lexstate *state, t_intern const *str) {
    if(state->last_token.kind == TOKEN_IDN) {
        return state->last_token.str_value == str;
    }
    return false;
}

static inline bool token_match_identifier(t_lexstate *state, t_intern const *str) {
    if(state->last_token.kind == TOKEN_IDN && state->last_token.str_value == str) {
        lex_next_token(state);
        return true;
    }
    return false;
}

static inline bool token_expect_identifier(t_lexstate *state, t_intern const *str) {
    if(state->last_token.kind == TOKEN_IDN && state->last_token.str_value == str) {
        lex_next_token(state);
        return true;
    }
    push_errorf("expected keyword %s, got %s", state->last_token.str_value->str, str->str);
    return false;
}

static void node_attach_next(t_ast_node *list, t_ast_node *to_attach) {
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

//
// expr0 = val | '(' expr ')'
// expr1 = expr0 | -expr1
// expr2 = expr1 [('*'|'/') expr1 ...]
// expr3 = expr2 [('+'|'-') expr2 ...]
// expr  = expr3 [('and', 'or') expr3 ...]
//

static t_ast_node *parse_expr(t_lexstate *state);

static t_ast_node *parse_expr_list(t_lexstate *state,
                                   t_token_kind seq_separator,
                                   t_token_kind seq_terminator) {
    t_ast_node *result = alloc_ast_node();
    result->type = AST_expr_list_node;
    if(token_is(state, seq_terminator)) {
        return result;
    }
    while(true) {
        t_ast_node *value = parse_expr(state);
        if(value == null) {
            parse_error(state, "error parsing expression in expr-list.");
            return result;
        }
        node_attach_next(result, value);
        if(token_match(state, seq_separator)) {
            continue;
        }
        break;
    }
    return result;
}

static t_ast_node *parse_expr1(t_lexstate *state) {
    t_ast_node *node;
    if(token_match(state, '(')) {
        node = parse_expr(state);
        token_expect(state, ')');
    }
    else if(state->last_token.kind == TOKEN_STR
            || state->last_token.kind == TOKEN_IDN
            || state->last_token.kind == TOKEN_INT
            || state->last_token.kind == TOKEN_FLT) {
        node = alloc_ast_node();
        node->type = AST_value_node;
        node->value_token = state->last_token;
        lex_next_token(state);
    }
    else return null;
    
    if(token_match(state, '(')) {
        t_ast_node *function_call = alloc_ast_node();
        t_ast_node *parameter_list = parse_expr_list(state, ',', ')');
        token_expect(state, ')');
        function_call->type = AST_binary_expr_node;
        function_call->binary_opr1 = node;
        function_call->binary_opr2 = parameter_list;
    }
    return node;
}

static t_ast_node *parse_expr2(t_lexstate *state) {
    // sub
    if(token_match(state, '-')) {
        t_ast_node *node = alloc_ast_node();
        node->type = AST_unary_expr_node;
        node->unary_opr1 = parse_expr2(state);
        node->unary_op = '-';
        return node;
    }
    // address-of
    else if(token_match(state, '$')) {
        t_ast_node *node = alloc_ast_node();
        node->type = AST_unary_expr_node;
        node->unary_opr1 = parse_expr2(state);
        node->unary_op = '$';
        return node;
    }
    // deref
    else if(token_match(state, '@')) {
        t_ast_node *node = alloc_ast_node();
        node->type = AST_unary_expr_node;
        node->unary_opr1 = parse_expr2(state);
        node->unary_op = '@';
        return node;
    }
    else {
        t_ast_node *result = parse_expr1(state);
        while(true) {
            if(token_match(state, '[')) {
                t_ast_node *first_index = parse_expr(state);
                t_ast_node *node = alloc_ast_node();
                if(token_match(state, ':')) {
                    node->type = AST_ternary_expr_node; //slicing operator
                    node->ternary_opr1 = result;
                    node->ternary_opr2 = first_index;
                    node->ternary_opr3 = parse_expr(state);
                    node->ternary_op = TERNARY_slice;
                    token_expect(state, ']');
                }
                else {
                    node->type = AST_binary_expr_node; // array access operator
                    node->binary_opr1 = result;
                    node->binary_opr2 = first_index;
                    node->binary_op = BINARY_subscript;
                    token_expect(state, ']');
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
    if(null != operand_left) {
        while(true) {
            if(token_is(state, '*')) {
                
                t_token op_token = state->last_token;
                lex_next_token(state);
                t_ast_node *operand_right = parse_expr2(state);
                t_ast_node *node = alloc_ast_node();
                node->type = AST_binary_expr_node;
                node->binary_op = '*';
                node->binary_opr1 = operand_left;
                node->binary_opr2 = operand_right;
                operand_left = node;
            }
            else if(token_is(state, '/')) {
                
                t_token op_token = state->last_token;
                lex_next_token(state);
                t_ast_node *operand_right = parse_expr2(state);
                t_ast_node *node = alloc_ast_node();
                node->type = AST_binary_expr_node;
                node->binary_op = '/';
                node->binary_opr1 = operand_left;
                node->binary_opr2 = operand_right;
                operand_left = node;
            }
            else break;
        }
    }
    return operand_left;
}

static t_ast_node *parse_expr4(t_lexstate *state) {
    t_ast_node *operand_left = parse_expr3(state);
    if(null != operand_left) {
        while(true) {
            if(token_is(state, '+')) {
                
                t_token op_token = state->last_token;
                lex_next_token(state);
                t_ast_node *operand_right = parse_expr3(state);
                t_ast_node *node = alloc_ast_node();
                node->type = AST_binary_expr_node;
                node->binary_op = '+';
                node->binary_opr1 = operand_left;
                node->binary_opr2 = operand_right;
                operand_left = node;
            }
            else if(token_is(state, '-')) {
                
                t_token op_token = state->last_token;
                lex_next_token(state);
                t_ast_node *operand_right = parse_expr3(state);
                t_ast_node *node = alloc_ast_node();
                node->type = AST_binary_expr_node;
                node->binary_op = '-';
                node->binary_opr1 = operand_left;
                node->binary_opr2 = operand_right;
                operand_left = node;
            }
            else break;
        }
    }
    return operand_left;
}

static t_ast_node *parse_expr5(t_lexstate *state) {
    t_ast_node *operand_left = parse_expr4(state);
    if(null != operand_left) {
        while(true) {
            if(token_is(state, '<')) {
                t_token op_token = state->last_token;
                lex_next_token(state);
                t_ast_node *operand_right = parse_expr4(state);
                
                t_ast_node *node = alloc_ast_node();
                node->type = AST_binary_expr_node;
                node->binary_op = '<';
                node->binary_opr1 = operand_left;
                node->binary_opr2 = operand_right;
                operand_left = node;
            }
            else if(token_is(state, '>')) {
                t_token op_token = state->last_token;
                lex_next_token(state);
                t_ast_node *operand_right = parse_expr4(state);
                
                t_ast_node *node = alloc_ast_node();
                node->type = AST_binary_expr_node;
                node->binary_op = '>';
                node->binary_opr1 = operand_left;
                node->binary_opr2 = operand_right;
                operand_left = node;
            }
            else if(token_is(state, TOKEN_CMP_LEQ)) {
                t_token op_token = state->last_token;
                lex_next_token(state);
                t_ast_node *operand_right = parse_expr4(state);
                
                t_ast_node *node = alloc_ast_node();
                node->type = AST_binary_expr_node;
                node->binary_op = BINARY_leq;
                node->binary_opr1 = operand_left;
                node->binary_opr2 = operand_right;
                operand_left = node;
            }
            else if(token_is(state, TOKEN_CMP_GEQ)) {
                t_token op_token = state->last_token;
                lex_next_token(state);
                t_ast_node *operand_right = parse_expr4(state);
                
                t_ast_node *node = alloc_ast_node();
                node->type = AST_binary_expr_node;
                node->binary_op = BINARY_geq;
                node->binary_opr1 = operand_left;
                node->binary_opr2 = operand_right;
                operand_left = node;
            }
            else if(token_is(state, TOKEN_CMP_EQ)) {
                t_token op_token = state->last_token;
                lex_next_token(state);
                t_ast_node *operand_right = parse_expr4(state);
                
                t_ast_node *node = alloc_ast_node();
                node->type = AST_binary_expr_node;
                node->binary_op = BINARY_eq;
                node->binary_opr1 = operand_left;
                node->binary_opr2 = operand_right;
                operand_left = node;
            }
            else if(token_is(state, TOKEN_CMP_NEQ)) {
                t_token op_token = state->last_token;
                lex_next_token(state);
                t_ast_node *operand_right = parse_expr4(state);
                
                t_ast_node *node = alloc_ast_node();
                node->type = AST_binary_expr_node;
                node->binary_op = BINARY_neq;
                node->binary_opr1 = operand_left;
                node->binary_opr2 = operand_right;
                operand_left = node;
            }
            else break;
        }
    }
    return operand_left;
}

static t_ast_node *parse_expr6(t_lexstate *state) {
    t_ast_node *operand_left = parse_expr5(state);
    if(null != operand_left) {
        while(token_is_identifier(state, keyword_and)) {
            t_token op_token = state->last_token;
            lex_next_token(state);
            
            t_ast_node *operand_right = parse_expr5(state);
            
            t_ast_node *node = alloc_ast_node();
            node->type = AST_binary_expr_node;
            node->binary_op = BINARY_and;
            node->binary_opr1 = operand_left;
            node->binary_opr2 = operand_right;
            operand_left = node;
        }
    }
    return operand_left;
}

static t_ast_node *parse_expr7(t_lexstate *state) {
    t_ast_node *operand_left = parse_expr6(state);
    if(null != operand_left) {
        while(token_is_identifier(state, keyword_or)) {
            t_token op_token = state->last_token;
            lex_next_token(state);
            
            t_ast_node *operand_right = parse_expr6(state);
            
            t_ast_node *node = alloc_ast_node();
            node->type = AST_binary_expr_node;
            node->binary_op = BINARY_or;
            node->binary_opr1 = operand_left;
            node->binary_opr2 = operand_right;
            operand_left = node;
        }
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
        if(token_is(state, TOKEN_ADD_ASS)) {
            
            t_token op_token = state->last_token;
            lex_next_token(state);
            t_ast_node *operand_right = parse_expr7(state);
            t_ast_node *node = alloc_ast_node();
            node->type = AST_binary_expr_node;
            node->binary_op = BINARY_add_ass;
            node->binary_opr1 = operand_left;
            node->binary_opr2 = operand_right;
            operand_left = node;
        }
        else if(token_is(state, TOKEN_SUB_ASS)) {
            
            t_token op_token = state->last_token;
            lex_next_token(state);
            t_ast_node *operand_right = parse_expr7(state);
            t_ast_node *node = alloc_ast_node();
            node->type = AST_binary_expr_node;
            node->binary_op = BINARY_sub_ass;
            node->binary_opr1 = operand_left;
            node->binary_opr2 = operand_right;
            operand_left = node;
        }
        else if(token_is(state, TOKEN_MUL_ASS)) {
            
            t_token op_token = state->last_token;
            lex_next_token(state);
            t_ast_node *operand_right = parse_expr7(state);
            t_ast_node *node = alloc_ast_node();
            node->type = AST_binary_expr_node;
            node->binary_op = BINARY_sub_ass;
            node->binary_opr1 = operand_left;
            node->binary_opr2 = operand_right;
            operand_left = node;
        }
        else if(token_is(state, TOKEN_DIV_ASS)) {
            
            t_token op_token = state->last_token;
            lex_next_token(state);
            t_ast_node *operand_right = parse_expr7(state);
            t_ast_node *node = alloc_ast_node();
            node->type = AST_binary_expr_node;
            node->binary_op = BINARY_div_ass;
            node->binary_opr1 = operand_left;
            node->binary_opr2 = operand_right;
            operand_left = node;
        }
        else if(token_is(state, '=')) {
            
            t_token op_token = state->last_token;
            lex_next_token(state);
            t_ast_node *operand_right = parse_expr7(state);
            t_ast_node *node = alloc_ast_node();
            node->type = AST_binary_expr_node;
            node->binary_op = '-';
            node->binary_opr1 = operand_left;
            node->binary_opr2 = operand_right;
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
    t_ast_node *node = alloc_ast_node();
    
    t_ast_node *condition = parse_expr(state);
    t_ast_node *block = parse_stmts(state);
    if(token_match_identifier(state, keyword_else)) {
        t_ast_node *else_stmt;
        if(token_is(state, '{')) {
            else_stmt = parse_stmts(state);
        }
        else {
            else_stmt = parse_stmts(state);
            token_expect(state, ';');
        }
        node->if_false_block = else_stmt;
    }
    
    node->if_condition = condition;
    node->if_true_block = block;
    node->stmt_cat = STMT_if;
    node->type = AST_stmt_node; // ???
    return node;
}

static t_ast_node *parse_while_stmt(t_lexstate *state) {
    token_expect_identifier(state, keyword_while);
    t_ast_node *condition = parse_expr(state);
    t_ast_node *block = parse_stmts(state);
    
    t_ast_node *node = alloc_ast_node();
    node->type = AST_stmt_node;
    node->stmt_cat = STMT_while;
    node->while_condition = condition;
    node->while_block = block;
    return node;
}

static t_ast_node *parse_type(t_lexstate *state) {
    t_ast_node *node = alloc_ast_node();
    node->type = AST_type_node;
    node->type_cat = TYPE_none;
    
    t_token primitive = state->last_token;
    if(token_expect(state, TOKEN_IDN)) {
        node->type_cat = TYPE_primitive;
        node->primitive_typename = primitive.str_value;
    }
    else return null;
    
    while(true) {
        if(node->type_cat != TYPE_none) {
            if(token_match(state, '$')) {
                t_ast_node *pnode = alloc_ast_node();
                pnode->type = AST_type_node;
                pnode->type_cat = TYPE_pointer;
                pnode->base_type = node;
                node = pnode;
            }
            else if(token_match(state, '[')) {
                if(!token_expect(state, ']')) {
                    return null;
                }
                t_ast_node *pnode = alloc_ast_node();
                pnode->type = AST_type_node;
                pnode->type_cat = TYPE_slice;
                pnode->base_type = node;
                node = pnode;
            }
            else if(token_match(state, TOKEN_LEFT_ARROW)) {
                if(!token_expect(state, '(')) {
                    return null;
                }
                t_ast_node *decl_list = alloc_ast_node();
                t_ast_node *function_node = alloc_ast_node();
                function_node->type = AST_type_node;
                function_node->type_cat = TYPE_function;
                function_node->function_parameters = decl_list;
                function_node->function_return_type = node;
                node = function_node;
                while(true) {
                    if(token_is(state, ')')) {
                        break;
                    }
                    t_ast_node *parameter_type = parse_type(state);
                    if(parameter_type == null) {
                        parse_error(state, "no type in function declarator");
                    }
                    t_intern const *opt_param_name = null;
                    t_token parameter_name = state->last_token;
                    if(token_match(state, TOKEN_IDN)) {
                        opt_param_name = parameter_name.str_value;
                    }
                    t_ast_node *func_param_decl_node = alloc_ast_node();
                    func_param_decl_node->type = AST_decl_node;
                    func_param_decl_node->decl_type = parameter_type;
                    func_param_decl_node->decl_name = opt_param_name;
                    func_param_decl_node->decl_value = null;
                    node_attach_next(decl_list, func_param_decl_node);
                    if(!token_match(state, ';')) {
                        break;
                    }
                }
                if(!token_expect(state, ')')) {
                    return null;
                }
            }
            else break;
        }
    }
    return node;
}

static t_ast_node *parse_declaration(t_lexstate *state) {
    token_expect(state, ':');
    
    t_ast_node *node = alloc_ast_node();
    node->type = AST_decl_node;
    node->decl_type = parse_type(state);
    
    t_token name = state->last_token;
    if(token_expect(state, TOKEN_IDN)) {
        node->decl_name = name.str_value;
    }
    
    if(token_match(state, '=')) {
        node->decl_value = parse_expr(state);
        token_expect(state, ';');
    }
    else if(token_is(state, '{')) {
        node->decl_value = parse_stmts(state);
    }
    else {
        if(!token_expect(state, ';')) {
            return null;
        }
    }
    
    assert(node->decl_type != null);
    if(node->decl_type->type_cat == TYPE_function) {
        node_attach_next(all_procs, node);
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
    else if(token_is(state, ':')) {
        node = parse_declaration(state);
    }
    else if(token_match_identifier(state, keyword_return)) {
        node = alloc_ast_node();
        node->type = AST_stmt_node;
        node->stmt_cat = STMT_return;
        node->stmt_value = parse_expr(state);
        token_expect(state, ';');
    }
    else if(token_match_identifier(state, keyword_break)) {
        node = alloc_ast_node();
        node->type = AST_stmt_node;
        node->stmt_cat = STMT_break;
        token_expect(state, ';');
    }
    else if(token_match_identifier(state, keyword_continue)) {
        node = alloc_ast_node();
        node->type = AST_stmt_node;
        node->stmt_cat = STMT_continue;
        token_expect(state, ';');
    }
    else if(token_match_identifier(state, keyword_print)) {
        node = alloc_ast_node();
        node->type = AST_stmt_node;
        node->stmt_cat = STMT_print;
        node->stmt_value = parse_expr(state);
        token_expect(state, ';');
    }
    else if(token_is(state, '{')) {
        node = parse_stmts(state);
    }
    else {
        node = parse_assignment(state);
        token_expect(state, ';');
    }
    return node;
}

static t_ast_node *parse_stmts(t_lexstate *state) {
    if(!token_expect(state, '{')) return null;
    
    t_ast_node *block = alloc_ast_node();
    block->type = AST_stmt_block;
    while(true) {
        if(token_match(state, '}')) {
            break;
        }
        else {
            t_ast_node *node = parse_stmt(state);
            node_attach_next(block, node);
        }
    }
    return block;
}

static t_ast_node *parse_global_scope(t_lexstate *state) {
    t_ast_node *block = alloc_ast_node();
    block->type = AST_stmt_block;
    while(true) {
        if(token_match(state, TOKEN_EOF)) {
            break;
        }
        else {
            t_ast_node *node = parse_declaration(state);
            if(node == null) {
                parse_error(state, "unable to parse declaration");
                return block;
            }
            node_attach_next(block, node);
        }
    }
    return block;
}

static bool assert_token_type(t_token *token, t_token_kind kind) {
    if(token->kind != kind) {
        push_errorf("expected %s, found '%s'", get_token_kind_name(kind), get_token_string(token));
        return false;
    }
    return true;
}

static t_token token_eof = {.kind = TOKEN_EOF};
static t_token ast_expr_node_evaluate(t_ast_node *ast_node) {
    if(ast_node == null) {
        push_errorf("empty expression");
        return token_eof;
    }
    
    if(ast_node->type == AST_value_node) {
        return ast_node->value_token;
    }
    else if(ast_node->type == AST_unary_expr_node) {
        t_token value = ast_expr_node_evaluate(ast_node->unary_opr1);
        t_unary_op_cat op = ast_node->unary_op;
        if(value.kind == TOKEN_INT) {
            if(op == '-') {
                value.int_value = -value.int_value;
                return value;
            }
        }
        else {
            push_errorf("operation %s not permitted on token type %s",
                        get_token_kind_name(op), get_token_string(&value));
        }
    }
    else if(ast_node->type == AST_binary_expr_node) {
        t_binary_op_cat op = ast_node->binary_op;
        t_token result;
        t_token left = ast_expr_node_evaluate(ast_node->binary_opr1);
        t_token right = ast_expr_node_evaluate(ast_node->binary_opr2);
        
        if(left.kind == TOKEN_INT && right.kind == TOKEN_INT) {
            result.kind = TOKEN_INT;
            switch(op) {
                case '+': {
                    result.int_value = left.int_value + right.int_value;
                } break;
                case '-': {
                    result.int_value = left.int_value - right.int_value;
                } break;
                case '*': {
                    result.int_value = left.int_value * right.int_value;
                } break;
                case '/': {
                    if(right.int_value == 0) push_errorf("division by zero");
                    else result.int_value = left.int_value / right.int_value;
                } break;
            }
            return result;
        }
        else {
            push_errorf("operation %s not permitted on tokens %s, %s",
                        get_token_kind_name(op), 
                        get_token_string(&left),
                        get_token_string(&right));
        }
        return token_eof;
    }
    else {
        push_errorf("not an expression");
    }
    return token_eof;
}

static char const *get_unary_op_string(t_unary_op_cat cat) {
    switch(cat) {
        case '-': return "-";
        case '$': return "$";
        case '@': return "@";
    }
    return "<invalid unary operation>";
}

static char const *get_binary_op_string(t_binary_op_cat cat) {
    switch(cat) {
        case '=': return "=";
        case '+': return "+";
        case '-': return "-";
        case '*': return "*";
        case '/': return "/";
        case BINARY_add_ass: return "+=";
        case BINARY_sub_ass: return "-=";
        case BINARY_div_ass: return "*=";
        case BINARY_mul_ass: return "/=";
        case BINARY_and: return "and";
        case BINARY_or: return "or";
        case '<': return "<";
        case '>': return ">";
        case BINARY_leq: return "<=";
        case BINARY_geq: return ">=";
        case BINARY_eq: return "==";
        case BINARY_neq: return "!=";
        case BINARY_subscript: return "subscr";
    }
    return "<invalid binary operation>";
}

static char const *get_ternary_op_string(t_ternary_op_cat cat) {
    switch(cat) {
        case TERNARY_slice: return "slice";
    }
    return "<invalid ternary operation>";
}

static void ast_node_print_lisp(t_ast_node *ast_node, int level) {
    assert(ast_node != null);
    if(ast_node->type == AST_value_node) {
        if(ast_node->value_token.kind == TOKEN_INT) {
            printf("%llu", ast_node->value_token.int_value);
        }
        else if(ast_node->value_token.kind == TOKEN_IDN) {
            printf("%s", ast_node->value_token.str_value->str);
        }
        else if(ast_node->value_token.kind == TOKEN_STR) {
            printf("\"%s\"", ast_node->value_token.str_value->str);
        }
        else if(ast_node->value_token.kind == TOKEN_FLT) {
            printf("%f", ast_node->value_token.flt_value);
        }
    }
    if(ast_node->type == AST_unary_expr_node) {
        printf("(");
        printf("%s", get_unary_op_string(ast_node->unary_op));
        printf(" ");
        ast_node_print_lisp(ast_node->unary_opr1, 0);
        printf(")");
    }
    else if(ast_node->type == AST_binary_expr_node) {
        if(ast_node->binary_op == BINARY_subscript) {
            printf("(subscr ");
        }
        else {
            printf("(%s ", get_binary_op_string(ast_node->binary_op));
        }
        ast_node_print_lisp(ast_node->binary_opr1, 0);
        printf(" ");
        ast_node_print_lisp(ast_node->binary_opr2, 0);
        printf(")");
    }
    else if(ast_node->type == AST_ternary_expr_node) {
        printf("(%s ", get_ternary_op_string(ast_node->ternary_op));
        ast_node_print_lisp(ast_node->ternary_opr1, 0);
        printf(" ");
        if(ast_node->ternary_opr2 == null) {
            printf("start");
        }
        else ast_node_print_lisp(ast_node->ternary_opr2, 0);
        printf(" ");
        if(ast_node->ternary_opr3 == null) {
            printf("end");
        }
        else {
            ast_node_print_lisp(ast_node->ternary_opr3, 0);
        }
        printf(")");
    }
    else if(ast_node->type == AST_stmt_node) {
        switch(ast_node->stmt_cat) {
            case STMT_if: {
                printf("(if ");
                ast_node_print_lisp(ast_node->if_condition, level+1);
                ast_node_print_lisp(ast_node->if_true_block, level+1);
                if(ast_node->if_false_block != null) {
                    ast_node_print_lisp(ast_node->if_false_block, level+1);
                }
                printf("\n");
                print_level(level);
                printf(")");
            } break;
            case STMT_while: {
                printf("(while ");
                ast_node_print_lisp(ast_node->while_condition, level+1);
                ast_node_print_lisp(ast_node->while_block, level+1);
                printf("\n");
                print_level(level);
                printf(")");
            } break;
            case STMT_break: {
                printf("break");
            } break;
            case STMT_return: {
                printf("(return ");
                if(ast_node->stmt_value != null) {
                    printf("\n");
                    print_level(level+1);
                    ast_node_print_lisp(ast_node->stmt_value, level+1);
                    printf("\n");
                    print_level(level);
                }
                printf(")");
            } break;
            case STMT_continue: {
                printf("continue");
            } break;
            case STMT_print: {
                printf("(print ");
                printf("\n");
                print_level(level+1);
                ast_node_print_lisp(ast_node->stmt_value, level+1);
                printf("\n");
                print_level(level);
                printf(")");
            } break;
        }
    }
    else if(ast_node->type == AST_stmt_block) {
        printf("\n");
        print_level(level);
        printf("(compound");
        for(t_ast_node *node = ast_node->first;
            node != null;
            node = node->next) {
            printf("\n");
            print_level(level+1);
            ast_node_print_lisp(node, level + 1);
        }
        printf("\n");
        print_level(level);
        printf(")");
    }
    else if(ast_node->type == AST_decl_node) {
        printf("(decl");
        if(null != ast_node->decl_name) {
            printf(" %s ", ast_node->decl_name->str);
        }
        else printf(" NONAME ");
        ast_node_print_lisp(ast_node->decl_type, level+1);
        if(null != ast_node->decl_value) {
            printf(" ");
            ast_node_print_lisp(ast_node->decl_value, level+1);
        }
        printf(")");
    }
    else if(ast_node->type == AST_type_node) {
        switch(ast_node->type_cat) {
            case TYPE_primitive: {
                printf("%s", ast_node->primitive_typename->str);
            } break;
            case TYPE_pointer: {
                printf("(pointer to ");
                ast_node_print_lisp(ast_node->base_type, 0);
                printf(")");
            } break;
            case TYPE_slice: {
                printf("(slice of ");
                ast_node_print_lisp(ast_node->base_type, 0);
                printf(")");
            } break;
            case TYPE_function: {
                printf("(function returning ");
                ast_node_print_lisp(ast_node->function_return_type, level+1);
                for(t_ast_node *param = ast_node->function_parameters->first;
                    param != null;
                    param = param->next) {
                    printf("\n");
                    print_level(level+1);
                    printf("param: ");
                    ast_node_print_lisp(param, level+1);
                }
                printf("\n");
                print_level(level);
                printf(")");
            } break;
            default: {
                printf("(type ERROR");
            }
        }
    }
}

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
