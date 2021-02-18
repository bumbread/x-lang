
enum {
    TOKEN_EOF = 0,   // end of file
    // All ascii characters are a separate token
    // 0..127
    
    TOKEN_INT = 128, // integer numbers
    TOKEN_IDN,
    TOKEN_CMP_NEQ,
    TOKEN_CMP_EQ,
    TOKEN_CMP_LEQ,
    TOKEN_CMP_GEQ,
    TOKEN_LEFT_ARROW,
} typedef t_token_kind;

enum {
    TOKEN_SUBKIND_NONE,
    TOKEN_SUBKIND_INT,
    TOKEN_SUBKIND_CHAR,
} typedef t_token_subkind;

struct {
    t_token_kind kind;
    t_token_subkind subkind;
    char const *start;
    char const *end;
    union {
        i64 int_value;
        t_intern const *str_value;
    };
    u64 line;
    u64 offset;
} typedef t_token;

struct {
    char const *filename;
    char const *stream;
    t_token last_token;
    u64 line;
    u64 offset;
} typedef t_lexstate;

static void lex_init(t_lexstate *state, char const *filename, char const *stream) {
    state->filename = filename;
    state->stream = stream;
    state->line = 1;
    state->offset = 1;
    t_token eof_token; // to be removed later
    eof_token.start = eof_token.end = null;
    eof_token.kind = TOKEN_EOF;
    state->last_token = eof_token;
}

static inline char const lex_next_char(t_lexstate *state) {
    char result = *state->stream;
    state->stream += 1;
    state->offset += 1;
    if(result == '\n') {
        state->line += 1;
        state->offset = 0;
    }
    return result;
}

static inline bool lex_match_char(t_lexstate *state, char c) {
    if(*state->stream == c) {
        lex_next_char(state);
        return true;
    }
    return false;
}

static u64 char_to_digit[256] = {
    ['0'] = 0,
    ['1'] = 1,
    ['2'] = 2,
    ['3'] = 3,
    ['4'] = 4,
    ['5'] = 5,
    ['6'] = 6,
    ['7'] = 7,
    ['8'] = 8,
    ['9'] = 9,
    ['a'] = 10,['A'] = 10,
    ['b'] = 11,['B'] = 11,
    ['c'] = 12,['C'] = 12,
    ['d'] = 13,['D'] = 13,
    ['e'] = 14,['E'] = 14,
    ['f'] = 15,['F'] = 15,
};

static u64 escape_char[256] = {
    ['n'] = '\n',
    ['t'] = '\t',
    ['r'] = '\r',
    ['a'] = '\a',
    ['b'] = '\b',
    ['v'] = '\v',
    ['\\'] = '\\',
    ['\''] = '\'',
    ['"'] = '"',
};

static bool is_digit(u64 base, char c) {
    u64 digit = char_to_digit[c];
    return (digit < base) && (digit != 0 || c == '0');
}

static u64 lex_integer(u64 base, char const *stream, char const **end) {
    u64 value = 0;
    while(is_digit(base, *stream)) {
        value = base*value + char_to_digit[*stream];
        stream += 1;
    }
    *end = stream;
    return value;
}

// note: not used
static char lex_string_escape(char const *stream, char const **end) {
    char val;
    if(is_digit(16, *stream)) {
        u64 result = lex_integer(16, stream, &stream);
        if(result > 0xff) {
            push_errorf("value %x is unacceptable for a char literal", result);
        }
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
        lex_next_char(state);
    }
    
    char const *start = state->stream;
    
    // lex identifiers
    if(isalpha(*state->stream) || *state->stream == '_') {
        state->last_token.kind = TOKEN_IDN;
        while(isalnum(*state->stream) || *state->stream == '_') {
            lex_next_char(state);
        }
        char const *end = state->stream;
        t_intern const *string_value = intern_string(start, end);
        state->last_token.str_value = string_value;
    }
    
    // lex number literals
    else if(isdigit(*state->stream)) {
        
        // searching the '.' to find whether the number is flt
        while(isdigit(*state->stream)) state->stream+=1;
        if(*state->stream == '.') {
            push_errorf("unexpected symbol '.'");
#if 0
            state->stream = start;
            f64 val = strtod(start, (char **)&state->stream);
            state->last_token.flt_value = val;
            state->last_token.kind = TOKEN_FLT;
#endif
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
            state->last_token.subkind = TOKEN_SUBKIND_INT;
            state->last_token.kind = TOKEN_INT;
        }
    }
    else if(lex_match_char(state, '\'')) { // character literal
        u64 val;
        if(lex_match_char(state, '\\')) {
            val = lex_string_escape(state->stream, &state->stream);
        }
        else if(isprint(*state->stream)) {
            val = *state->stream;
            state->stream += 1;
        }
        else {
            push_errorf("value %x is unacceptable for a char literal", *state->stream);
        }
        state->last_token.kind = TOKEN_INT;
        state->last_token.subkind = TOKEN_SUBKIND_CHAR;
        state->last_token.int_value = val;
        if(!lex_match_char(state, '\'')) {
            push_errorf("expected a char literal to close");
        }
    }
    else if(lex_match_char(state, '"')) { // string literal
        push_errorf("unexpected symbol '\"'");
#if 0    
        string_builder_start();
        while(true) {
            //char c = state_next_char(state);
            if(lex_match_char(state, '"')) break;
            else if(lex_match_char(state, '\\')) {
                string_builder_append_char(lex_string_escape(state->stream, &state->stream));
            }
            else {
                string_builder_append_char(*state->stream++);
            }
        }
        char *result = string_builder_finish();
        t_intern const *intern = intern_string(result, result + string_builder.len);
        state->last_token.str_value = intern;
        state->last_token.kind = TOKEN_STR;
#endif
    }
    else { // parse operators.
        
        if(lex_match_char(state, '=')) {
            state->last_token.kind = '=';
            if(lex_match_char(state, '=')) {
                state->last_token.kind = TOKEN_CMP_EQ;
            }
        }
        else if(lex_match_char(state, '!')) {
            state->last_token.kind = '!';
            if(lex_match_char(state, '=')) {
                state->last_token.kind = TOKEN_CMP_NEQ;
            }
        }
        else if(lex_match_char(state, '>')) {
            state->last_token.kind = '>';
            if(lex_match_char(state, '=')) {
                state->last_token.kind = TOKEN_CMP_GEQ;
            }
        }
        else if(lex_match_char(state, '<')) {
            state->last_token.kind = '<';
            if(lex_match_char(state, '=')) {
                state->last_token.kind = TOKEN_CMP_LEQ;
            }
            if(lex_match_char(state, '-')) {
                state->last_token.kind = TOKEN_LEFT_ARROW;
            }
        }
        else if(*state->stream == 0) {
            state->last_token.kind = TOKEN_EOF;
        }
        else {
            state->last_token.kind = *state->stream;
            lex_next_char(state);
        }
    }
    char const *end = state->stream;
    state->last_token.start = start;
    state->last_token.end = end;
    state->last_token.line = state->line;
    state->last_token.offset = state->offset;
}

static char const *get_token_kind_name(t_token_kind kind) {
    if(kind == TOKEN_INT) {return "INT";}
    else if(kind == TOKEN_IDN) {return "NAME";}
    //else if(kind == TOKEN_FLT) {return "FLOAT";}
    //else if(kind == TOKEN_STR) {return "STRING";}
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
    else if(kind == TOKEN_CMP_NEQ) {return "!=";}
    else if(kind == TOKEN_CMP_EQ) {return "==";}
    else if(kind == TOKEN_CMP_LEQ) {return "<=";}
    else if(kind == TOKEN_CMP_GEQ) {return ">=";}
    else if(kind == TOKEN_LEFT_ARROW) {return "<-";}
    else if(kind == 0) {return "EOF";}
    return "{unknown token}";
}

static char const *get_token_string(t_token *token) {
    t_token_kind kind = token->kind;
    if(kind == TOKEN_IDN) {
        return token->str_value->str;
    }
    //else if(kind == TOKEN_FLT) {return "FLOAT";}
    //else if(kind == TOKEN_STR) {return "STRING";}
    return get_token_kind_name(kind);
}

static void print_token(t_token *token) {
    printf("%s", get_token_string(token));
}

