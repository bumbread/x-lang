
static inline bool unexpected_last_token(t_lexstate *state) {
    push_errorf(state->last_token.loc, "unexpected token %s.", get_token_string(&state->last_token));
    return false;
}

static inline void parse_error(t_lexstate *state, char const *string) {
    push_errorf(state->last_token.loc, "%s", string);
}

//--------------------||
// EXPRESSION PARSING ||
//--------------------||

static t_expr_data *parse_expr(t_lexstate *state);

static t_expr_data *parse_expr_value(t_lexstate *state) {
    t_token_location expr_start = state->last_token.loc;
    t_expr_data *node;
    if(token_match_kind(state, '(')) {
        node = parse_expr(state);
        if(node == null) {
            return null;
        }
        if(!token_expect_kind(state, ')')) {
            return null;
        }
    }
    else if(token_is_kind(state, TOKEN_str)) {
        node = make_static_value(expr_start);
        node->cat = EXPR_value;
        node->value.cat = VALUE_string;
        node->value.s = state->last_token.str_value;
        lex_next_token(state);
    }
    else if(token_is_kind(state, TOKEN_int)) {
        node = make_static_value(expr_start);
        node->cat = EXPR_value;
        node->value.cat = VALUE_int;
        node->value.i = state->last_token.int_value;
        lex_next_token(state);
    }
    else if(token_is_kind(state, TOKEN_flt)) {
        node = make_static_value(expr_start);
        node->cat = EXPR_value;
        node->value.cat = VALUE_float;
        node->value.f = state->last_token.flt_value;
        lex_next_token(state);
    }
    else if(token_is_kind(state, TOKEN_idn)) {
        t_intern const *identifier_name = state->last_token.str_value;
        assert(identifier_name != null);
        node = make_identifier_expr(identifier_name, expr_start);
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
            if(!token_is_kind(state, ')') && !token_match_kind(state, ',')) {
                parse_error(state, "expected either , or )");
                return null;
            }
        }
        if(!token_expect_kind(state, ')')) {
            return null;
        }
        node = make_function_call(node, parameter_list, expr_start);
    }
    return node;
}

static t_expr_data *parse_expr_term(t_lexstate *state) {
    t_token_location expr_start = state->last_token.loc;
    
    // sub
    if(token_match_kind(state, '-')) {
        t_expr_data *expr = parse_expr_term(state);
        if(expr == null) return null;
        return make_unary_expr(UNARY_sub, expr, expr_start);
    }
    
    // address-of
    else if(token_match_kind(state, '$')) {
        t_expr_data *expr = parse_expr_term(state);
        if(expr == null) return null;
        return make_unary_expr(UNARY_addr, expr, expr_start);
    }
    
    // deref
    else if(token_match_kind(state, '@')) {
        t_expr_data *expr = parse_expr_term(state);
        if(expr == null) return null;
        return make_unary_expr(UNARY_deref, expr, expr_start);
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
                    node = make_ternary_expr(TERNARY_slice, result, first_index, second_index, expr_start);
                    token_expect_kind(state, ']');
                }
                
                // array access
                else {
                    node = make_binary_expr(BINARY_subscript, result, first_index, expr_start);
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
    if(lhs == null) return null;
    while(true) {
        
        t_token_location sign_start = state->last_token.loc;
        if(token_is_kind(state, '*')) {
            lex_next_token(state);
            t_expr_data *rhs = parse_expr_term(state);
            if(rhs == null) return null;
            lhs = make_binary_expr(BINARY_mul, lhs, rhs, sign_start);
        }
        
        else if(token_is_kind(state, '/')) {
            lex_next_token(state);
            t_expr_data *rhs = parse_expr_term(state);
            if(rhs == null) return null;
            lhs= make_binary_expr(BINARY_div, lhs, rhs, sign_start);
        }
        else break;
    }
    return lhs;
}

static t_expr_data *parse_expr_arithmetic(t_lexstate *state) {
    t_expr_data *lhs = parse_expr_factor(state);
    if(lhs != null) {
        while(true) {
            t_token_location sign_start = state->last_token.loc;
            
            if(token_is_kind(state, '+')) {
                lex_next_token(state);
                t_expr_data *rhs = parse_expr_factor(state);
                if(rhs == null) return null;
                lhs = make_binary_expr(BINARY_add, lhs, rhs, sign_start);
            }
            
            else if(token_is_kind(state, '-')) {
                lex_next_token(state);
                t_expr_data *rhs = parse_expr_factor(state);
                if(rhs == null) return null;
                lhs = make_binary_expr(BINARY_sub, lhs, rhs, sign_start);
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
        t_token_location sign_start = state->last_token.loc;
        
        if(token_is_kind(state, '<')) {
            lex_next_token(state);
            t_expr_data *rhs = parse_expr_arithmetic(state);
            if(rhs == null) return null;
            lhs = make_binary_expr(BINARY_less, lhs, rhs, sign_start);
        }
        
        else if(token_is_kind(state, '>')) {
            lex_next_token(state);
            t_expr_data *rhs = parse_expr_arithmetic(state);
            if(rhs == null) return null;
            lhs = make_binary_expr(BINARY_greater, lhs, rhs, sign_start);
        }
        
        else if(token_is_kind(state, TOKEN_cmp_leq)) {
            lex_next_token(state);
            t_expr_data *rhs = parse_expr_arithmetic(state);
            if(rhs == null) return null;
            lhs = make_binary_expr(BINARY_leq, lhs, rhs, sign_start);
        }
        
        else if(token_is_kind(state, TOKEN_cmp_geq)) {
            lex_next_token(state);
            t_expr_data *rhs = parse_expr_arithmetic(state);
            if(rhs == null) return null;
            lhs = make_binary_expr(BINARY_geq, lhs, rhs, sign_start);
        }
        
        else if(token_is_kind(state, TOKEN_cmp_eq)) {
            lex_next_token(state);
            t_expr_data *rhs = parse_expr_arithmetic(state);
            if(rhs == null) return null;
            lhs = make_binary_expr(BINARY_eq, lhs, rhs, sign_start);
        }
        
        else if(token_is_kind(state, TOKEN_cmp_neq)) {
            lex_next_token(state);
            t_expr_data *rhs = parse_expr_arithmetic(state);
            if(rhs == null) return null;
            lhs = make_binary_expr(BINARY_neq, lhs, rhs, sign_start);
        }
        else break;
    }
    return lhs;
}

static t_expr_data *parse_expr_and_level(t_lexstate *state) {
    t_expr_data *lhs = parse_expr_relational(state);
    if(lhs == null) return null;
    
    while(token_is_identifier(state, keyword_and)) {
        t_token_location and_op_location = state->last_token.loc;
        lex_next_token(state);
        t_expr_data *rhs = parse_expr_relational(state);
        if(rhs == null) return null;
        lhs = make_binary_expr(BINARY_and, lhs, rhs, and_op_location);
    }
    return lhs;
}

static t_expr_data *parse_expr_or_level(t_lexstate *state) {
    t_expr_data *lhs = parse_expr_and_level(state);
    if(lhs == null) return null;
    while(token_is_identifier(state, keyword_or)) {
        t_token_location or_op_location = state->last_token.loc;
        lex_next_token(state);
        t_expr_data *rhs = parse_expr_and_level(state);
        if(rhs == null) return null;
        lhs = make_binary_expr(BINARY_or, lhs, rhs, or_op_location);
    }
    return lhs;
}

static t_expr_data *parse_expr(t_lexstate *state) {
    return parse_expr_or_level(state);
}

/* STATEMENTS PARSER */


static t_stmt_data *parse_expr_stmt(t_lexstate *state) {
    t_token_location stmt_start = state->last_token.loc;
    
    t_expr_data *expr = parse_expr(state);
    if(expr == null) return null;
    
    t_token_location op_loc = state->last_token.loc;
    if(token_is_kind(state, TOKEN_add_ass)) {
        lex_next_token(state);
        t_expr_data *rhs = parse_expr(state);
        if(rhs == null) return null;
        expr = make_binary_expr(BINARY_add_ass, expr, rhs, op_loc);
    }
    
    else if(token_is_kind(state, TOKEN_sub_ass)) {
        lex_next_token(state);
        t_expr_data *rhs = parse_expr(state);
        if(rhs == null) return null;
        expr = make_binary_expr(BINARY_sub_ass, expr, rhs, op_loc);
    }
    
    else if(token_is_kind(state, TOKEN_mul_ass)) {
        lex_next_token(state);
        t_expr_data *rhs = parse_expr(state);
        if(rhs == null) return null;
        expr = make_binary_expr(BINARY_mul_ass, expr, rhs, op_loc);
    }
    
    else if(token_is_kind(state, TOKEN_div_ass)) {
        lex_next_token(state);
        t_expr_data *rhs = parse_expr(state);
        if(rhs == null) return null;
        expr = make_binary_expr(BINARY_div_ass, expr, rhs, op_loc);
    }
    
    else if(token_is_kind(state, '=')) {
        lex_next_token(state);
        t_expr_data *rhs = parse_expr(state);
        if(rhs == null) return null;
        expr = make_binary_expr(BINARY_ass, expr, rhs, op_loc);
    }
    
    return make_expression_stmt(expr, stmt_start);
}


static t_stmt_list *parse_stmt_list(t_lexstate *state);
static t_stmt_data *parse_stmt_block(t_lexstate *state);
static t_stmt_data *parse_stmt(t_lexstate *state);
static t_decl_data *parse_declaration(t_lexstate *state);

static t_stmt_data *parse_if_stmt(t_lexstate *state) {
    t_token_location statement_start = state->last_token.loc;
    
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
            if(false_branch == null) return null;
        }
        else {
            false_branch = parse_stmt(state);
            if(false_branch == null) return null;
        }
    }
    
    if(condition == null) return null;
    if(true_branch == null) return null;
    
    return make_if_stmt(condition, true_branch, false_branch, statement_start);
}

static t_stmt_data *parse_while_stmt(t_lexstate *state) {
    t_token_location statement_start = state->last_token.loc;
    
    token_expect_identifier(state, keyword_while);
    t_expr_data *condition = parse_expr(state);
    t_stmt_data *block = null;
    
    if(!token_is_kind(state, ';')) {
        block = parse_stmt_block(state);
    }
    
    if(condition == null) return null;
    if(block == null) return null;
    
    return make_while_stmt(condition, block, statement_start);
}

static t_type_data *parse_type(t_lexstate *state) {
    t_type_data *type = null;
    
    if(token_expect_peek_kind(state, TOKEN_idn)) {
        if(state->last_token.str_value == keyword_int) {
            type = make_int_type();
        }
        else if(state->last_token.str_value == keyword_bool) {
            type = make_bool_type();
        }
        else if(state->last_token.str_value == keyword_string) {
            type = make_string_type();
        }
        else if(state->last_token.str_value == keyword_float) {
            type = make_float_type();
        }
        else if(state->last_token.str_value == keyword_byte) {
            type = make_byte_type();
        }
        else {
            parse_error(state, "aliase types are not supported");
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
            while(!token_match_kind(state, ')')) {
                t_decl_data *param_decl = parse_declaration(state);
                if(param_decl == null) return null;
                decl_list_push(decl_list, param_decl);
                if(!token_match_kind(state, ',') && !token_is_kind(state, ')')) {
                    parse_error(state, "expected either , or )");
                    return null;
                }
            }
            type = make_type_function(type, decl_list);
        }
        // end
        else break;
    }
    
    return type;
}

static t_decl_data *parse_declaration(t_lexstate *state) {
    t_type_data *type = parse_type(state);
    t_intern const *name;
    
    if(token_expect_peek_kind(state, TOKEN_idn)) {
        if(token_is_keyword(&state->last_token)) {
            push_errorf(state->last_token.loc, "keywords are not allowed as variable name");
            return null;
        }
        name = state->last_token.str_value;
        lex_next_token(state);
    }
    
    if(type == null) return null;
    
    // TODO(bumbread): if function push to all procs list.
    if(token_match_kind(state, '=')) {
        t_expr_data *expr = parse_expr(state);
        if(expr == null) return null;
        return make_decl_expr_value(name, type, expr);
    }
    else if(token_is_kind(state, '{')) {
        t_stmt_list *stmt_list = parse_stmt_list(state);
        if(stmt_list == null) return null;
        return make_decl_block_value(name, type, stmt_list);
    }
    else {
        return make_decl_no_value(name, type);
    }
}

static t_stmt_data *parse_declaration_stmt(t_lexstate *state) {
    t_token_location stmt_start = state->last_token.loc;
    token_expect_kind(state, ':');
    t_decl_data *decl_data = parse_declaration(state);
    if(decl_data == null) return null;
    if(decl_data->cat == DECL_expr_value || decl_data->cat == DECL_no_value) {
        if(!token_expect_kind(state, ';')) return null;
    }
    else token_match_kind(state, ';');
    t_stmt_data *decl = make_decl_stmt(decl_data, stmt_start);
    return decl;
}

static t_stmt_data *parse_return_stmt(t_lexstate *state) {
    t_token_location stmt_start = state->last_token.loc;
    token_match_identifier(state, keyword_return);
    t_expr_data *expr = null;
    if(!token_match_kind(state, ';')) {
        expr = parse_expr(state);
        if(expr == null) return null;
        if(!token_match_kind(state, ';')) {
            return null;
        }
    }
    return make_return_stmt(expr, stmt_start);
}

static t_stmt_data *parse_print_stmt(t_lexstate *state) {
    t_token_location stmt_start = state->last_token.loc;
    token_match_identifier(state, keyword_print);
    t_expr_data *expr = parse_expr(state);
    if(expr == null) return null;
    if(!token_expect_kind(state, ';')) return null;
    return make_print_stmt(expr, stmt_start);
}

static t_stmt_data *parse_break_stmt(t_lexstate *state) {
    t_token_location stmt_start = state->last_token.loc;
    token_expect_identifier(state, keyword_break);
    if(!token_expect_kind(state, ';')) return null;
    return make_stmt_cat(STMT_break, stmt_start);
}

static t_stmt_data *parse_continue_stmt(t_lexstate *state) {
    t_token_location stmt_start = state->last_token.loc;
    token_expect_identifier(state, keyword_continue);
    if(!token_expect_kind(state, ';')) return null;
    return make_stmt_cat(STMT_continue, stmt_start);
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
        else {
            while(!token_match_kind(state, ';')) {
                lex_next_token(state);
                if(token_is_kind(state, TOKEN_eof)) {
                    break;
                }
            }
        }
    }
    if(!token_expect_kind(state, '}')) {
        return null;
    }
    return list;
}

static t_stmt_data *parse_stmt_block(t_lexstate *state) {
    t_token_location block_start = state->last_token.loc;
    if(!token_expect_kind(state, '{')) return null;
    
    t_stmt_data *block = make_stmt_block(block_start);
    while(!token_is_kind(state, '}')) {
        t_stmt_data *stmt = parse_stmt(state);
        if(stmt != null) {
            block_append_stmt(block, stmt);
        }
        else {
            while(!token_match_kind(state, ';')) {
                lex_next_token(state);
                if(token_is_kind(state, TOKEN_eof)) {
                    break;
                }
            }
        }
    }
    if(!token_expect_kind(state, '}')) {
        return null;
    }
    return block;
}

static t_stmt_list *parse_global_scope(t_lexstate *state) {
    t_stmt_list *stmts = alloc_stmt_list();
    while(!token_is_kind(state, TOKEN_eof)) {
        t_stmt_data *stmt = parse_stmt(state);
        if(stmt == null) {
            parse_error(state, "unable to parse stmt");
        }
        else if(stmt->cat == STMT_decl){
            stmt_list_push(stmts, stmt);
        }
        else {
            parse_error(state, "only declarations are allowed in top-level code");
        }
    }
    return stmts;
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
static t_stmt_list *parse_ast_node_global_level(char const *code_str) {
    t_lexstate state;
    lex_init(&state, "@test", code_str);
    lex_next_token(&state);
    t_stmt_list *code = parse_global_scope(&state);
    print_error_buffer();
    return code;
}
