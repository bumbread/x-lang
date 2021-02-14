
static void token_print(t_token token) {
    if(token.kind < 128 && isprint(token.kind)) {
        printf(" `%c` ", token.kind);
    }
    else if(token.kind == TOKEN_INT) {
        printf(" %lld ", token.int_value);
    }
    else if(token.kind == TOKEN_IDN) {
        printf(" [%.*s] ", (int)(token.end - token.start), token.start);
    }
    else {
        printf(" `%s` ", get_token_kind_name(token.kind));
    }
}

#define test_token_op(op)   lex_next_token(&state); assert(state.last_token.kind == (op))
#define test_token_int(val) lex_next_token(&state); assert(state.last_token.kind == TOKEN_INT && state.last_token.int_value == (val))
#define test_token_flt(val) lex_next_token(&state); assert(state.last_token.kind == TOKEN_FLT && state.last_token.flt_value == (val))
#define test_token_chr(val) lex_next_token(&state); assert(state.last_token.kind == TOKEN_INT && state.last_token.subkind == TOKEN_SUBKIND_CHAR && state.last_token.int_value == (val))
#define test_token_str(val) intern = intern_cstring(val); lex_next_token(&state); assert(state.last_token.kind == TOKEN_STR && intern == state.last_token.str_value);
static void test_lexing(void) {
    
    // operators.
    t_lexstate state;
    char const *string = "+ - == <= > < != !";
    lex_init(&state, string);
    test_token_op('+');
    test_token_op('-');
    test_token_op(TOKEN_CMP_EQ);
    test_token_op(TOKEN_CMP_LEQ);
    test_token_op('>');
    test_token_op('<');
    test_token_op(TOKEN_CMP_NEQ);
    test_token_op('!');
    
    
    // integer literals
    string = " 123\n   0123 00 ";
    lex_init(&state, string);
    test_token_int(123);
    test_token_int(123);
    test_token_int(0);
    
    
#if 0  
    // floating literals
    string = "123.12      0.123 0.\t\t 12.e+12";
    lex_init(&state, string);
    test_token_flt(123.12);
    test_token_flt(0.123);
    test_token_flt(0.);
    test_token_flt(12.e+12);
#endif
    
    // char literals
    string = "  '2'      '\\n'  '\"' '\\10'";
    lex_init(&state, string);
    test_token_chr('2');
    test_token_chr('\n');
    test_token_chr('"');
    test_token_chr(0x10);
    
#if 0  
    // string_literals
    init_interns(malloc);
    string_builder_init();
    t_intern const *intern;
    string = "  \"2\"      \"apappa\\naaoao\"  \"'\" \" \" \"'\\20\"";
    lex_init(&state, string);
    test_token_str("2");
    test_token_str("apappa\naaoao");
    test_token_str("'");
    test_token_str(" ");
    test_token_str("' ");
#endif
}
#undef test_token_op
#undef test_token_int
#undef test_token_flt
#undef test_token_chr
#undef test_token_str

static void test_interns(void) {
    init_interns(malloc);
    t_intern const *str_1 = intern_cstring("hello");
    t_intern const *str_2 = intern_cstring("hello");
    t_intern const *str_3 = intern_cstring("hellow");
    assert(interns_equal(str_1, str_2));
    assert(interns_equal(str_2, str_2));
    assert(interns_equal(str_2, str_3) == false);
    assert(interns_equal(str_1, str_3) == false);
}
