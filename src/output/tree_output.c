
static void print_expr(t_expr_data *expr) {
    assert(expr != null);
    switch(expr->cat) {
        case EXPR_value: {
            if(expr->value.cat == VALUE_int) {
                printf("%llu", expr->value.i);
            }
            else if(expr->value.cat == VALUE_string) {
                printf("\"%s\"", expr->value.s->str);
            }
            else if(expr->value.cat == VALUE_float) {
                printf("%f", expr->value.f);
            }
        } break;
        case EXPR_variable: {
            printf("%s", expr->var_name->str);
        } break;
        case EXPR_unary: {
            printf("(%s ", get_operator_string(expr->operation.cat));
            print_expr(expr->operation.expr1);
            printf(")");
        } break;
        case EXPR_binary: {
            printf("(%s ", get_operator_string(expr->operation.cat));
            print_expr(expr->operation.expr1);
            printf(" ");
            print_expr(expr->operation.expr2);
            printf(")");
        } break;
        case EXPR_ternary: {
            printf("%s ", get_operator_string(expr->operation.cat));
            print_expr(expr->operation.expr1);
            printf(" ");
            if(expr->operation.expr2 != null) {
                print_expr(expr->operation.expr2);
            }
            else {
                printf("start");
            }
            printf(" ");
            if(expr->operation.expr3 != null) {
                print_expr(expr->operation.expr3);
            }
            else {
                printf("end");
            }
            printf(")");
        } break;
        case EXPR_function_call: {
            t_function_data *func = &expr->func;
            print_expr(func->callee);
            printf("(");
            for(t_expr_list_node *expr_node = func->parameters->first;
                expr_node != null;
                expr_node = expr_node->next) {
                t_expr_data *expr = expr_node->data;
                print_expr(expr);
            }
            printf(")");
        } break;
        default: assert(false);
    }
}

static void print_type(t_type_data *type) {
    assert(type != null);
    switch(type->cat) {
        case TYPE_int: {
            printf("int");
        } break;
        case TYPE_float: {
            printf("float");
        } break;
        case TYPE_bool: {
            printf("bool");
        } break;
        case TYPE_string: {
            printf("string");
        } break;
        case TYPE_byte: {
            printf("byte");
        } break;
        case TYPE_pointer: {
            print_type(type->pointer_base);
            printf("$");
        } break;
        case TYPE_slice: {
            print_type(type->slice_base);
            printf("[]");
        } break;
        case TYPE_function: {
            print_type(type->func.return_type);
            printf("<-(");
            for(t_decl_list_node *decl_node = type->func.parameters->first;
                decl_node != null;
                decl_node = decl_node->next) {
                t_decl_data *decl = decl_node->data;
                print_type(decl->type);
                if(decl_node->next != null) {
                    printf(", ");
                }
            }
            printf(")");
        } break;
        default: assert(false);
    }
}

static void print_stmt(t_stmt_data *stmt, int level);
static void print_stmt_list(t_stmt_list *list, int level) {
    for(t_stmt_list_node *stmt_node = list->first;
        stmt_node != null;
        stmt_node = stmt_node->next) {
        t_stmt_data *stmt = stmt_node->data;
        print_stmt(stmt, level);
    }
}

static void print_decl(t_decl_data *decl, int level) {
    printf(":");
    print_type(decl->type);
    printf(" %s", decl->name->str);
    if(decl->cat == DECL_block_value) {
        printf(" {\n");
        print_stmt_list(decl->block_data, level+1);
        printf("}\n");
    }
    else if(decl->cat == DECL_expr_value) {
        printf(" = ");
        print_expr(decl->value);
        printf(";\n");
    }
    else printf(";\n");
}

static void print_stmt(t_stmt_data *stmt, int level) {
    switch(stmt->cat) {
        case STMT_break: {
            print_level(level);
            printf("break;\n");
        } break;
        case STMT_continue: {
            print_level(level);
            printf("continue;\n");
        } break;
        case STMT_return: {
            print_level(level);
            if(stmt->return_expr != null) {
                printf("return ");
                print_expr(stmt->return_expr);
                printf(";\n");
            }
            else {
                printf("return;\n");
            }
        } break;
        case STMT_print: {
            print_level(level);
            printf("print ");
            print_expr(stmt->print_expr);
            printf(";\n");
        } break;
        case STMT_if: {
            printf("\n");
            print_level(level);
            printf("if ");
            print_expr(stmt->if_data.condition);
            
            t_stmt_data *true_branch = stmt->if_data.true_branch;
            if(true_branch != null) {
                printf(" ");
                print_stmt(true_branch, level);
            }
            else printf(";\n");
            
            t_stmt_data *false_branch = stmt->if_data.false_branch;
            if(false_branch != null) {
                print_level(level);
                printf("else");
                if(false_branch->cat == STMT_block) {
                    printf(" ");
                    print_stmt(false_branch, level);
                }
                else {
                    printf(" ");
                    print_stmt(false_branch, 0);
                }
            }
        } break;
        case STMT_while: {
            print_level(level);
            printf("while ");
            print_expr(stmt->while_data.condition);
            if(stmt->while_data.block != null) {
                print_stmt(stmt->while_data.block, level+1);
            }
            else printf(";\n");
        } break;
        case STMT_decl: {
            print_level(level);
            t_decl_data *decl = stmt->decl_data;
            print_decl(decl, level);
        } break;
        case STMT_expr: {
            print_level(level);
            print_expr(stmt->expr);
            printf(";\n");
        } break;
        case STMT_block: {
            printf("{\n");
            print_stmt_list(&stmt->block_data, level+1);
            print_level(level);
            printf("}\n");
        } break;
        default: assert(false);
    }
}

static void print_decl_list(t_decl_list *decls) {
    for(t_decl_list_node *decl_node = decls->first;
        decl_node != null;
        decl_node = decl_node->next) {
        t_decl_data *decl = decl_node->data;
        print_decl(decl, 0);
        printf("\n");
    }
}
