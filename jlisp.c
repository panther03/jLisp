#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>
#include <editline/history.h>

#include "mpc.h"

long maximum(long x, long y, long z){
	if (x > y && x > z) {
		return x;
	}else if (y > x && y > z) {
		return y;
	}else {
		return z;
	}
}

long lisp_pow(long x, long y){
	int ans = 1;
	int i;
	for (i=1; i<=y; i++){
		ans = ans * x;
	}
	return ans;
}

long lisp_mod(long x, long y){
	int ans=x;
	while (ans > y) {
		ans -= y;
	}
	return ans;
}

typedef struct {
	int type;
	long num;
	int err;
}lval;

enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };
enum { LVAL_ERR, LVAL_NUM };

/* Create a new number type lval */
lval lval_num(long x) {
  lval v;
  v.type = LVAL_NUM;
  v.num = x;
  return v;
}

/* Create a new error type lval */
lval lval_err(int x) {
  lval v;
  v.type = LVAL_ERR;
  v.err = x;
  return v;
}

void lval_print(lval v){
	switch (v.type) {
		case LVAL_NUM: printf("%li",v.num); break;

		case LVAL_ERR:
			if (v.err == LERR_DIV_ZERO) {
				printf("Error: Division by Zero!");
			}
			if (v.err == LERR_BAD_OP) {
				printf("Error: Bad Operator");
			}
			if (v.err == LERR_BAD_NUM) {
				printf("Error: Invalid Number");
			}
		break;
	}
}

void lval_println(lval v) { lval_print(v); putchar('\n'); }

lval eval_op(lval x, char* op, lval y) {
                if (x.type=LVAL_ERR) { return x; }
		if (y.type=LVAL_ERR) { return y; }

		if (strcmp(op, "+") == 0) { return lval_num(x.num+y.num); }
		if (strcmp(op, "-") == 0) { return lval_num(x.num-y.num); }
		if (strcmp(op, "*") == 0) { return lval_num(x.num*y.num); }
		if (strcmp(op, "/") == 0) { return y.num == 0 ? lval_err(LERR_DIV_ZERO) : lval_num(x.num/y.num); }
		if (strcmp(op, "%") == 0) { return lval_num(lisp_mod(x.num,y.num)); }
		if (strcmp(op, "^") == 0) { return lval_num(lisp_pow(x.num,y.num)); }
		return lval_err(LERR_BAD_OP);
}

lval eval(mpc_ast_t* t) {
  	/* If tagged as number return it directly, otherwise expression. */
  	if (strstr(t->tag, "number")) { 
		errno = 0;
		long x = strtol(t->contents, NULL, 10);
		return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
	}

  	/* The operator is always second child. */
  	char* op = t->children[1]->contents;

  	/* We store the third child in `x` */
  	lval x = eval(t->children[2]);
	
  	/* Iterate the remaining children, combining using our operator */
  	int i = 3;
  	while (strstr(t->children[i]->tag, "expr")) {
    		x = eval_op(x, op, eval(t->children[i]));
    		i++;
 	}

  	return x;
}


int main(int argc, char** argv){
	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Operator = mpc_new("operator");
	mpc_parser_t* Expr = mpc_new("expr");
	mpc_parser_t* JLisp = mpc_new("jlisp");

	mpca_lang(MPCA_LANG_DEFAULT,
	"                                   \
     number : /-?[0-9]+/;				\
	 operator : '+' | '*' | '-' | '/'| '%' | '^';	\
	 expr : <number> | '(' <operator> <expr>+ ')';\
	 jlisp : <number> | /^/ <operator> <expr>+ /$/; \
	"
	,Number,Operator,Expr,JLisp);
	/*jlisp : <number> | /^/ <operator> <expr>+ /$/;\*/
	puts("JDC Lisp 1.0 (Press Ctrl-C to exit)");
        while (1){
		char* input = readline("jlisp> ");
		add_history(input);
		mpc_result_t r;
		if (mpc_parse("<stdin>", input, JLisp, &r)) {
		        lval result = eval(r.output);
			lval_println(result);
			mpc_ast_delete(r.output);
		}else {
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}
		free(input);
	}
	mpc_cleanup(4,Number,Operator,Expr,JLisp);
	return 0;
}
