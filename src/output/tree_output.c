
static int level = 0;

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

static void print_stmt(t_stmt_data *stmt) {
    switch(stmt->cat) {
        case STMT_break: {
            printf("break;");
        } break;
        case STMT_continue: {
            printf("continue;");
        } break;
    }
}

