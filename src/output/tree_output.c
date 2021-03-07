
static int level = 0;

static void ast_node_print_tree(t_ast_node *node, bool is_newline) {
    if(is_newline) {
        printf("\n");
        print_level(level);
    }
    level += 1;
    if(node != null) {
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
                ast_node_print_tree(node->expr.opr1, false);
                printf(")");
            }
            else if(node->expr.cat == EXPR_binary_op) {
                assert(op_is_binary(node->expr.op));
                if(node->expr.op != BINARY_function_call) {
                    printf("(");
                    ast_node_print_tree(node->expr.opr1, false);
                    printf(" %s ", get_operator_string(node->expr.op));
                    ast_node_print_tree(node->expr.opr2, false);
                    printf(")");
                }
                else {
                    printf("(call ");
                    ast_node_print_tree(node->expr.opr1, false);
                    t_ast_node *param_list = node->expr.opr2;
                    assert(param_list->cat == AST_list_node);
                    printf("(");
                    for(t_ast_list_link *param_link = param_list->list.first;
                        param_link != null;
                        param_link = param_link->next) {
                        ast_node_print_tree(param_link->p, false);
                        if(param_link->next != null) {
                            printf(", ");
                        }
                    }
                    printf(")");
                    printf(")");
                }
            }
            else if(node->expr.cat == EXPR_ternary_op) {
                assert(op_is_ternary(node->expr.op));
                assert(node->expr.op == TERNARY_slice);
                printf("(");
                ast_node_print_tree(node->expr.opr1, false);
                printf("[");
                if(node->expr.opr2 == null) {
                    printf("start");
                }
                else ast_node_print_tree(node->expr.opr2, false);
                printf(":");
                if(node->expr.opr3 == null) {
                    printf("end");
                }
                else {
                    ast_node_print_tree(node->expr.opr3, false);
                }
                printf("])");
            }
            else assert(false);
        }
        else if(node->cat == AST_stmt_node) {
            switch(node->stmt.cat) {
                case STMT_assignment: {
                    ast_node_print_tree(node->stmt.lvalue, false);
                    printf(" %s ", get_operator_string(node->stmt.ass_op));
                    ast_node_print_tree(node->stmt.rvalue, false);
                    printf(";");
                } break;
                case STMT_if: {
                    printf("if(");
                    ast_node_print_tree(node->stmt.if_condition, false);
                    printf("):");
                    if(node->stmt.if_true_block == null) {
                        printf("\n");
                        print_level(level);
                        printf("pass;");
                    }
                    else {
                        t_ast_node *block = node->stmt.if_true_block;
                        assert(block->cat = AST_stmt_node);
                        assert(block->stmt.cat = STMT_block);
                        for(t_ast_list_link *link = block->stmt.statements.first;
                            link != null;
                            link = link->next) {
                            t_ast_node *statement = link->p;
                            ast_node_print_tree(statement, true);
                        }
                    }
                    if(node->stmt.if_false_block != null) {
                        printf("\n");
                        print_level(level-1);
                        printf("else:");
                        t_ast_node *block = node->stmt.if_false_block;
                        assert(block->cat = AST_stmt_node);
                        assert(block->stmt.cat = STMT_block);
                        for(t_ast_list_link *link = block->stmt.statements.first;
                            link != null;
                            link = link->next) {
                            t_ast_node *statement = link->p;
                            ast_node_print_tree(statement, true);
                        }
                    }
                } break;
                case STMT_while: {
                    printf("while(");
                    ast_node_print_tree(node->stmt.while_condition, false);
                    printf("):");
                    if(node->stmt.while_block == null) {
                        printf("\n");
                        print_level(level);
                        printf("pass;");
                    }
                    else {
                        t_ast_node *block = node->stmt.while_block;
                        assert(block->cat = AST_stmt_node);
                        assert(block->stmt.cat = STMT_block);
                        for(t_ast_list_link *link = block->stmt.statements.first;
                            link != null;
                            link = link->next) {
                            t_ast_node *statement = link->p;
                            ast_node_print_tree(statement, true);
                        }
                    }
                } break;
                case STMT_break: {
                    printf("break;");
                } break;
                case STMT_return: {
                    printf("return");
                    if(node->stmt.stmt_value != null) {
                        printf(" ");
                        ast_node_print_tree(node->stmt.stmt_value, false);
                    }
                    printf(";");
                } break;
                case STMT_continue: {
                    printf("continue;");
                } break;
                case STMT_print: {
                    printf("print ");
                    ast_node_print_tree(node->stmt.stmt_value, false);
                    printf(";");
                } break;
                case STMT_block: {
                    printf("block:");
                    for(t_ast_list_link *link = node->stmt.statements.first;
                        link != null;
                        link = link->next) {
                        t_ast_node *statement = link->p;
                        ast_node_print_tree(statement, true);
                    }
                } break;
                case STMT_declaration: {
                    printf("decl ");
                    if(node->stmt.decl_name != null) {
                        printf("%s :", node->stmt.decl_name->str);
                    }
                    ast_node_print_tree(node->stmt.decl_type, false);
                    if(null != node->stmt.decl_value) {
                        printf(" = ");
                        ast_node_print_tree(node->stmt.decl_value, false);
                    }
                    //printf(";");
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
                    ast_node_print_tree(node->type.base_type, false);
                    printf(")");
                } break;
                case TYPE_slice: {
                    printf("(slice of ");
                    ast_node_print_tree(node->type.base_type, false);
                    printf(")");
                } break;
                case TYPE_function: {
                    printf("(function returning ");
                    ast_node_print_tree(node->type.return_type, false);
                    t_ast_node *parameter_list_node = node->type.parameters;
                    assert(parameter_list_node->cat == AST_list_node);
                    for(t_ast_list_link *link = parameter_list_node->list.first;
                        link != null;
                        link = link->next) {
                        t_ast_node *param = link->p;
                        printf("(param ");
                        ast_node_print_tree(param, false);
                        printf(")");
                    }
                    printf(")");
                } break;
                default: {
                    assert(false);
                }
            }
        }
        else if(node->cat == AST_list_node) {
            assert(false);
            for(t_ast_list_link *link = node->list.first;
                link != null;
                link = link->next) {
                t_ast_node *param = link->p;
                printf("\n");
                print_level(level+1);
                printf("item: ");
                ast_node_print_tree(param, level+1);
            }
        }
        else {
            assert(false);
        }
    }
    else {
        printf("<ERROR>");
    }
    level -= 1;
}


