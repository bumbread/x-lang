
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
        case EXPR_identifier: {
            printf("%s", expr->variable->str);
        } break;
        case EXPR_unary: {
            printf("(%s ", get_operator_string(expr->operation.cat));
            print_expr(expr->operation.expr1);
            printf(")");
        } break;
        case EXPR_binary: {
            printf("%s ", get_operator_string(expr->operation.cat));
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
            printf("call ");
            print_expr(func->callee);
            printf(" with (");
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
            print_type(type->function_data.return_type);
            printf("<-(");
            for(t_decl_list_node *decl_node = type->function_data.parameters->first;
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
    print_level(level);
    switch(stmt->cat) {
        case STMT_break: {
            printf("break;\n");
        } break;
        case STMT_continue: {
            printf("continue;\n");
        } break;
        case STMT_return: {
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
            printf("print ");
            print_expr(stmt->print_expr);
            printf(";\n");
        } break;
        case STMT_if: {
            printf("if ");
            print_expr(stmt->if_data.condition);
            if(stmt->if_data.true_branch != null) {
                print_stmt(stmt->if_data.true_branch, level+1);
            }
            else printf(";\n");
            if(stmt->if_data.false_branch != null) {
                print_stmt(stmt->if_data.false_branch, level+1);
            }
            else printf(";\n");
        } break;
        case STMT_while: {
            printf("while ");
            print_expr(stmt->while_data.condition);
            if(stmt->while_data.block != null) {
                print_stmt(stmt->while_data.block, level+1);
            }
            else printf(";\n");
        } break;
        case STMT_decl: {
            t_decl_data *decl = stmt->decl_data;
            print_decl(decl, level);
        } break;
        case STMT_expr: {
            print_expr(stmt->expr);
            printf(";\n");
        } break;
        case STMT_block: {
            print_stmt_list(&stmt->block_data, level);
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
