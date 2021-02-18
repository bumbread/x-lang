
#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<stdarg.h>
#include<ctype.h>
#include<assert.h>
#include<stdlib.h>
#include<stdint.h>

#include"common.c"
#include<stdio.h>
#include"memory.c"
#include"string.c"

#include"lexer.c"
#include"parser.c"
#include"checker.c"
#include"test.c"

#include"c_output/generator.c"

int main(void) {
    init_errors(20);
    string_builder_init();
    
    test_lexing();
    test_interns();
    
    init_interns(malloc);
    parser_init_memory(10*mb, malloc(10*mb));
    
    byte *buf;
    char const *filename = "test.x";
    FILE *input = fopen(filename, "rb");
    if(null == input) {
        printf("filename '%s' not found\n", filename);
    }
    
    fseek(input, 0, SEEK_END);
    size_t input_size = ftell(input);
    fseek(input, 0, SEEK_SET);
    buf = malloc(input_size);
    fread(buf, input_size, 1, input);
    
    t_lexstate state;
    lex_init(&state, filename, (char *)buf);
    lex_next_token(&state);
    t_ast_node *code = parse_global_scope(&state);
    check_errors();
    
    char const *output_filename = "test.c";
    FILE *output = fopen(output_filename, "w");
    generate_output(output, code);
#if 0
    printf("\n");
    ast_node_print_lisp(code, 0);
    printf("\n");
#endif
    
    return 0;
}
