#pragma once

#include "ast.h"

void print_decl(Decl* decl);
void print_expr(Expr *expr);
void print_expr_line(Expr *expr);
void print_stmt_block(StmtList block);
void print_stmt(Stmt *stmt);

void print_test();