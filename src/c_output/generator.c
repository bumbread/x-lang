
static int identation_level = 0;
static void ident(FILE *out) {
    for(int i = 0; i < identation_level; i += 1) {
        fprintf(out, " ");
    }
}

static int tmp_var_count = 0;
static int mk_temp_num(void) {
    return tmp_var_count++;
}

static void print_temp(FILE *out, int temp_num) {
    fprintf(out, "_tmp%d", temp_num);
}

static void print_output(FILE *out, t_ast_node *code) {
    printf("int main(void) {\n");
    identation_level += 1;
    ident(out);printf("return 0;\n");
    identation_level -= 1;
    printf("}\n");
}

