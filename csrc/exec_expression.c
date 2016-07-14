#include <runtime.h>
#include <math.h>
#include <exec.h>


static value toggle (value x)
{
    if (x == efalse) return etrue;
    return efalse;
}



static CONTINUATION_5_3(do_equal, block, perf, execf, value, value,  heap, operator, value *); \
static void do_equal(block bk, perf p, execf n, value a, value b, heap h, operator op, value *r)
{
    start_perf(p);
    if ((op != op_flush)  && (op != op_close)) {                                                 
        value ar = lookup(r, a);
        value br = lookup(r, b);
        if (!value_equals(ar, br)) return;
    }
    apply(n, h, op, r);
    stop_perf(p);
}


#define DO_UNARY_TRIG(__name, __op)                                                               \
    static CONTINUATION_5_3(__name, block, perf, execf, value, value, heap, operator, value *); \
    static void __name (block b, perf p, execf n, value dest, value a, heap h, operator op, value *r) \
    {                                                                                                \
        start_perf(p);                                                     \
        if ((op == op_flush)  || (op == op_close)) {                                                 \
            apply(n, h, op, r);                                          \
            stop_perf(p);                                              \
            return;                                                     \
        }                                                                                            \
        value ar = lookup(r, a);                                                                     \
        if ((type_of(ar) != float_space )) {                                                         \
            exec_error(b->ev, "attempt to do math on non-number", a);                                   \
        } else {                                                                                     \
            double deg = *(double *)ar;                                                              \
            double rad = deg * M_PI/180.0;                                                           \
            r[reg(dest)] = box_float(__op(rad));                                                     \
            apply(n, h, op, r);                                          \
        }                                                                                            \
       stop_perf(p);                                              \
    }

#define DO_UNARY_BOOLEAN(__name, __op)                                                               \
    static CONTINUATION_5_3(__name, block, perf, execf, value, value, heap, operator, value *); \
    static void __name (block b, perf p, execf n, value dest, value a, heap h, operator op, value *r) \
    {                                                                                                \
        start_perf(p);\
        if ((op == op_flush)  || (op == op_close)) {                                                 \
            apply(n, h, op, r);                                          \
            stop_perf(p);                                              \
            return;                                                     \
        }                                                                                            \
        value ar = lookup(r, a);                                                                     \
        if ((ar != etrue) && (ar != efalse)) {                                                       \
            exec_error(b->ev, "attempt to flip non boolean", a);                                        \
        } else {                                                                                     \
            r[reg(dest)] = __op(ar);                                                                 \
            apply(n, h, op, r);                                          \
        }                                                                                            \
        stop_perf(p);                                              \
    }


#define DO_UNARY_NUMERIC(__name, __op)                                                               \
    static CONTINUATION_5_3(__name, block, perf, execf, value, value, heap, operator, value *); \
    static void __name (block b, perf p, execf n, value dest, value a, heap h, operator op, value *r) \
    {                                                                                                \
        start_perf(p);                                                     \
        if ((op == op_flush)  || (op == op_close)) {                                                 \
            apply(n, h, op, r);                                          \
            stop_perf(p);                                              \
            return;                                                     \
        }                                                                                            \
        value ar = lookup(r, a);                                                                     \
        if ((type_of(ar) != float_space )) {                                                         \
            exec_error(b->e, "attempt to do math on non-number", a);                                   \
        } else {                                                                                     \
            r[reg(dest)] = box_float(__op(*(double *)ar));                                           \
            apply(n, h, op, r);                                          \
        }                                                                                            \
        stop_perf(p);                                              \
    }

#define BUILD_UNARY(__name, __do_op)   \
    static execf __name (block bk, node n)  \
    {                                           \
        vector a = vector_get(n->arguments, 0); \
        return cont(bk->h,                       \
                __do_op,                        \
                bk,                              \
                register_perf(bk->ev, n),         \
                resolve_cfg(bk, n, 0),          \
                vector_get(a, 0),    \
                vector_get(a, 1));   \
    }


#define DO_BINARY_NUMERIC(__name, __op)                                                              \
    static CONTINUATION_6_3(__name, block, perf, execf, value, value, value,  heap, operator, value *); \
    static void __name (block bk, perf p, execf n, value dest, value a, value b, heap h, operator op, value *r) \
    {                                                                                                \
        start_perf(p);\
        if ((op == op_flush)  || (op == op_close)) {                                                 \
            apply(n, h, op, r);                                          \
            stop_perf(p);                                              \
            return;                                                    \
        }       \
        value ar = lookup(r, a);                                                                     \
        value br = lookup(r, b);                                                                     \
        if ((type_of(ar) != float_space ) || (type_of(br) != float_space)) {                         \
            exec_error(bk->ev, "attempt to " #__name" non-numbers", a, b);                            \
            prf("UHOH %v, %v\n", ar, br);                                                            \
        } else {                                                                                     \
            r[reg(dest)] = box_float(*(double *)ar __op *(double *)br);                              \
            apply(n, h, op, r);                                          \
        }                                                                                            \
        stop_perf(p);\
    }

#define DO_BINARY_BOOLEAN(__name, __op)                                                                \
    static CONTINUATION_6_3(__name, block, perf, execf, value, value, value, heap, operator, value *); \
    static void __name (block bk, perf p, execf n, value dest, value a, value b, heap h, operator op, value *r) \
    {                                                                                                  \
         start_perf(p);\
        if ((op == op_flush) || (op == op_close)) {                                                 \
            apply(n, h, op, r);                                          \
            stop_perf(p);                                              \
            return;                                                     \
        }                                                                                            \
        value ar = lookup(r, a);                                                                     \
        value br = lookup(r, b);                                                                     \
        \
        if ((type_of(ar) == float_space ) && (type_of(br) == float_space)) {                         \
            r[reg(dest)] = (*(double *)ar __op *(double *)br) ? etrue : efalse;                      \
            apply(n, h, op, r);                                          \
        } else if ((type_of(ar) == estring_space ) && (type_of(br) == estring_space)) {              \
            r[reg(dest)] = (ar __op br) ? etrue : efalse;                                            \
            apply(n, h, op, r);                                          \
        } else if ((type_of(ar) == uuid_space ) && (type_of(br) == uuid_space)) {                    \
            r[reg(dest)] = (ar __op br) ? etrue : efalse;                                            \
            apply(n, h, op, r);                                          \
        } else if ((ar == etrue || ar == efalse ) && (br == etrue || br == efalse)) {                \
            r[reg(dest)] = (ar __op br) ? etrue : efalse;                                            \
            apply(n, h, op, r);                                          \
        } else {                                                                                     \
            exec_error(bk->ev, "attempt to __op different types", a, b);                              \
        }                                                                                            \
       stop_perf(p);\
    }


#define BUILD_BINARY(__name, __do_op)   \
    static execf __name (block bk, node n)  \
    {                                           \
        vector a = vector_get(n->arguments, 0); \
        return cont(bk->h,                   \
                __do_op,                        \
                bk,                             \
                register_perf(bk->ev, n),     \
                resolve_cfg(bk, n, 0),           \
                vector_get(a, 0),    \
                vector_get(a, 1),    \
                vector_get(a, 2));   \
    }


#define DO_BINARY_FILTER(__name, __op)                                                               \
    static CONTINUATION_5_3(__name, block, perf, execf, value, value,  heap, operator, value *); \
    static void __name (block bk, perf p, execf n, value a, value b, heap h, operator op, value *r) \
    {                                                                                                \
        start_perf(p);                                                     \
        if ((op == op_flush)  || (op == op_close)) {                                                 \
            apply(n, h, op, r);                                                                      \
            stop_perf(p);\
            return;                                                                                  \
        }                                                                                            \
        value ar = lookup(r, a);                                                                     \
        value br = lookup(r, b);                                                                     \
        if ((type_of(ar) == float_space ) && (type_of(br) == float_space)) {                         \
            if (*(double *)ar __op *(double *)br)                                                    \
                apply(n, h, op, r);                                      \
        } else if ((type_of(ar) == estring_space ) && (type_of(br) == estring_space)) {              \
            if (ar __op br)                                                                          \
                apply(n, h, op, r);                                      \
        } else if ((type_of(ar) == uuid_space ) && (type_of(br) == uuid_space)) {                    \
            if (ar __op br)                                                                          \
                apply(n, h, op, r);                                      \
        } else if ((ar == etrue || ar == efalse ) && (br == etrue || br == efalse)) {                \
            if (ar __op br)                                                                          \
                apply(n, h, op, r);                                      \
        } else {                                                                                     \
            exec_error(bk->ev, "attempt to __op different types", a, b);                                 \
        }                                                                                            \
        stop_perf(p);                                                  \
    }


#define BUILD_BINARY_FILTER(__name, __do_op)   \
    static execf __name (block bk, node n)  \
    {                                           \
        vector a = vector_get(n->arguments, 0); \
        return cont(bk->h,                       \
                __do_op,                        \
                bk,                              \
                register_perf(bk->ev, n),         \
                resolve_cfg(bk, n, 0),           \
                vector_get(a, 0),    \
                vector_get(a, 1));   \
    }



DO_UNARY_TRIG(do_sin, sin)
BUILD_UNARY(build_sin, do_sin)

DO_UNARY_TRIG(do_cos, cos)
BUILD_UNARY(build_cos, do_cos)

DO_UNARY_TRIG(do_tan, tan)
BUILD_UNARY(build_tan, do_tan)

DO_UNARY_BOOLEAN(do_toggle, toggle)
BUILD_UNARY(build_toggle, do_toggle)

DO_BINARY_NUMERIC(do_plus, +)
BUILD_BINARY(build_plus, do_plus)

DO_BINARY_NUMERIC(do_minus, -)
BUILD_BINARY(build_minus, do_minus)

DO_BINARY_NUMERIC(do_multiply, *)
BUILD_BINARY(build_multiply, do_multiply)

DO_BINARY_NUMERIC(do_divide, /)
BUILD_BINARY(build_divide, do_divide)

DO_BINARY_FILTER(do_less_than, <)
BUILD_BINARY_FILTER(build_less_than, do_less_than)
DO_BINARY_BOOLEAN(do_is_less_than, <)
BUILD_BINARY(build_is_less_than, do_is_less_than)

DO_BINARY_FILTER(do_less_than_or_equal, <=)
BUILD_BINARY_FILTER(build_less_than_or_equal, do_less_than_or_equal)
DO_BINARY_BOOLEAN(do_is_less_than_or_equal, <=)
BUILD_BINARY(build_is_less_than_or_equal, do_is_less_than_or_equal)

DO_BINARY_FILTER(do_greater_than, >)
BUILD_BINARY_FILTER(build_greater_than, do_greater_than)
DO_BINARY_BOOLEAN(do_is_greater_than, >)
BUILD_BINARY(build_is_greater_than, do_is_greater_than)

DO_BINARY_FILTER(do_greater_than_or_equal, >=)
BUILD_BINARY_FILTER(build_greater_than_or_equal, do_greater_than_or_equal)
DO_BINARY_BOOLEAN(do_is_greater_than_or_equal, >=)
BUILD_BINARY(build_is_greater_than_or_equal, do_is_greater_than_or_equal)

// @TODO: make assign do its job instead of just filtering
//DO_BINARY_FILTER(do_equal, ==)
BUILD_BINARY_FILTER(build_equal, do_equal)
DO_BINARY_BOOLEAN(do_is_equal, ==)
BUILD_BINARY(build_is_equal, do_is_equal)

DO_BINARY_FILTER(do_not_equal, !=)
BUILD_BINARY_FILTER(build_not_equal, do_not_equal)
DO_BINARY_BOOLEAN(do_is_not_equal, !=)
BUILD_BINARY(build_is_not_equal, do_is_not_equal)

static CONTINUATION_5_3(do_is, block, perf, execf, value, value, heap, operator, value *);
static void do_is (block bk, perf p, execf n, value dest, value a, heap h, operator op, value *r)
{
    start_perf(p);
    r[reg(dest)] = lookup(r, a);
    stop_perf(p);
    apply(n, h, op, r);
}

BUILD_UNARY(build_is, do_is)


void register_exec_expression(table builders)
{
    table_set(builders, intern_cstring("plus"), build_plus);
    table_set(builders, intern_cstring("minus"), build_minus);
    table_set(builders, intern_cstring("multiply"), build_multiply);
    table_set(builders, intern_cstring("divide"), build_divide);
    table_set(builders, intern_cstring("less_than"), build_less_than);
    table_set(builders, intern_cstring("less_than_or_equal"), build_less_than_or_equal);
    table_set(builders, intern_cstring("greater_than"), build_greater_than);
    table_set(builders, intern_cstring("greater_than_or_equal"), build_greater_than_or_equal);
    table_set(builders, intern_cstring("equal"), build_equal);
    table_set(builders, intern_cstring("not_equal"), build_not_equal);
    table_set(builders, intern_cstring("is"), build_is);
    table_set(builders, intern_cstring("is_less_than"), build_is_less_than);
    table_set(builders, intern_cstring("is_less_than_or_equal"), build_is_less_than_or_equal);
    table_set(builders, intern_cstring("is_greater_than"), build_is_greater_than);
    table_set(builders, intern_cstring("is_greater_than_or_equal"), build_is_greater_than_or_equal);
    table_set(builders, intern_cstring("is_equal"), build_is_equal);
    table_set(builders, intern_cstring("is_not_equal"), build_is_not_equal);
    table_set(builders, intern_cstring("sin"), build_sin);
    table_set(builders, intern_cstring("cos"), build_cos);
    table_set(builders, intern_cstring("tan"), build_tan);
    table_set(builders, intern_cstring("toggle"), build_toggle);
}
