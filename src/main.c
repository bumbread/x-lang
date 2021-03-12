
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

#include"compiler_desc.c"
#include"lexer.c"
#include"nodes.c"
#include"parser.c"
#include"checker.c"
#include"test.c"

//#include"output/c_output.c"
#include"output/tree_output.c"

int main(void) {
    
    init_error_buffer(20);
    string_builder_init();
    
    test_lexing();
    test_interns();
    
    init_interns(malloc);
    init_compiler();
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
    buf = malloc(1+input_size);
    fread(buf, input_size, 1, input);
    buf[input_size] = 0;
    
    t_lexstate state;
    lex_init(&state, filename, (char *)buf);
    lex_next_token(&state);
    t_stmt_list *code = parse_global_scope(&state);
    print_error_buffer();
    if(now_errors == 0) {
        check_code(code);
        print_error_buffer();
    }
    
    print_code_root(code);
    //ast_node_print_tree(code, 0);
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
    
    printf("application terminated successfully");
    return 0;
}
