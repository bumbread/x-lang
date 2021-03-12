// LISTS

static t_expr_list *alloc_expr_list(void) {
    t_expr_list *result = global_alloc(sizeof(t_expr_list));
    result->first = null;
    result->last = null;
    result->count = 0;
    return result;
}

static t_expr_list_node *alloc_expr_list_node(void) {
    t_expr_list_node *result = global_alloc(sizeof(t_expr_list_node));
    result->next = null;
    result->prev = null;
    result->data = null;
    return result;
}

static void expr_list_push(t_expr_list *list, t_expr_data *expr) {
    t_expr_list_node *to_attach = alloc_expr_list_node();
    to_attach->next = null;
    to_attach->prev = list->last;
    to_attach->data = expr;
    if(list->last != null) {
        list->last->next = to_attach;
    }
    else {
        list->first = to_attach;
    }
    list->last = to_attach;
    list->count += 1;
}

static t_stmt_list *alloc_stmt_list(void) {
    t_stmt_list *result = global_alloc(sizeof(t_stmt_list));
    result->first = null;
    result->last = null;
    result->count = 0;
    return result;
}

static t_stmt_list_node *alloc_stmt_list_node(void) {
    t_stmt_list_node *result = global_alloc(sizeof(t_stmt_list_node));
    result->next = null;
    result->prev = null;
    result->data = null;
    return result;
}

static void stmt_list_push(t_stmt_list *list, t_stmt_data *stmt) {
    t_stmt_list_node *to_attach = alloc_stmt_list_node();
    to_attach->next = null;
    to_attach->prev = list->last;
    to_attach->data = stmt;
    if(list->last != null) {
        list->last->next = to_attach;
    }
    else {
        list->first = to_attach;
    }
    list->last = to_attach;
    list->count += 1;
}

static t_decl_list *alloc_decl_list(void) {
    t_decl_list *result = global_alloc(sizeof(t_decl_list));
    result->first = null;
    result->last = null;
    result->count = 0;
    return result;
}

static t_decl_list_node *alloc_decl_list_node(void) {
    t_decl_list_node *result = global_alloc(sizeof(t_decl_list_node));
    result->next = null;
    result->prev = null;
    result->data = null;
    return result;
}

static void decl_list_push(t_decl_list *list, t_decl_data *decl) {
    t_decl_list_node *to_attach = alloc_decl_list_node();
    to_attach->next = null;
    to_attach->prev = list->last;
    to_attach->data = decl;
    if(list->last != null) {
        list->last->next = to_attach;
    }
    else {
        list->first = to_attach;
    }
    list->last = to_attach;
    list->count += 1;
}

// DECL STACK

typedef t_decl_list t_decl_stack;
typedef t_decl_list_node t_decl_stack_node;

static t_decl_stack *alloc_decl_stack(void) {
    t_decl_stack *result = global_alloc(sizeof(t_decl_stack));
    result->first = null;
    result->last = null;
    result->count = 0;
    return result;
}

static t_decl_stack_node *alloc_stack_node(void) {
    t_decl_stack_node *result = global_alloc(sizeof(t_decl_stack_node));
    result->next = null;
    result->prev = null;
    result->data = null;
    return result;
}

static void decl_push(t_decl_stack *stack, t_decl_data *data) {
    assert(data != null);
    decl_list_push(stack, data);
}

static void stack_list_push_frame(t_decl_stack *stack) {
    decl_list_push(stack, null);
}

static void stack_list_pop_frame(t_decl_stack *stack) {
    t_decl_list_node *search_node = stack->last;
    while(search_node->data != null) {
        search_node = search_node->prev;
        if(search_node == null) {
            assert(0 && "unbalanced pushes and pops");
            return;
        }
    }
    t_decl_list_node *new_last = search_node->prev;
    if(new_last != null) {
        new_last->next = null;
    }
    stack->last = new_last;
}

// AST DATA

static t_expr_data *make_expr(t_token_location loc) {
    t_expr_data *result = global_alloc(sizeof(t_expr_data));
    memset(result, 0, sizeof(t_expr_data));
    result->loc = loc;
    return result;
}

static t_stmt_data *make_stmt(t_token_location loc) {
    t_stmt_data *result = global_alloc(sizeof(t_stmt_data));
    memset(result, 0, sizeof(t_stmt_data));
    result->loc = loc;
    return result;
}

static t_type_data *make_type(void) {
    t_type_data *result = global_alloc(sizeof(t_type_data));
    memset(result, 0, sizeof(t_type_data));
    return result;
}

static t_decl_data *make_decl(void) {
    t_decl_data *result = global_alloc(sizeof(t_decl_data));
    memset(result, 0, sizeof(t_decl_data));
    return result;
}

static t_expr_data *make_static_value(t_token_location loc) {
    t_expr_data *result = make_expr(loc);
    result->loc = loc;
    result->cat = EXPR_value;
    result->flags = EXPR_static;
    return result;
}

static t_type_data *make_int_type(void) {
    t_type_data *result = make_type();
    result->cat = TYPE_int;
    return result;
}

static t_type_data *make_float_type(void) {
    t_type_data *result = make_type();
    result->cat = TYPE_float;
    return result;
}

static t_type_data *make_byte_type(void) {
    t_type_data *result = make_type();
    result->cat = TYPE_byte;
    return result;
}

static t_type_data *make_bool_type(void) {
    t_type_data *result = make_type();
    result->cat = TYPE_bool;
    return result;
}

static t_type_data *make_string_type(void) {
    t_type_data *result = make_type();
    result->cat = TYPE_string;
    return result;
}

static t_expr_data *make_value(t_token_location loc) {
    t_expr_data *result = make_expr(loc);
    result->cat = EXPR_value;
    result->flags = 0;
    return result;
}

static t_expr_data *make_identifier_expr(t_intern const *name, t_token_location loc) {
    assert(name != null);
    t_expr_data *result = make_expr(loc);
    result->cat = EXPR_variable;
    result->flags = 0;
    result->var_name = name;
    return result;
}

static t_expr_data *make_unary_expr(f_operation_cat op, t_expr_data *expr, t_token_location loc) {
    assert(op_is_unary(op));
    t_expr_data *result = make_expr(loc);
    result->cat = EXPR_unary;
    result->flags = 0;
    result->operation.cat = op;
    result->operation.expr1 = expr;
    return result;
}

static t_expr_data *make_binary_expr(f_operation_cat op,
                                     t_expr_data *expr1,
                                     t_expr_data *expr2,
                                     t_token_location loc) {
    assert(op_is_binary(op));
    t_expr_data *result = make_expr(loc);
    result->cat = EXPR_binary;
    result->flags = 0;
    result->operation.cat = op;
    result->operation.expr1 = expr1;
    result->operation.expr2 = expr2;
    return result;
}

static t_expr_data *make_ternary_expr(f_operation_cat op,
                                      t_expr_data *expr1,
                                      t_expr_data *expr2,
                                      t_expr_data *expr3,
                                      t_token_location loc) {
    assert(op_is_ternary(op));
    t_expr_data *result = make_expr(loc);
    result->cat = EXPR_ternary;
    result->flags = 0;
    result->operation.cat = op;
    result->operation.expr1 = expr1;
    result->operation.expr2 = expr2;
    result->operation.expr3 = expr3;
    return result;
}

static t_expr_data *make_function_call(t_expr_data *callee,
                                       t_expr_list *parameters,
                                       t_token_location loc) {
    assert(callee != null);
    t_expr_data *result = make_expr(loc);
    result->cat = EXPR_function_call;
    result->flags = 0;
    result->func.callee = callee;
    result->func.parameters = parameters;
    return result;
}

static t_type_data *make_type_pointer_to(t_type_data *type) {
    assert(type != null);
    t_type_data *result = make_type();
    result->flags = 0;
    result->pointer_base = type;
    return result;
}

static t_type_data *make_type_slice_of(t_type_data *type) {
    assert(type != null);
    t_type_data *result = make_type();
    result->cat = TYPE_slice;
    result->flags = 0;
    result->slice_base = type;
    return result;
}

static t_type_data *make_type_function(t_type_data *return_type,
                                       t_decl_list *parameters) {
    assert(return_type != null);
    t_type_data *result = make_type();
    result->cat = TYPE_function;
    result->flags = 0;
    result->func.return_type = return_type;
    result->func.parameters = parameters;
    return result;
}

static t_stmt_data *make_if_stmt(t_expr_data *condition,
                                 t_stmt_data *true_branch,
                                 t_stmt_data *opt_false_branch,
                                 t_token_location loc) {
    assert(condition != null);
    t_stmt_data *result = make_stmt(loc);
    result->cat = STMT_if;
    result->if_data.condition = condition;
    result->if_data.true_branch = true_branch;
    result->if_data.false_branch = opt_false_branch;
    return result;
}

static t_stmt_data *make_while_stmt(t_expr_data *condition,
                                    t_stmt_data *block,
                                    t_token_location loc) {
    assert(condition != null);
    t_stmt_data *result = make_stmt(loc);
    result->cat = STMT_while;
    result->while_data.condition = condition;
    result->while_data.block = block;
    return result;
}

static t_stmt_data *make_expression_stmt(t_expr_data *expr, t_token_location loc) {
    assert(expr != null);
    t_stmt_data *result = make_stmt(loc);
    result->cat = STMT_expr;
    result->expr = expr;
    return result;
}

static t_stmt_data *make_decl_stmt(t_decl_data *decl, t_token_location loc) {
    assert(decl != null);
    t_stmt_data *result = make_stmt(loc);
    result->cat = STMT_decl;
    result->decl_data = decl;
    return result;
}

static t_stmt_data *make_return_stmt(t_expr_data *expr, t_token_location loc) {
    t_stmt_data *result = make_stmt(loc);
    result->cat = STMT_return;
    result->return_expr = expr;
    return result;
}

static t_stmt_data *make_print_stmt(t_expr_data *expr, t_token_location loc) {
    t_stmt_data *result = make_stmt(loc);
    result->cat = STMT_print;
    result->print_expr = expr;
    return result;
}

static t_stmt_data *make_stmt_cat(f_stmt_cat cat, t_token_location loc) {
    t_stmt_data *result = make_stmt(loc);
    result->cat = cat;
    return result;
}

static t_stmt_data *make_stmt_block(t_token_location loc) {
    t_stmt_data *result = make_stmt(loc);
    result->cat = STMT_block;
    result->block_data.first = null;
    result->block_data.last = null;
    result->block_data.count = 0;
    return result;
}

static void block_append_stmt(t_stmt_data *block, t_stmt_data *stmt) {
    assert(block->cat == STMT_block);
    stmt_list_push(&block->block_data, stmt);
}

static t_decl_data *make_decl_no_value(t_intern const *name, t_type_data *type) {
    assert(name != null);
    t_decl_data *result = make_decl();
    result->cat = DECL_no_value;
    result->name = name;
    result->type = type;
    return result;
}

static t_decl_data *make_decl_block_value(t_intern const *name, t_type_data *type, t_stmt_list *block) {
    assert(name != null);
    t_decl_data *result = make_decl();
    result->cat = DECL_block_value;
    result->name = name;
    result->type = type;
    result->block_data = block;
    return result;
}

static t_decl_data *make_decl_expr_value(t_intern const *name, t_type_data *type, t_expr_data *value) {
    assert(name != null);
    assert(value != null);
    t_decl_data *result = make_decl();
    result->cat = DECL_expr_value;
    result->name = name;
    result->type = type;
    result->value = value;
    return result;
}

// TODO(bumbread): special handling for aliases, when have them.
static bool can_dereference_type(t_type_data *type) {
    assert(type != null);
    return type->cat == TYPE_pointer;
}

static bool can_call_type(t_type_data *type) {
    assert(type != null);
    return type->cat == TYPE_function;
}

static bool can_subscript_type(t_type_data *type) {
    assert(type != null);
    return type->cat == TYPE_string || type->cat == TYPE_slice;
}

static bool can_slice_type(t_type_data *type) {
    assert(type != null);
    return type->cat == TYPE_string || type->cat == TYPE_slice;
}

static bool is_arithmetic_type(t_type_data *type) {
    assert(type != null);
    return type->cat == TYPE_int || type->cat == TYPE_float;
}

static bool is_logical_type(t_type_data *type) {
    assert(type != null);
    return type->cat == TYPE_bool;
}

// TODO(bumbread): actually follow the alias type traces when implemented
static bool are_types_arithmetical(t_type_data *type1, t_type_data *type2) {
    assert(type1 != null);
    assert(type2 != null);
    if(is_arithmetic_type(type1) && is_arithmetic_type(type2)) {
        return type1->cat == type2->cat;
    }
    return false;
}

// < > <= >=
static bool are_types_relational(t_type_data *type1, t_type_data *type2) {
    assert(type1 != null);
    assert(type2 != null);
    if(type1->cat == TYPE_pointer && type2->cat == TYPE_pointer) return true;
    if(type1->cat == TYPE_int && type2->cat == TYPE_int)         return true;
    if(type1->cat == TYPE_byte && type2->cat == TYPE_byte)       return true;
    if(type1->cat == TYPE_float && type2->cat == TYPE_float)     return true;
    return false;
}

// = !=
static bool are_types_comparable(t_type_data *type1, t_type_data *type2) {
    assert(type1 != null);
    assert(type2 != null);
    if(type1->cat == type2->cat) {
        return true;
    }
    return false;
}

static bool are_types_logical(t_type_data *type1, t_type_data *type2) {
    assert(type1 != null);
    assert(type2 != null);
    return type1->cat == TYPE_bool && type2->cat == TYPE_bool;
}

// TODO(bumbread): figure out why did i type the comment below
// TODO(bumbread): replace recusrive calls with another function
// e.g. types_are_equal
static bool can_assign_type_to_another(t_type_data *to, t_type_data *from) {
    assert(to != null);
    assert(from != null);
    switch(to->cat) {
        case TYPE_int: return from->cat == TYPE_int || from->cat == TYPE_byte;
        case TYPE_byte: return from->cat == TYPE_byte;
        case TYPE_bool: return from->cat == TYPE_bool;
        case TYPE_string: return from->cat == TYPE_string;
        case TYPE_float: return from->cat == TYPE_float;
        case TYPE_slice: return can_assign_type_to_another(to->slice_base, from->slice_base);
        case TYPE_pointer: return can_assign_type_to_another(to->pointer_base, from->pointer_base);
        case TYPE_function: {
            t_function_type_data *to_func = &to->func;
            t_function_type_data *from_func = &from->func;
            if(!can_assign_type_to_another(to_func->return_type, from_func->return_type)) {
                return false;
            }
            if(to_func->parameters->count != from_func->parameters->count) {
                return false;
            }
            
            t_decl_list_node *to_decl = to_func->parameters->first;
            t_decl_list_node *from_decl = from_func->parameters->first;
            for(;
                to_decl != null;
                to_decl = to_decl->next, from_decl = from_decl->next) {
                t_decl_data *to_param_decl = to_decl->data;
                t_decl_data *from_param_decl = from_decl->data;
                assert(to_param_decl != null);
                assert(from_param_decl != null);
                if(!can_assign_type_to_another(to_param_decl->type, from_param_decl->type)) {
                    return false;
                }
            }
            return true;
        }
        default: assert(false);
    }
    return false;
}

static char const *get_short_type_name(t_type_data *type) {
    assert(type != null);
    switch(type->cat) {
        case TYPE_int: return "int";
        case TYPE_byte: return "byte";
        case TYPE_bool: return "bool";
        case TYPE_string: return "string";
        case TYPE_float: return "float";
        case TYPE_pointer: return "pointer";
        case TYPE_slice: return "slice";
        case TYPE_function: return "function";
        default: assert(false);
    }
    return "???";
}
