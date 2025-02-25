#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_L_EX 101

// Structure definition
typedef struct {
    double item[MAX_L_EX];
    int top;
} Stack;

typedef struct {
    bool is_num;
    union {
        double num;
        char op;
    } value;
} Expression_elements;

typedef enum {
    TOKEN_NUMBER,
    TOKEN_PLUS,
    TOKEN_DIVIDE,
    TOKEN_MULTIPLY,
    TOKEN_MINUS,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_END,
    TOKEN_INVALID
} Token_type;

typedef struct {
    Token_type type;
} Token;

// Function prototypes
void get_infix_ex(Expression_elements *infix_ex);
Token get_token(Expression_elements infix_ex);
bool parse_expression(const Expression_elements *infix_ex, int *index_infix_ex);
bool parse_term(const Expression_elements *infix_ex, int *index_infix_ex);
bool parse_factor(const Expression_elements *infix_ex, int *index_infix_ex);
void turn_to_rpn(const Expression_elements *infix_ex, Expression_elements *rpn);
void stack_init(Stack *s);
bool is_empty(const Stack *s);
bool is_full(const Stack *s);
double check_top(const Stack *s);
double pop(Stack *s);
void push(Stack *s, double value);
double apply_operators(double a, double b, char operator);
bool is_digit(char ch);
int lv_op(char op);
double compute_rpn(const Expression_elements *rpn);

int main(void) {
    Expression_elements infix_ex[MAX_L_EX],
                        rpn[MAX_L_EX];

    printf("A simple calculator!\n");
    printf("Input your expression:\n");

    get_infix_ex(infix_ex);
    turn_to_rpn(infix_ex, rpn);
    printf("The answer is %lf",compute_rpn(rpn));

    return 0;
}

void get_infix_ex(Expression_elements *infix_ex) {
    // This function is to get the infix_ex from stdin.
    //
    // @param infix_ex: the address of infix_ex structure array.
    // @return: none.

    // Check if the address of infix_ex array is a NULL pointer.
    if (infix_ex==NULL) {
        fprintf(stderr, "ERROR: The infix_ex pointer not found");
        exit(EXIT_FAILURE);
    }

    char ch=0;
    int index_infix_ex=0;
    bool last_is_digit=false;
    int last_num=0;
    
    while((ch=(char)getchar())!=EOF) {
        if(index_infix_ex==MAX_L_EX) {
            fprintf(stderr, "ERROR: Expression overflow!");
            exit(1);
        }

        if(ch==' ') {
            continue;
        }

        if(is_digit(ch)) {
            if(last_is_digit) {
                last_num=last_num*10+ch-'0';
            } else {
                last_is_digit=true;
                last_num=ch-'0';
            }
        } else if (ch=='\n') {
            if(last_is_digit) {
                infix_ex[index_infix_ex].is_num=true;
                infix_ex[index_infix_ex++].value.num=last_num;
                break;
            }
            break;
        } else if(last_is_digit) {
            infix_ex[index_infix_ex].is_num=true;
            infix_ex[index_infix_ex++].value.num=last_num;

            infix_ex[index_infix_ex].is_num=false;
            infix_ex[index_infix_ex++].value.op=ch;

            last_is_digit=false;
        } else {
            infix_ex[index_infix_ex].is_num=false;
            infix_ex[index_infix_ex++].value.op=ch;
        }
    }

    // Add '\0' as the end. 
    infix_ex[index_infix_ex].is_num=false;
    infix_ex[index_infix_ex++].value.op='\0';

    // Debugging expressions
    for(int i=0 ; infix_ex[index_infix_ex].is_num && infix_ex[index_infix_ex].value.op=='\0' ; i++) {
        if(infix_ex[i].is_num) {
            printf("%.0f",infix_ex[i].value.num);
        } else {
            printf("%c",infix_ex[i].value.op);
        }
    } printf("\n");
}

Token get_token(const Expression_elements infix_ex) {
    // This function is used to get a char from infix expression and turn it into a token.(working as a Lexer)
    // @param infix_ex: an element of the infix expression struct array.
    // @return token: the token wo get from infix expression.
    if(infix_ex.is_num) {
        return (Token){TOKEN_NUMBER};
    } else {
        Token token;
        switch(infix_ex.value.op) {
            case '+': token.type=TOKEN_PLUS; break;
            case '-': token.type=TOKEN_MINUS; break;
            case '*': token.type=TOKEN_MULTIPLY; break;
            case '/': token.type=TOKEN_DIVIDE; break;
            case '\0': token.type=TOKEN_END; break;
            // Process the invalid token.
            default: token.type=TOKEN_INVALID;
                     fprintf(stderr, "ERROR: Invalid token %c", token.type);
                     exit(1);
        }
        return token;
    }
}

// TODO: Yesterday the last thing I did is adding '\0' to the tail of expressions as the end, then using this '\0' to determine if the expression has ended.
// The next thing I should do is complete the parse. (Note that the token_type enum contains sth I haven't use, like TOKEN_END)
// And notice the ending condition of parse.
bool parse_expression(const Expression_elements *infix_ex, int *index_infix_ex) {
    if(!parse_term(infix_ex, index_infix_ex)) {
        fprintf(stderr, "ERROR: Expected term.");
        exit(1);
    }
    Token token=get_token(infix_ex[*index_infix_ex]); (*index_infix_ex)++;
    while(token.type==TOKEN_PLUS || token.type==TOKEN_MINUS) {
        if(!parse_term(infix_ex, index_infix_ex)) {
            fprintf(stderr, "ERROR: Expected term.");
            exit(1);
        }
        token=get_token(infix_ex[*index_infix_ex]); (*index_infix_ex)++;
    }
    (*index_infix_ex)--;
    return true;
}

bool parse_term(const Expression_elements *infix_ex, int *index_infix_ex) {
    if(!parse_factor(infix_ex, index_infix_ex)) {
        fprintf(stderr, "ERROR: Expected factor.");
        exit(1);
    }
    Token token=get_token(infix_ex[*index_infix_ex]); (*index_infix_ex)++;
    while(token.type==TOKEN_MULTIPLY || token.type==TOKEN_DIVIDE) {
        if(!parse_factor(infix_ex, index_infix_ex)) return false;
        token=get_token(infix_ex[*index_infix_ex]); (*index_infix_ex)++;
    }
    (*index_infix_ex)--;
    return true;
}

bool parse_factor(const Expression_elements *infix_ex, int *index_infix_ex) {
    Token token=get_token(infix_ex[*index_infix_ex]); (*index_infix_ex)++;
    if(token.type==TOKEN_LPAREN) {
        if(!parse_expression(infix_ex, index_infix_ex)) return false;
        token=get_token(infix_ex[*index_infix_ex]); (*index_infix_ex)++;
        if(token.type!=TOKEN_RPAREN) {
            fprintf(stderr, "ERROR: Expected ')'.");
            exit(1);
        }
    } else if(token.type!=TOKEN_NUMBER) {
        return false;
    }
    return true;
}

// Stack operation functions

void stack_init(Stack *s) {
    s->top=-1;
}

bool is_empty(const Stack *s) {
    return s->top==-1?true:false;
}

bool is_full(const Stack *s) {
    return s->top==MAX_L_EX-1?true:false;
}

double check_top(const Stack *s) {
    return s->item[s->top];
}

double pop(Stack *s) {
    if(is_empty(s)) {
        fprintf(stderr, "ERROR: Stack underflow!");
        exit(1);
    }
    return s->item[(s->top)--];
}

void push(Stack *s, const double value) {
    if(is_full(s)) {
        fprintf(stderr, "ERROR: Stack overflow!");
        exit(1);
    }
    s->item[++(s->top)]=value;
}

double apply_operators(const double a, const double b, const char operator) {
    double ret=0;
    switch(operator) {
        case '+': ret=a+b; break;
        case '-': ret=a-b; break;
        case '*': ret=a*b; break;
        case '/': ret=a*1.0/b; break;
        default: fprintf(stderr, "ERROR: Operator cannot be figured out");
                 exit(1);
    }
    return ret;
}

bool is_digit(const char ch) {
    if(ch>='0' && ch<='9') {
        return true;
    } else {
        return false;
    }
}

int lv_op(const char op) {
    int level=0;
    switch(op) {
        case '+':
        case '-': level=1; break;
        case '*':
        case '/': level=2; break;
        default: fprintf(stderr, "ERROR: Operator cannot be figured out");
                 exit(1);
    }
    return level;
}

void turn_to_rpn(const Expression_elements *infix_ex, Expression_elements *rpn) {
    // This function is to turn the infix expression into rpn(reserve polish notation).
    // @param infix_ex: the address of infix expression array.
    // @param rpn: the address of rpn array.
    // @return: none.

    Stack stack;
    stack_init(&stack);

    int index_rpn=0;

    // Using scheduling algorithm to turn the infix expression to reversed poland notation.
    for (int index_infix_ex=0;;index_infix_ex++) {
        // Process the numbers.
        if(infix_ex[index_infix_ex].is_num) {
            rpn[index_rpn].is_num=true;
            rpn[index_rpn++].value.num=infix_ex[index_infix_ex].value.num;
            printf("%.0f ",rpn[index_rpn-1].value.num);
        } else {
            // The ending condition.
            if(infix_ex[index_infix_ex].value.op=='\0') break;

            // Process the operators.
            if(is_empty(&stack) || check_top(&stack)=='(' || infix_ex[index_infix_ex].value.op=='(') {
                push(&stack, infix_ex[index_infix_ex].value.op);
            } else if(infix_ex[index_infix_ex].value.op==')') {
                char tmp;
                while((tmp=(char)pop(&stack))!='(') {
                    rpn[index_rpn].is_num=false;
                    rpn[index_rpn++].value.op=tmp;
                    printf("%c ",rpn[index_rpn-1].value.op);
                }
            } else if(lv_op(infix_ex[index_infix_ex].value.op) > lv_op((char)check_top(&stack))) {
                push(&stack, infix_ex[index_infix_ex].value.op);
            } else {
                while(!is_empty(&stack) && check_top(&stack)!='(' && lv_op((char)check_top(&stack)) >= lv_op(infix_ex[index_infix_ex].value.op)) {
                    rpn[index_rpn].is_num=false;
                    rpn[index_rpn++].value.op=(char)pop(&stack);
                    printf("%c ",rpn[index_rpn-1].value.op);
                }
                push(&stack, infix_ex[index_infix_ex].value.op);
            }
        }
    }

    // Clear the stack (pop out the remaining operators in the stack).
    while(!is_empty(&stack)) {
        rpn[index_rpn].is_num=false;
        rpn[index_rpn++].value.op=(char)pop(&stack);
        printf("%c ",rpn[index_rpn-1].value.op);
    }

    printf("\n");

    // Add '\0' as the end.
    rpn[index_rpn].is_num=false;
    rpn[index_rpn++].value.op='\0';

    // For debugging
    for(int i=0;i<index_rpn;i++) {
        if(rpn[i].is_num) {
            printf("%.0f ",rpn[i].value.num);
        } else {
            printf("%c ",rpn[i].value.op);
        }
    }
    printf("\n");
}

double compute_rpn(const Expression_elements *rpn) {
    // This function is to figure out the result of the rpn.
    // @param rpn: The address of rpn.
    // @return ret: The result of computation.

    double ret=0;

    Stack stack;
    stack_init(&stack);

    for(int index_rpn = 0;;index_rpn++) {
        if(rpn[index_rpn].is_num) {
            push(&stack, rpn[index_rpn].value.num);

        // Ending condition.
        } else if(rpn[index_rpn].value.op=='\0') {
            break;

        } else if(!is_empty(&stack)) {
            push(&stack, apply_operators(pop(&stack), pop(&stack), rpn[index_rpn].value.op));
        }
    }

    ret=pop(&stack);

    return ret;
}





















