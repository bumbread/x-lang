
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

//#include"output/c_output.c"
#include"output/tree_output.c"

int main(void) {
    
    init_errors(20);
    string_builder_init();
    
    test_lexing();
    test_interns();
    
    init_interns(malloc);
    parser_init_memory();
    //checker_init_types();
    
    byte *buf;
    char const *filename = "test.x";
    FILE *input = fopen(filename, "rb");
    
    if(null == input) {
        printf("filename '%s' not found\n", filename);
        return 1;
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
    
    ast_node_print_tree(code, 0);
    printf("\n\n");
    
#if 0    
    t_ast_node *code = parse_ast_node_global_level(":int");
    printf("\n\noutput tree:\n\n");
    ast_node_print_tree(code, 0);
    printf("\n");
#endif
    
#if 0
    char const *output_filename = "test.c";
    FILE *output = fopen(output_filename, "w");
    print_output(stdout, code);
#endif
    
    return 0;
}
