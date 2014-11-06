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

int main(){	
	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Operator = mpc_new("operator");
	mpc_parser_t* Expr = mpc_new("expr");
	mpc_parser_t* JLisp = mpc_new("jlisp");

	mpca_lang(MPCA_LANG_DEFAULT,
	"						\
	 number : /-?[0-9]+/;				\
	 operator : '+' | '*' | '-' | '/'| '%' | '^';	\
	 expr : <number> | '(' <operator> <expr>+ ')';\ 
	 jlisp : <number> | /^/ <operator> <expr>+ /$/;\
	",Number,Operator,Expr,JLisp);
	puts("JDC Lisp 1.0");
	long eval_op(long x, char* op, long y) {
		if (strcmp(op, "+") == 0) { return x+y; }
		if (strcmp(op, "-") == 0) { return x-y; }
		if (strcmp(op, "*") == 0) { return x*y; }
		if (strcmp(op, "/") == 0) { return x/y; }
		if (strcmp(op, "%") == 0) { return lisp_mod(x,y); }
		if (strcmp(op, "^") == 0) { return lisp_pow(x,y); }
	}
	long eval(mpc_ast_t* t) {
  	/* If tagged as number return it directly, otherwise expression. */ 
  	if (strstr(t->tag, "number")) { return atoi(t->contents); }
  
  	/* The operator is always second child. */
  	char* op = t->children[1]->contents;
  
  	/* We store the third child in `x` */
  	long x = eval(t->children[2]);
  
  	/* Iterate the remaining children, combining using our operator */
  	int i = 3;
  	while (strstr(t->children[i]->tag, "expr")) {
    		x = eval_op(x, op, eval(t->children[i]));
    		i++;
 	 }
  
  	return x;  
	}
	while (1){
		char* input = readline("jlisp> ");
		add_history(input);
		mpc_result_t r;
		if (mpc_parse("<stdin>", input, JLisp, &r)) {
			long outcome = eval(r.output);
			printf("%li \n",outcome);
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
