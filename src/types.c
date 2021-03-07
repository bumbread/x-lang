
static t_ast_node *get_default_value_for_type(t_ast_node *type) {
    assert(type->cat == AST_type_node);
    switch(type->type.cat) {
        case TYPE_alias: {
            if(type->type.name == keyword_int) {
                t_ast_node *result = alloc_node();
                result->cat = AST_expr_node;
                result->expr.cat = EXPR_int_value;
                result->expr.ivalue = 0;
                return result;
            }
            if(type->type.name == keyword_string) {
                t_ast_node *result = alloc_node();
                result->cat = AST_expr_node;
                result->expr.cat = EXPR_string_value;
                result->expr.svalue = 0;
                return result;
            }
            if(type->type.name == keyword_bool) {
                t_ast_node *result = alloc_node();
                result->cat = AST_expr_node;
                result->expr.cat = EXPR_int_value;
                result->expr.ivalue = 0;
                return result;
            }
            if(type->type.name == keyword_byte) {
                t_ast_node *result = alloc_node();
                result->cat = AST_expr_node;
                result->expr.cat = EXPR_int_value;
                result->expr.ivalue = 0;
                return result;
            }
            if(type->type.name == keyword_float) {
                t_ast_node *result = alloc_node();
                result->cat = AST_expr_node;
                result->expr.cat = EXPR_float_value;
                result->expr.fvalue = 0;
                return result;
            }
        } break;
        case TYPE_pointer: {
            t_ast_node *result = alloc_node();
            result->cat = AST_expr_node;
            result->expr.cat = EXPR_int_value;
            result->expr.ivalue = 0;
            return result;
        } break;
        case TYPE_slice:
        case TYPE_function: {
            return null;
        } break;
        default: assert(false);
    }
    return false;
}

static bool is_reference_type(t_ast_node *type_node) {
    assert(type_node->cat == AST_type_node);
    return type_node->type.cat == TYPE_pointer;
}

static bool is_callable_type(t_ast_node *type_node) {
    assert(type_node->cat == AST_type_node);
    return type_node->type.cat == TYPE_function;
}

static bool is_subscriptable_type(t_ast_node *type_node) {
    assert(type_node->cat == AST_type_node);
    if(type_node->type.cat == TYPE_slice)
        return true;
    if(type_node->type.cat == TYPE_alias)
        return type_node->type.name == keyword_string;
    return false;
}

static bool is_sliceable_type(t_ast_node *type_node) {
    assert(type_node->cat == AST_type_node);
    if(type_node->type.cat == TYPE_slice)
        return true;
    if(type_node->type.cat == TYPE_alias)
        return type_node->type.name == keyword_string;
    return false;
}

static bool is_arithmetic_type(t_ast_node *type_node) {
    assert(type_node->cat == AST_type_node);
    if(type_node->type.cat == TYPE_alias) {
        return type_node->type.name == keyword_int 
            || type_node->type.name == keyword_float;
    }
    return false;
}

static bool is_logical_type(t_ast_node *type1) {
    assert(type1->cat == AST_type_node);
    if(type1->type.cat == TYPE_alias) {
        if(type1->type.name == keyword_bool) {
            return true;
        }
    }
    return false;
}


static bool are_types_arithmetically_compatible(t_ast_node *type1, t_ast_node *type2) {
    if(is_arithmetic_type(type1)) {
        if(is_arithmetic_type(type2)) {
            // assumption that both arithmetic types are TYPE_alias
            return type1->type.name == type2->type.name;
        }
    }
    return false;
}

// < > <= >=
static bool are_types_relational(t_ast_node *type1, t_ast_node *type2) {
    assert(type1->cat == AST_type_node);
    assert(type2->cat == AST_type_node);
    if(type1->type.cat == TYPE_pointer) {
        // TODO(bumbread): check whether the underlying types are equal?
        if(type2->type.cat == TYPE_pointer) return true;
    }
    if(type1->type.cat == TYPE_alias) {
        if(type2->type.cat == TYPE_alias) {
            if(type1->type.name == keyword_int && type2->type.name == keyword_int) return true;
            if(type2->type.name == keyword_float && type2->type.name == keyword_float) return true;
        }
    }
    return false;
}

// = !=
static bool are_types_comparable(t_ast_node *type1, t_ast_node *type2) {
    assert(type1->cat == AST_type_node);
    assert(type2->cat == AST_type_node);
    if(type1->type.cat == type2->type.cat) {
        if(type1->type.cat != TYPE_slice) {
            // TODO(bumbread): fix assumption that aliases can not refer to different types.
            if(type1->type.cat == TYPE_alias) {
                if(type1->type.name == type2->type.name) {
                    return true;
                }
            }
            else {
                return true;
            }
        }
    }
    return false;
}

static bool are_types_logical(t_ast_node *type1, t_ast_node *type2) {
    assert(type1->cat == AST_type_node);
    assert(type2->cat == AST_type_node);
    if(type1->type.cat == TYPE_alias && type2->type.cat == TYPE_alias) {
        if(type1->type.name == keyword_bool && type2->type.name == keyword_bool) {
            return true;
        }
    }
    return false;
}

static bool can_assign_types(t_ast_node *to, t_ast_node *from) {
    // TODO(bumbread): replace recusrive calls with another function
    // e.g. types_are_equal
    assert(to->cat == AST_type_node);
    assert(from->cat == AST_type_node);
    if(to->type.cat == from->type.cat) {
        switch(to->type.cat) {
            case TYPE_alias: {
                // TODO(bumbread): fix assumption that unequal aliases are
                // unequal types.
                return to->type.name == from->type.name;
            }
            case TYPE_slice:
            case TYPE_pointer: {
                return can_assign_types(to->type.base_type, to->type.base_type);
            } break;
            case TYPE_function: {
                if(can_assign_types(to->type.return_type, from->type.return_type)) {
                    t_ast_node *params1 = to->type.parameters;
                    t_ast_node *params2 = from->type.parameters;
                    assert(params1->cat == AST_list_node);
                    assert(params2->cat == AST_list_node);
                    t_ast_list_link *link1;
                    t_ast_list_link *link2;
                    for(link1 = params1->list.first, link2 = params2->list.first;
                        link1 != null && link2 != null;
                        link1 = link1->next, link2 = link2->next) {
                        t_ast_node *param1_decl = link1->p;
                        t_ast_node *param2_decl = link2->p;
                        assert(param1_decl->cat == AST_stmt_node);
                        assert(param2_decl->cat == AST_stmt_node);
                        assert(param1_decl->stmt.cat == STMT_declaration);
                        assert(param2_decl->stmt.cat == STMT_declaration);
                        if(!can_assign_types(param1_decl->stmt.decl_type, param2_decl->stmt.decl_type)) {
                            return false;
                        }
                    }
                    if(link1 != link2) {
                        return false;
                    }
                    return true;
                }
                return false;
            } break;
        }
    }
    return false;
}

static char const *get_short_type_name(t_ast_node *type) {
    assert(type->cat == AST_type_node);
    switch(type->type.cat) {
        case TYPE_alias: return type->type.name->str;
        case TYPE_pointer: return "pointer";
        case TYPE_slice: return "slice";
        case TYPE_function: return "function";
        default: assert(false);
    }
    return "???";
}

static void print_type(t_ast_node *type) {
    assert(type->cat == AST_type_node);
    switch(type->type.cat) {
        case TYPE_alias: {
            printf("%s", type->type.name->str); 
        } break;
        case TYPE_pointer: {
            printf("pointer to ");
            print_type(type->type.base_type);
        } break;
        case TYPE_slice: {
            printf("slice of ");
            print_type(type->type.base_type);
        } break;
        case TYPE_function: {
            printf("function taking (");
            t_ast_node *params_list = type->type.parameters;
            for(t_ast_list_link *param = params_list->list.first;
                param != null;
                param = param->next) {
                t_ast_node *param_decl = param->p;
                assert(param_decl->cat == AST_stmt_node);
                assert(param_decl->stmt.cat == STMT_declaration);
                print_type(param_decl->stmt.decl_type);
                if(param->next != null) {
                    printf(", ");
                }
            }
            printf(") returning ");
            print_type(type->type.return_type);
        } break;
        default: assert(false);
    }
}
