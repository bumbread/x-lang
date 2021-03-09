
static inline bool unexpected_last_token(t_lexstate *state) {
    push_errorf("%s(%d, %d): unexpected token %s.",
                state->filename,
                state->last_token.loc.line, state->last_token.loc.offset,
                get_token_string(&state->last_token));
    return false;
}

static inline void parse_error(char const *string, t_location loc) {
    push_errorf("%s(%d, %d): %s",
                loc.filename,
                loc.line, loc.offset,
                string);
}

//--------------------||
// EXPRESSION PARSING ||
//--------------------||

static t_expr_data *parse_expr(t_lexstate *state);

static t_expr_data *parse_expr_value(t_lexstate *state) {
    t_expr_data *node;
    if(token_match_kind(state, '(')) {
        node = parse_expr(state);
        token_expect_kind(state, ')');
    }
    else if(state->last_token.kind == TOKEN_str) {
        node = make_static_value();
        node->cat = EXPR_value;
        node->value.cat = VALUE_string;
        node->value.s = state->last_token.str_value;
        lex_next_token(state);
    }
    else if(state->last_token.kind == TOKEN_int) {
        node = make_static_value();
        node->cat = EXPR_value;
        node->value.cat = VALUE_int;
        node->value.i = state->last_token.int_value;
        lex_next_token(state);
    }
    else if(state->last_token.kind == TOKEN_flt) {
        node = make_static_value();
        node->cat = EXPR_value;
        node->value.cat = VALUE_float;
        node->value.f = state->last_token.flt_value;
        lex_next_token(state);
    }
    else if(state->last_token.kind == TOKEN_idn) {
        node = make_identifier_expr(state->last_token.str_value);
        lex_next_token(state);
    }
    else {
        unexpected_last_token(state);
        return null;
    }
    
    if(token_match_kind(state, '(')) {
        t_expr_list *parameter_list = alloc_expr_list();
        while(!token_is_kind(state, ')')) {
            t_expr_data *expr = parse_expr(state);
            if(expr == null) return null;
            expr_list_push(parameter_list, expr);
            if(!token_is_kind(state, ')')) {
                token_match_kind(state, ',');
            }
        }
        token_expect_kind(state, ')');
        node = make_function_call(node, parameter_list);
    }
    return node;
}

static t_expr_data *parse_expr_term(t_lexstate *state) {
    // sub
    if(token_match_kind(state, '-')) {
        t_expr_data *expr = parse_expr_term(state);
        if(expr == null) return null;
        return make_unary_expr(UNARY_sub, expr);
    }
    
    // address-of
    else if(token_match_kind(state, '$')) {
        t_expr_data *expr = parse_expr_term(state);
        if(expr == null) return null;
        return make_unary_expr(UNARY_addr, expr);
    }
    
    // deref
    else if(token_match_kind(state, '@')) {
        t_expr_data *expr = parse_expr_term(state);
        if(expr == null) return null;
        return make_unary_expr(UNARY_deref, expr);
    }
    
    else {
        t_expr_data *result = parse_expr_value(state);
        if(result == null) {
            return null;
        }
        while(true) {
            if(token_match_kind(state, '[')) {
                t_expr_data *first_index = null;
                if(!token_is_kind(state, ':')) {
                    first_index = parse_expr(state);
                    if(first_index == null) return null;
                }
                t_expr_data *node;
                
                // slicing operator
                if(token_match_kind(state, ':')) {
                    t_expr_data *second_index = null;
                    if(!token_is_kind(state, ']')) {
                        second_index = parse_expr(state);
                        if(second_index == null) return null;
                    }
                    node = make_ternary_expr(TERNARY_slice, result, first_index, second_index);
                    token_expect_kind(state, ']');
                }
                
                // array access
                else {
                    node = make_binary_expr(BINARY_subscript, result, first_index);
                    token_expect_kind(state, ']');
                }
                result = node;
            }
            else break;
        }
        return result;
    }
}

static t_expr_data *parse_expr_factor(t_lexstate *state) {
    t_expr_data *lhs = parse_expr_term(state);
    if(null == lhs) return null;
    while(true) {
        
        if(token_is_kind(state, '*')) {
            lex_next_token(state);
            t_expr_data *rhs = parse_expr_term(state);
            if(rhs == null) return null;
            lhs = make_binary_expr(BINARY_mul, lhs, rhs);
        }
        
        else if(token_is_kind(state, '/')) {
            lex_next_token(state);
            t_expr_data *rhs = parse_expr_term(state);
            if(rhs == null) return null;
            lhs= make_binary_expr(BINARY_div, lhs, rhs);
        }
        else break;
    }
    return lhs;
}

static t_expr_data *parse_expr_arithmetic(t_lexstate *state) {
    t_expr_data *lhs = parse_expr_factor(state);
    if(lhs != null) {
        while(true) {
            
            if(token_is_kind(state, '+')) {
                lex_next_token(state);
                t_expr_data *rhs = parse_expr_factor(state);
                if(rhs == null) return null;
                lhs = make_binary_expr(BINARY_add, lhs, rhs);
            }
            
            else if(token_is_kind(state, '-')) {
                lex_next_token(state);
                t_expr_data *rhs = parse_expr_factor(state);
                if(rhs == null) return null;
                lhs = make_binary_expr(BINARY_sub, lhs, rhs);
            }
            else break;
        }
    }
    return lhs;
}

// TODO(bumbread): should there be a while loop?
static t_expr_data *parse_expr_relational(t_lexstate *state) {
    t_expr_data *lhs = parse_expr_arithmetic(state);
    if(lhs == null) return null;
    while(true) {
        
        if(token_is_kind(state, '<')) {
            lex_next_token(state);
            t_expr_data *rhs = parse_expr_arithmetic(state);
            if(rhs == null) return null;
            lhs = make_binary_expr(BINARY_less, lhs, rhs);
        }
        
        else if(token_is_kind(state, '>')) {
            lex_next_token(state);
            t_expr_data *rhs = parse_expr_arithmetic(state);
            if(rhs == null) return null;
            lhs = make_binary_expr(BINARY_greater, lhs, rhs);
        }
        
        else if(token_is_kind(state, TOKEN_cmp_leq)) {
            lex_next_token(state);
            t_expr_data *rhs = parse_expr_arithmetic(state);
            if(rhs == null) return null;
            lhs = make_binary_expr(BINARY_leq, lhs, rhs);
        }
        
        else if(token_is_kind(state, TOKEN_cmp_geq)) {
            lex_next_token(state);
            t_expr_data *rhs = parse_expr_arithmetic(state);
            if(rhs == null) return null;
            lhs = make_binary_expr(BINARY_geq, lhs, rhs);
        }
        
        else if(token_is_kind(state, TOKEN_cmp_eq)) {
            lex_next_token(state);
            t_expr_data *rhs = parse_expr_arithmetic(state);
            if(rhs == null) return null;
            lhs = make_binary_expr(BINARY_eq, lhs, rhs);
        }
        
        else if(token_is_kind(state, TOKEN_cmp_neq)) {
            lex_next_token(state);
            t_expr_data *rhs = parse_expr_arithmetic(state);
            if(rhs == null) return null;
            lhs = make_binary_expr(BINARY_neq, lhs, rhs);
        }
        else break;
    }
    return lhs;
}

static t_expr_data *parse_expr_and_level(t_lexstate *state) {
    t_expr_data *lhs = parse_expr_relational(state);
    if(lhs == null) return null;
    while(token_is_identifier(state, keyword_and)) {
        lex_next_token(state);
        t_expr_data *rhs = parse_expr_relational(state);
        if(rhs == null) return null;
        lhs = make_binary_expr(BINARY_and, lhs, rhs);
    }
    return lhs;
}

static t_expr_data *parse_expr_or_level(t_lexstate *state) {
    t_expr_data *lhs = parse_expr_and_level(state);
    if(lhs == null) return null;
    while(token_is_identifier(state, keyword_or)) {
        lex_next_token(state);
        t_expr_data *rhs = parse_expr_and_level(state);
        if(rhs == null) return null;
        lhs = make_binary_expr(BINARY_or, lhs, rhs);
    }
    return lhs;
}

static t_expr_data *parse_expr(t_lexstate *state) {
    return parse_expr_or_level(state);
}

/* STATEMENTS PARSER */


static t_stmt_data *parse_expr_stmt(t_lexstate *state) {
    t_expr_data *expr = parse_expr(state);
    if(expr == null) return null;
    
    if(token_is_kind(state, TOKEN_add_ass)) {
        lex_next_token(state);
        t_expr_data *rhs = parse_expr(state);
        expr = make_binary_expr(BINARY_add_ass, expr, rhs);
    }
    
    else if(token_is_kind(state, TOKEN_sub_ass)) {
        lex_next_token(state);
        t_expr_data *rhs = parse_expr(state);
        expr = make_binary_expr(BINARY_sub_ass, expr, rhs);
    }
    
    else if(token_is_kind(state, TOKEN_mul_ass)) {
        lex_next_token(state);
        t_expr_data *rhs = parse_expr(state);
        expr = make_binary_expr(BINARY_mul_ass, expr, rhs);
    }
    
    else if(token_is_kind(state, TOKEN_div_ass)) {
        lex_next_token(state);
        t_expr_data *rhs = parse_expr(state);
        expr = make_binary_expr(BINARY_div_ass, expr, rhs);
    }
    
    else if(token_is_kind(state, '=')) {
        lex_next_token(state);
        t_expr_data *rhs = parse_expr(state);
        expr = make_binary_expr(BINARY_ass, expr, rhs);
    }
    
    return make_expression_stmt(expr);
}


static t_stmt_list *parse_stmt_list(t_lexstate *state);
static t_stmt_data *parse_stmt_block(t_lexstate *state);
static t_stmt_data *parse_stmt(t_lexstate *state);
static t_decl_data *parse_declaration(t_lexstate *state);

static t_stmt_data *parse_if_stmt(t_lexstate *state) {
    token_expect_identifier(state, keyword_if);
    t_expr_data *condition = parse_expr(state);
    t_stmt_data *true_branch = null;
    t_stmt_data *false_branch = null;
    
    // TODO(bumbread): do syntax for inline true branch stmt?
    if(!token_is_kind(state, ';')) {
        true_branch = parse_stmt_block(state);
    }
    if(token_match_identifier(state, keyword_else)) {
        if(token_is_kind(state, '{')) {
            false_branch = parse_stmt_block(state);
        }
        else {
            false_branch = parse_stmt(state);
            token_expect_kind(state, ';');
        }
    }
    
    return make_if_stmt(condition, true_branch, false_branch);
}

static t_stmt_data *parse_while_stmt(t_lexstate *state) {
    token_expect_identifier(state, keyword_while);
    t_expr_data *condition = parse_expr(state);
    t_stmt_data *block = null;
    
    if(!token_is_kind(state, ';')) {
        block = parse_stmt_block(state);
    }
    
    return make_while_stmt(condition, block);
}

static t_type_data *parse_type(t_lexstate *state) {
    
    t_type_data *type = null;
    
    if(token_expect_peek_kind(state, TOKEN_idn)) {
        if(state->last_token.str_value == keyword_int) {
            type = &type_int;
        }
        else if(state->last_token.str_value == keyword_bool) {
            type = &type_bool;
        }
        else if(state->last_token.str_value == keyword_string) {
            type = &type_string;
        }
        else if(state->last_token.str_value == keyword_float) {
            type = &type_float;
        }
        else if(state->last_token.str_value == keyword_byte) {
            type = &type_byte;
        }
        else {
            parse_error("aliase types are not supported", state->loc);
            return null;
        }
        lex_next_token(state);
    }
    else {
        return null;
    }
    
    while(true) {
        // pointer to
        if(token_match_kind(state, '$')) {
            type = make_type_pointer_to(type);
        }
        // slice of
        else if(token_match_kind(state, '[')) {
            if(!token_expect_kind(state, ']')) {
                printf("  note: x-lang doesn't have constant array types.");
                return null;
            }
            type = make_type_slice_of(type);
        }
        // function returning
        else if(token_match_kind(state, TOKEN_left_arrow)) {
            if(!token_expect_kind(state, '(')) {
                return null;
            }
            t_decl_list *decl_list = alloc_decl_list();
            while(!token_is_kind(state, ')')) {
                t_decl_data *param_decl = parse_declaration(state);
                decl_list_push(decl_list, param_decl);
                if(!token_match_kind(state, ',') && !token_is_kind(state, ')')) {
                    parse_error("expected either , or )", state->loc);
                    return null;
                }
            }
            if(!token_expect_kind(state, ')')) {
                return null;
            }
            type = make_type_function(type, decl_list);
        }
        // end
        else {
            break;
        }
    }
    
    return type;
}

static t_decl_data *parse_declaration(t_lexstate *state) {
    t_type_data *type = parse_type(state);
    t_intern const *name;
    
    if(token_expect_peek_kind(state, TOKEN_idn)) {
        // TODO(bumbread): maybe this should be handled at the cheker level?
        if(token_is_keyword(&state->last_token)) {
            push_errorf("keywords are not allowed as variable name", state->loc);
        }
        name = state->last_token.str_value;
        lex_next_token(state);
    }
    
    if(token_match_kind(state, '=')) {
        return make_decl_expr_value(name, type, parse_expr(state));
    }
    else if(token_is_kind(state, '{')) {
        return make_decl_block_value(name, type, parse_stmt_list(state));
    }
    else {
        return make_decl_no_value(name, type);
    }
    
    // TODO(bumbread): if function push to all procs list.
    return null;
}

static t_stmt_data *parse_declaration_stmt(t_lexstate *state) {
    token_expect_kind(state, ':');
    t_decl_data *decl_data = parse_declaration(state);
    t_stmt_data *decl = make_decl_stmt(decl_data);
    return decl;
}

static t_stmt_data *parse_return_stmt(t_lexstate *state) {
    token_match_identifier(state, keyword_return);
    t_expr_data *expr = null;
    if(!token_match_kind(state, ';')) {
        parse_expr(state);
    }
    return make_return_stmt(expr);
}

static t_stmt_data *parse_print_stmt(t_lexstate *state) {
    token_match_identifier(state, keyword_print);
    t_expr_data *expr = parse_expr(state);
    token_expect_kind(state, ';');
    return make_print_stmt(expr);
}

static t_stmt_data *parse_break_stmt(t_lexstate *state) {
    token_expect_identifier(state, keyword_break);
    token_expect_kind(state, ';');
    return make_stmt_cat(STMT_break);
}

static t_stmt_data *parse_continue_stmt(t_lexstate *state) {
    token_expect_identifier(state, keyword_continue);
    token_expect_kind(state, ';');
    return make_stmt_cat(STMT_continue);
}

static t_stmt_data *parse_stmt(t_lexstate *state) {
    if(token_is_identifier(state, keyword_if))            return parse_if_stmt(state);
    else if(token_is_identifier(state, keyword_while))    return parse_while_stmt(state);
    else if(token_is_kind(state, ':'))                    return parse_declaration_stmt(state);
    else if(token_is_identifier(state, keyword_return))   return parse_return_stmt(state);
    else if(token_is_identifier(state, keyword_break))    return parse_break_stmt(state);
    else if(token_is_identifier(state, keyword_continue)) return parse_continue_stmt(state);
    else if(token_is_identifier(state, keyword_print))    return parse_print_stmt(state);
    else if(token_match_kind(state, ';'))                 return null;
    else if(token_is_kind(state, '{'))                    return parse_stmt_block(state);
    else if(!state->last_token.kind == TOKEN_eof)         return parse_expr_stmt(state);
    return null;
}

static t_stmt_list *parse_stmt_list(t_lexstate *state) {
    if(!token_expect_kind(state, '{')) return null;
    
    t_stmt_list *list = alloc_stmt_list();
    while(!token_is_kind(state, '}')) {
        t_stmt_data *stmt = parse_stmt(state);
        if(stmt != null) {
            stmt_list_push(list, stmt);
        }
    }
    if(!token_expect_kind(state, '}')) {
        return null;
    }
    return list;
}

static t_stmt_data *parse_stmt_block(t_lexstate *state) {
    if(!token_expect_kind(state, '{')) return null;
    
    t_stmt_data *block = make_stmt_block();
    while(!token_is_kind(state, '}')) {
        t_stmt_data *stmt = parse_stmt(state);
        if(stmt != null) {
            block_append_stmt(block, stmt);
        }
    }
    if(!token_expect_kind(state, '}')) {
        return null;
    }
    return block;
}

static t_decl_list *parse_global_scope(t_lexstate *state) {
    t_decl_list *decls = alloc_decl_list();
    while(!token_is_kind(state, TOKEN_eof)) {
        token_expect_kind(state, ':');
        t_decl_data *decl = parse_declaration(state);
        if(decl == null) {
            parse_error("unable to parse declaration", state->loc);
            return decls;
        }
        decl_list_push(decls, decl);
    }
    return decls;
}

// debug functions
static t_expr_data *DEBUG_parse_ast_node_expr_level(char const *expr) {
    t_lexstate state;
    lex_init(&state, "@test", expr);
    lex_next_token(&state);
    t_expr_data *code = parse_expr(&state);
    print_error_buffer();
    return code;
}
static t_stmt_data *DEBUG_parse_ast_node_stmt_level(char const *stmt) {
    t_lexstate state;
    lex_init(&state, "@test", stmt);
    lex_next_token(&state);
    t_stmt_data *code = parse_stmt(&state);
    print_error_buffer();
    return code;
}
static t_decl_list *parse_ast_node_global_level(char const *code_str) {
    t_lexstate state;
    lex_init(&state, "@test", code_str);
    lex_next_token(&state);
    t_decl_list *code = parse_global_scope(&state);
    print_error_buffer();
    return code;
}
