

static int level = 0;

void python_print_function_parameters(t_ast_node *node) {
    assert(node->cat = AST_list_node);
    for(t_ast_list_link *param = node->list.first;
        param != null;
        param = param->next) {
        t_ast_node *param_node = param->p;
        assert(param_node->cat == AST_stmt_node);
        assert(param_node->stmt.cat = STMT_declaration);
        printf("%s", param_node->stmt.decl_name);
        if(param->next != null) {
            printf(", ");
        }
    }
}


static char const *python_get_operator_string(t_operator_cat cat) {
    switch(cat) {
        case UNARY_add: return "+";
        case UNARY_sub: return "-";
        case UNARY_addr: return "$";
        case UNARY_deref: return "@";
        case BINARY_ass: return "=";
        case BINARY_add: return "+";
        case BINARY_sub: return "-";
        case BINARY_mul: return "*";
        case BINARY_div: return "/";
        case BINARY_add_ass: return "+=";
        case BINARY_sub_ass: return "-=";
        case BINARY_div_ass: return "*=";
        case BINARY_mul_ass: return "/=";
        case BINARY_and: return "and";
        case BINARY_or: return "or";
        case BINARY_less: return "<";
        case BINARY_greater: return ">";
        case BINARY_leq: return "<=";
        case BINARY_geq: return ">=";
        case BINARY_eq: return "==";
        case BINARY_neq: return "!=";
        case BINARY_function_call: {return "call";
            case BINARY_subscript: return "subscript";
            case TERNARY_slice: return "slice";
        }
        return "<invalid unary operation>";
    }
    
    static void python_print_expression(t_ast_node *node) {
        switch(node->expr.cat) {
            case EXPR_int_value: {
                printf("%llu", node->expr.ivalue);
            }
            case EXPR_float_value: {
                printf("%f", node->expr.fvalue);
            }
            case EXPR_string_value: {
                // TODO(bumbread): print with escape sequences char-by-char?
                printf("\"%s\"", node->expr.svalue->str);
            }
            case EXPR_variable: {
                printf("%s", node->expr.var_name->str);
            }
            case EXPR_unary_op: {
                assert(op_is_unary(node->expr.op));
                printf("(");
                printf("%s", get_operator_string(node->expr.op));
                ast_node_print_python(node->expr.opr1, false);
                printf(")");
            }
            case EXPR_binary_op: {
                assert(op_is_binary(node->expr.op));
                printf("(");
                ast_node_print_python(node->expr.opr1, false);
                printf(" %s ", get_operator_string(node->expr.op));
                ast_node_print_python(node->expr.opr2, false);
                printf(")");
            }
            case EXPR_ternary_op: {
                assert(op_is_ternary(node->expr.op));
                assert(node->expr.op == TERNARY_slice);
                printf("(");
                ast_node_print_python(node->expr.opr1, false);
                printf("[");
                if(node->expr.opr2 != null) {
                    ast_node_print_python(node->expr.opr2, false);
                }
                printf(":");
                if(node->expr.opr3 != null) {
                    ast_node_print_python(node->expr.opr3, false);
                }
                printf("])");
            }
            default:
            assert(false);
        }
    }
    
    static void ast_node_print_python(t_ast_node *node, bool is_newline) {
        if(is_newline) {
            printf("\n");
            print_level(level);
        }
        level += 1;
        if(node != null) {
            if(node->cat == AST_expr_node) {
                python_print_expression(node);
            }
            else if(node->cat == AST_stmt_node) {
                switch(node->stmt.cat) {
                    case STMT_if: {
                        printf("if ");
                        ast_node_print_python(node->stmt.if_condition, false);
                        printf(":");
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
                                ast_node_print_python(statement, true);
                            }
                        }
                        if(node->stmt.if_false_block != null) {
                            printf("else:");
                            t_ast_node *block = node->stmt.if_false_block;
                            assert(block->cat = AST_stmt_node);
                            assert(block->stmt.cat = STMT_block);
                            for(t_ast_list_link *link = block->stmt.statements.first;
                                link != null;
                                link = link->next) {
                                t_ast_node *statement = link->p;
                                ast_node_print_python(statement, true);
                            }
                        }
                    } break;
                    case STMT_while: {
                        printf("while ");
                        ast_node_print_python(node->stmt.while_condition, false);
                        printf(":");
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
                                ast_node_print_python(statement, true);
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
                            ast_node_print_python(node->stmt.stmt_value, false);
                        }
                        printf(";");
                    } break;
                    case STMT_continue: {
                        printf("continue;");
                    } break;
                    case STMT_print: {
                        printf("print ");
                        ast_node_print_python(node->stmt.stmt_value, false);
                        printf(";");
                    } break;
                    case STMT_block: {
                        level -= 1;
                        for(t_ast_list_link *link = node->stmt.statements.first;
                            link != null;
                            link = link->next) {
                            t_ast_node *statement = link->p;
                            ast_node_print_python(statement, true);
                        }
                        level += 1;
                    } break;
                    case STMT_declaration: {
                        assert(node->stmt.decl_name != null);
                        assert(node->stmt.decl_type != null);
                        assert(node->stmt.decl_type->cat == AST_type_node);
                        if(node->stmt.decl_type->type.cat == TYPE_function) {
                            t_type_node *function_type = &node->stmt.decl_type->type;
                            printf("def %s(", node->stmt.decl_name->str);
                            python_print_function_parameters(function_type->parameters);
                            printf("):");
                            t_ast_node *block = node->stmt.decl_value;
                            assert(block->cat = AST_stmt_node);
                            assert(block->stmt.cat = STMT_block);
                            for(t_ast_list_link *link = block->stmt.statements.first;
                                link != null;
                                link = link->next) {
                                t_ast_node *statement = link->p;
                                ast_node_print_python(statement, true);
                            }
                        }
                        else {
                            if(null != node->stmt.decl_value) {
                                printf("%s = ", node->stmt.decl_name->str);
                                ast_node_print_python(node->stmt.decl_value, false);
                            }
                            printf(";");
                        } break;
                    }
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
                        ast_node_print_python(node->type.base_type, false);
                        printf(")");
                    } break;
                    case TYPE_slice: {
                        printf("(slice of ");
                        ast_node_print_python(node->type.base_type, false);
                        printf(")");
                    } break;
                    case TYPE_function: {
                        printf("(function returning ");
                        ast_node_print_python(node->type.return_type, false);
                        t_ast_node *parameter_list_node = node->type.parameters;
                        assert(parameter_list_node->cat == AST_list_node);
                        for(t_ast_list_link *link = parameter_list_node->list.first;
                            link != null;
                            link = link->next) {
                            t_ast_node *param = link->p;
                            printf("(param ");
                            ast_node_print_python(param, false);
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
                    ast_node_print_python(param, level+1);
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
    
    
    