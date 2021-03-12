
struct {
    char const *stream;
    t_token last_token;
    t_token_location loc;
} typedef t_lexstate;

static void lex_init(t_lexstate *state, char const *filename, char const *stream) {
    state->loc.filename = filename;
    state->loc.stream_start = stream;
    state->stream = stream;
    t_token eof_token; // to be removed later
    eof_token.start = eof_token.end = null;
    eof_token.kind = TOKEN_eof;
    state->last_token = eof_token;
}

static inline bool lex_match_char(t_lexstate *state, char c) {
    if(*state->stream == c) {
        state->stream += 1;
        return true;
    }
    return false;
}

static u64 lex_integer(u64 base, char const *stream, char const **end) {
    u64 value = 0;
    while(is_digit_in_base(base, *stream)) {
        value = base*value + char_to_digit[*stream];
        stream += 1;
    }
    *end = stream;
    return value;
}

static i64 lex_string_escape(char const *stream, char const **end) {
    i64 val;
    if(is_digit_in_base(16, *stream)) {
        i64 result = lex_integer(16, stream, &stream);
        val = (char)result;
    }
    else {
        val = (char)escape_char[*stream++];
    }
    *end = stream;
    return val;
}

static void lex_next_token(t_lexstate *state) {
    while(isspace(*state->stream)) {
        state->stream += 1;
    }
    
    char const *start = state->stream;
    state->last_token.flags = 0;
    state->last_token.loc = state->loc;
    state->last_token.loc.token_start = state->stream;
    
    // lex identifiers
    if(isalpha(*state->stream) || *state->stream == '_') {
        state->last_token.kind = TOKEN_idn;
        while(isalnum(*state->stream) || *state->stream == '_') {
            state->stream += 1;
        }
        char const *end = state->stream;
        t_intern const *string_value = intern_string(start, end);
        state->last_token.str_value = string_value;
        
        bool is_keyword = false;
        for(u64 keyword_index = 0; keyword_index < keywords_num; keyword_index += 1) {
            if(keywords[keyword_index] == string_value) {
                is_keyword = true;
            }
        }
        state->last_token.flags |= (is_keyword << FLAG_keyword);
    }
    
    // lex number literals
    else if(isdigit(*state->stream)) {
        
        // searching the '.' to find whether the number is flt
        while(isdigit(*state->stream)) state->stream+=1;
        if(*state->stream == '.') {
            state->stream = start;
            f64 val = strtod(start, (char **)&state->stream);
            state->last_token.flt_value = val;
            state->last_token.kind = TOKEN_flt;
        }
        else {
            state->stream = start;
            u64 base = 10;
            if(state->stream[0] == '0') {
                if(tolower(state->stream[1]) == 'x') base = 16;
                else if(tolower(state->stream[1] == 'o')) base = 8;
                else if(tolower(state->stream[1] == 'b')) base = 2;
            }
            state->last_token.int_value = lex_integer(base, state->stream, &state->stream);
            //state->last_token.subkind = SUBKIND_int;
            state->last_token.kind = TOKEN_int;
        }
    }
    else if(lex_match_char(state, '\'')) { // character literal
        i64 val;
        if(lex_match_char(state, '\\')) {
            val = lex_string_escape(state->stream, &state->stream);
            if(val > 0xff) {
                push_errorf(state->loc, "value %x is unacceptable for a char literal", val);
            }
        }
        else if(isprint(*state->stream)) {
            val = *state->stream;
            state->stream += 1;
        }
        else {
            push_errorf(state->loc, "value %x is unacceptable for a char literal", *state->stream);
        }
        state->last_token.kind = TOKEN_int;
        //state->last_token.subkind = SUBKIND_char;
        state->last_token.int_value = val;
        if(!lex_match_char(state, '\'')) {
            push_errorf(state->loc, "expected a char literal to close");
        }
    }
    else if(lex_match_char(state, '"')) { // string literal
        string_builder_start();
        while(true) {
            //char c = state_next_char(state);
            if(lex_match_char(state, '"')) break;
            else if(lex_match_char(state, '\\')) {
                i64 val = lex_string_escape(state->stream, &state->stream);
                if(val > 0xff) {
                    push_errorf(state->loc, "value %x is unacceptable for a hex escape", val);
                }
                string_builder_append_char((char)val);
            }
            else {
                string_builder_append_char(*state->stream++);
            }
        }
        char *result = string_builder_finish();
        t_intern const *intern = intern_string(result, result + string_builder.len);
        state->last_token.str_value = intern;
        state->last_token.kind = TOKEN_str;
    }
    else { // parse operators.
        
        if(lex_match_char(state, '=')) {
            state->last_token.kind = '=';
            if(lex_match_char(state, '=')) {
                state->last_token.kind = TOKEN_cmp_eq;
            }
        }
        else if(lex_match_char(state, '!')) {
            state->last_token.kind = '!';
            if(lex_match_char(state, '=')) {
                state->last_token.kind = TOKEN_cmp_neq;
            }
        }
        else if(lex_match_char(state, '>')) {
            state->last_token.kind = '>';
            if(lex_match_char(state, '=')) {
                state->last_token.kind = TOKEN_cmp_geq;
            }
        }
        else if(lex_match_char(state, '<')) {
            state->last_token.kind = '<';
            if(lex_match_char(state, '=')) {
                state->last_token.kind = TOKEN_cmp_leq;
            }
            if(lex_match_char(state, '-')) {
                state->last_token.kind = TOKEN_left_arrow;
            }
        }
        else if(lex_match_char(state, '+')) {
            state->last_token.kind = '+';
            if(lex_match_char(state, '=')) {
                state->last_token.kind = TOKEN_add_ass;
            }
        }
        else if(lex_match_char(state, '-')) {
            state->last_token.kind = '-';
            if(lex_match_char(state, '=')) {
                state->last_token.kind = TOKEN_sub_ass;
            }
        }
        else if(lex_match_char(state, '*')) {
            state->last_token.kind = '*';
            if(lex_match_char(state, '=')) {
                state->last_token.kind = TOKEN_mul_ass;
            }
        }
        else if(lex_match_char(state, '/')) {
            state->last_token.kind = '/';
            if(lex_match_char(state, '=')) {
                state->last_token.kind = TOKEN_div_ass;
            }
        }
        else if(*state->stream == 0) {
            state->last_token.kind = TOKEN_eof;
        }
        else {
            state->last_token.kind = *state->stream;
            state->stream += 1;
        }
    }
    char const *end = state->stream;
    state->last_token.start = start;
    state->last_token.end = end;
}

static inline bool token_is_keyword(t_token *token) {
    return (token->flags & (1<<FLAG_keyword)) != 0;
}

static inline bool token_is_kind(t_lexstate *state, t_token_kind kind) {
    return state->last_token.kind == kind;
}

static inline bool token_match_kind(t_lexstate *state, t_token_kind kind) {
    if(state->last_token.kind == kind) {
        lex_next_token(state);
        return true;
    }
    return false;
}

static inline bool token_expect_kind(t_lexstate *state, t_token_kind kind) {
    if(state->last_token.kind == kind) {
        lex_next_token(state);
        return true;
    }
    push_errorf(state->last_token.loc, "expected token %s, got %s",
                get_token_kind_name(kind),
                get_token_string(&state->last_token));
    return false;
}

static inline bool token_expect_peek_kind(t_lexstate *state, t_token_kind kind) {
    if(state->last_token.kind == kind) {
        return true;
    }
    push_errorf(state->last_token.loc, "expected token %s, got %s",
                get_token_kind_name(kind),
                get_token_string(&state->last_token));
    return false;
}


static inline bool token_is_identifier(t_lexstate *state, t_intern const *str) {
    if(state->last_token.kind == TOKEN_idn) {
        return state->last_token.str_value == str;
    }
    return false;
}

static inline bool token_match_identifier(t_lexstate *state, t_intern const *str) {
    if(state->last_token.kind == TOKEN_idn && state->last_token.str_value == str) {
        lex_next_token(state);
        return true;
    }
    return false;
}

static inline bool token_expect_identifier(t_lexstate *state, t_intern const *str) {
    if(state->last_token.kind == TOKEN_idn && state->last_token.str_value == str) {
        lex_next_token(state);
        return true;
    }
    push_errorf(state->last_token.loc, "expected keyword %s, got %s", state->last_token.str_value->str, str->str);
    return false;
}

