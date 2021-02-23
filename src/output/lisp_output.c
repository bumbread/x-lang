
static void ast_node_print_lisp(t_ast_node *node, int level) {
    if(node == null) {
        printf("<ERROR>");
        return;
    }
    if(node->cat == AST_expr_node) {
        if(node->expr.cat == EXPR_int_value) {
            printf("%llu", node->expr.ivalue);
        }
        else if(node->expr.cat == EXPR_float_value) {
            printf("%f", node->expr.fvalue);
        }
        else if(node->expr.cat == EXPR_string_value) {
            // TODO(bumbread): print with escape sequences char-by-char?
            printf("\"%s\"", node->expr.svalue->str);
        }
        else if(node->expr.cat == EXPR_variable) {
            printf("%s", node->expr.var_name->str);
        }
        else if(node->expr.cat == EXPR_unary_op) {
            assert(op_is_unary(node->expr.op));
            printf("(");
            printf("%s", get_operator_string(node->expr.op));
            printf(" ");
            ast_node_print_lisp(node->expr.opr1, level+1);
            printf(")");
        }
        else if(node->expr.cat == EXPR_binary_op) {
            assert(op_is_binary(node->expr.op));
            printf("(%s ", get_operator_string(node->expr.op));
            ast_node_print_lisp(node->expr.opr1, level+1);
            printf(" ");
            ast_node_print_lisp(node->expr.opr2, level+1);
            printf(")");
        }
        else if(node->expr.cat == EXPR_ternary_op) {
            assert(op_is_ternary(node->expr.op));
            assert(node->expr.op == TERNARY_slice);
            printf("(%s ", get_operator_string(node->expr.op));
            ast_node_print_lisp(node->expr.opr1, level+1);
            printf(" ");
            if(node->expr.opr2 == null) {
                printf("start");
            }
            else ast_node_print_lisp(node->expr.opr2, level+1);
            printf(" ");
            if(node->expr.opr3 == null) {
                printf("end");
            }
            else {
                ast_node_print_lisp(node->expr.opr3, level+1);
            }
            printf(")");
        }
        else assert(false);
    }
    else if(node->cat == AST_stmt_node) {
        switch(node->stmt.cat) {
            case STMT_if: {
                printf("(if ");
                ast_node_print_lisp(node->stmt.if_condition, level+1);
                ast_node_print_lisp(node->stmt.if_true_block, level+1);
                if(node->stmt.if_false_block != null) {
                    ast_node_print_lisp(node->stmt.if_false_block, level+1);
                }
                printf("\n");
                print_level(level);
                printf(")");
            } break;
            case STMT_while: {
                printf("(while ");
                ast_node_print_lisp(node->stmt.while_condition, level+1);
                ast_node_print_lisp(node->stmt.while_block, level+1);
                printf("\n");
                print_level(level);
                printf(")");
            } break;
            case STMT_break: {
                printf("break");
            } break;
            case STMT_return: {
                printf("(return ");
                if(node->stmt.stmt_value != null) {
                    printf("\n");
                    print_level(level+1);
                    ast_node_print_lisp(node->stmt.stmt_value, level+1);
                    printf("\n");
                    print_level(level);
                }
                printf(")");
            } break;
            case STMT_continue: {
                printf("continue");
            } break;
            case STMT_print: {
                printf("(print ");
                printf("\n");
                print_level(level+1);
                ast_node_print_lisp(node->stmt.stmt_value, level+1);
                printf("\n");
                print_level(level);
                printf(")");
            } break;
            case STMT_block: {
                printf("\n");
                print_level(level);
                printf("(compound");
                for(t_ast_list_link *link = node->stmt.statements.first;
                    link != null;
                    link = link->next) {
                    t_ast_node *statement = link->p;
                    printf("\n");
                    print_level(level+1);
                    ast_node_print_lisp(statement, level + 1);
                }
                printf("\n");
                print_level(level);
                printf(")");
            } break;
            case STMT_declaration: {
                printf("(decl");
                if(null != node->stmt.decl_name) {
                    printf(" %s ", node->stmt.decl_name->str);
                }
                else printf(" NONAME ");
                ast_node_print_lisp(node->stmt.decl_type, level+1);
                if(null != node->stmt.decl_value) {
                    printf(" ");
                    ast_node_print_lisp(node->stmt.decl_value, level+1);
                }
                printf(")");
            } break;
            default: assert(false);
        }
    }
    else if(node->cat == AST_type_node) {
        switch(node->type.cat) {
            case TYPE_alias: {
                printf("%s", node->type.name->str);
            } break;
            case TYPE_pointer: {
                printf("(pointer to ");
                ast_node_print_lisp(node->type.base_type, 0);
                printf(")");
            } break;
            case TYPE_slice: {
                printf("(slice of ");
                ast_node_print_lisp(node->type.base_type, 0);
                printf(")");
            } break;
            case TYPE_function: {
                printf("(function returning ");
                ast_node_print_lisp(node->type.return_type, level+1);
                t_ast_node *parameter_list_node = node->type.parameters;
                assert(parameter_list_node->cat == AST_list_node);
                for(t_ast_list_link *link = parameter_list_node->list.first;
                    link != null;
                    link = link->next) {
                    t_ast_node *param = link->p;
                    printf("\n");
                    print_level(level+1);
                    printf("param: ");
                    ast_node_print_lisp(param, level+1);
                }
                printf("\n");
                print_level(level);
                printf(")");
            } break;
            default: {
                printf("(type ERROR");
            }
        }
    }
    else if(node->cat == AST_list_node) {
        for(t_ast_list_link *link = node->list.first;
            link != null;
            link = link->next) {
            t_ast_node *param = link->p;
            printf("\n");
            print_level(level+1);
            printf("param: ");
            ast_node_print_lisp(param, level+1);
        }
    }
    else {
        assert(false);
    }
}

