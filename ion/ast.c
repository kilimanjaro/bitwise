#include "ast.h"
#include "common.h"
#include "lex.h"

#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

Arena ast_arena;

#define AST_DUP(x) ast_dup(x, num_##x * sizeof(*x))

void *ast_alloc(size_t size) {
    assert(size != 0);
    void *ptr = arena_alloc(&ast_arena, size);
    memset(ptr, 0, size);
    return ptr;
}

void *ast_dup(const void *src, size_t size) {
    if (size == 0) {
        return NULL;
    }
    void *ptr = arena_alloc(&ast_arena, size);
    memcpy(ptr, src, size);
    return ptr;
}

StmtBlock ast_dup_block(StmtBlock block) {
    return (StmtBlock){ast_dup(block.stmts, block.num_stmts * sizeof(*block.stmts)), block.num_stmts};
}

TypeSpec *typespec_new(TypeSpecKind kind) {
    TypeSpec *t = ast_alloc(sizeof(TypeSpec));
    t->kind = kind;
    return t;
}

TypeSpec *typespec_name(const char* name) {
    TypeSpec *t = typespec_new(TYPESPEC_NAME);
    t->name = name;
    return t;
}

TypeSpec *typespec_ptr(TypeSpec *elem) {
    TypeSpec *t = typespec_new(TYPESPEC_PTR);
    t->ptr.elem = elem;
    return t;
}

TypeSpec *typespec_array(TypeSpec *elem, Expr *size) {
    TypeSpec *t = typespec_new(TYPESPEC_ARRAY);
    t->array.elem = elem;
    t->array.size = size;
    return t;
}

TypeSpec *typespec_func(TypeSpec **args, size_t num_args, TypeSpec *ret) {
    TypeSpec *t = typespec_new(TYPESPEC_FUNC);
    t->func.args = AST_DUP(args);
    t->func.num_args = num_args;
    t->func.ret = ret;
    return t;
}

Decl *decl_new(DeclKind kind, const char *name) {
    Decl *d = ast_alloc(sizeof(Decl));
    d->kind = kind;
    d->name = name;
    return d;
}

Decl *decl_enum(const char *name, EnumItem *items, size_t num_items) {
    Decl *d = decl_new(DECL_ENUM, name);
    d->enum_decl.items = AST_DUP(items);
    d->enum_decl.num_items = num_items;
    return d;
}

Decl *decl_aggregte(DeclKind kind, const char *name, AggregateItem *items, size_t num_items) {
    assert(kind == DECL_STRUCT || kind == DECL_UNION);
    Decl *d = decl_new(kind, name);
    d->aggregate.items = AST_DUP(items);
    d->aggregate.num_items = num_items;
    return d;
}

Decl *decl_union(const char *name, AggregateItem *items, size_t num_items) {
    Decl *d = decl_new(DECL_UNION, name);
    d->aggregate.items = AST_DUP(items);
    d->aggregate.num_items = num_items;
    return d;
}

Decl *decl_var(const char *name, TypeSpec *type, Expr *expr) {
    Decl *d = decl_new(DECL_VAR, name);
    d->var.type = type;
    d->var.expr = expr;
    return d;
}

Decl *decl_func(const char *name, FuncParam *params, size_t num_params, TypeSpec *ret_type, StmtBlock block) {
    Decl *d = decl_new(DECL_FUNC, name);
    d->func.params = AST_DUP(params);
    d->func.num_params = num_params;
    d->func.ret_type = ret_type;
    d->func.block = ast_dup_block(block);
    return d;
}

Decl *decl_const(const char *name, Expr *expr) {
    Decl *d = decl_new(DECL_CONST, name);
    d->const_decl.expr = expr;
    return d;
}

Decl *decl_typedef(const char *name, TypeSpec *type) {
    Decl *d = decl_new(DECL_TYPEDEF, name);
    d->typedef_decl.type = type;
    return d;
}


Expr *expr_new(ExprKind kind) {
    Expr *e = ast_alloc(sizeof(Expr));
    e->kind = kind;
    return e;
}

Expr *expr_int(int64_t int_val) {
    Expr *e = expr_new(EXPR_INT);
    e->int_val = int_val;
    return e;
}

Expr *expr_float(double float_val) {
    Expr *e = expr_new(EXPR_FLOAT);
    e->float_val = float_val;
    return e;
}

Expr *expr_str(const char *str) {
    Expr *e = expr_new(EXPR_STR);
    e->str_val = str;
    return e;
}

Expr *expr_name(const char *name) {
    Expr *e = expr_new(EXPR_NAME);
    e->name = name;
    return e;
}

Expr *expr_call(Expr *expr, Expr **args, size_t num_args) {
    Expr *e = expr_new(EXPR_CALL);
    e->call.expr = expr;
    e->call.args = AST_DUP(args);
    e->call.num_args = num_args;
    return e;
}

Expr *expr_index(Expr *expr, Expr *index) {
    Expr *e = expr_new(EXPR_INDEX);
    e->index.expr = expr;
    e->index.index = index;
    return e;
}

Expr *expr_field(Expr *expr, const char *name) {
    Expr *e = expr_new(EXPR_FIELD);
    e->field.expr = expr;
    e->field.name = name;
    return e;
}

Expr *expr_cast(TypeSpec *cast_type, Expr *cast) {
    Expr *e = expr_new(EXPR_CAST);
    e->cast.type = cast_type;
    e->cast.expr = cast;
    return e;
}

Expr *expr_compound(TypeSpec *type, Expr **args, size_t num_args) {
    Expr *e = expr_new(EXPR_COMPOUND);
    e->compound.type = type;
    e->compound.args = AST_DUP(args);
    e->compound.num_args = num_args;
    return e;
}
 
Expr *expr_unary(TokenKind op, Expr *expr) {
    Expr *e = expr_new(EXPR_UNARY);
    e->unary.op = op;
    e->unary.expr = expr;
    return e;
}

Expr *expr_binary(TokenKind op, Expr *left, Expr *right) {
    Expr *e = expr_new(EXPR_BINARY);
    e->binary.op = op;
    e->binary.left = left;
    e->binary.right = right;
    return e;
}

Expr *expr_ternary(Expr *cond, Expr *then_expr, Expr *else_expr) {
    Expr *e = expr_new(EXPR_TERNARY);
    e->ternary.cond = cond;
    e->ternary.then_expr = then_expr;
    e->ternary.else_expr = else_expr;
    return e;
}

Expr *expr_sizeof_expr(Expr *expr) {
    Expr *e = expr_new(EXPR_SIZEOF_EXPR);
    e->sizeof_expr = expr;
    return e;
}

Expr *expr_sizeof_type(TypeSpec *type) {
    Expr *e = expr_new(EXPR_SIZEOF_TYPE);
    e->sizeof_type = type;
    return e;
}


Stmt *stmt_new(StmtKind kind) {
    Stmt *s = ast_alloc(sizeof(Stmt));
    s->kind = kind;
    return s;
}

Stmt *stmt_decl(Decl *decl) {
    Stmt *s = stmt_new(STMT_DECL);
    s->decl = decl;
    return s;
}

Stmt *stmt_return(Expr *expr) {
    Stmt *s = stmt_new(STMT_RETURN);
    s->return_stmt.expr = expr;
    return s;
}

Stmt *stmt_break() {
    return stmt_new(STMT_BREAK);
}

Stmt *stmt_continue() {
    return stmt_new(STMT_CONTINUE);
}

Stmt *stmt_block(StmtBlock block) {
    Stmt *s = stmt_new(STMT_BLOCK);
    s->block = block;
    return s;
}

Stmt *stmt_if(Expr *cond, StmtBlock then_block, ElseIf *elseifs, size_t num_elseifs, StmtBlock else_block) {
    Stmt *s = stmt_new(STMT_IF);
    s->if_stmt.cond = cond;
    s->if_stmt.then_block = ast_dup_block(then_block);
    s->if_stmt.elseifs = AST_DUP(elseifs);
    s->if_stmt.num_elseifs = num_elseifs;
    s->if_stmt.else_block = ast_dup_block(else_block);
    return s;
}

Stmt *stmt_while(Expr *cond, StmtBlock block) {
    Stmt *s = stmt_new(STMT_WHILE);
    s->while_stmt.cond = cond;
    s->while_stmt.block = ast_dup_block(block);
    return s;
}

Stmt *stmt_do_while(Expr *cond, StmtBlock block) {
    Stmt *s = stmt_new(STMT_DO_WHILE);
    s->while_stmt.cond = cond;
    s->while_stmt.block = ast_dup_block(block);
    return s;
}

Stmt *stmt_for(Stmt* init, Expr *cond, Stmt* next, StmtBlock block) {
    Stmt *s = stmt_new(STMT_FOR);
    s->for_stmt.init = init;
    s->for_stmt.cond = cond;
    s->for_stmt.next = next;
    s->for_stmt.block = ast_dup_block(block);
    return s;
}

Stmt *stmt_switch(Expr *expr, SwitchCase *cases, size_t num_cases) {
    Stmt *s = stmt_new(STMT_SWITCH);
    s->switch_stmt.expr = expr;
    s->switch_stmt.cases = AST_DUP(cases);
    s->switch_stmt.num_cases = num_cases;
    return s;
}

Stmt *stmt_assign(TokenKind op, Expr *left, Expr *right) {
    Stmt *s = stmt_new(STMT_ASSIGN);
    s->assign.op = op;
    s->assign.left = left;
    s->assign.right = right;
    return s;
}

Stmt *stmt_init(const char *name, Expr *expr) {
    Stmt *s = stmt_new(STMT_INIT);
    s->init.name = name;
    s->init.expr = expr;
    return s;
}

Stmt *stmt_expr(Expr *expr) {
    Stmt *s = stmt_new(STMT_EXPR);
    s->expr = expr;
    return s;
}

#undef AST_DUP