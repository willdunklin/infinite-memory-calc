#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>

// constants
#define EOS	257
#define NUM	258

// Block of 4 digits
struct block {
    struct block* next;
    struct block* prev;
    int val;
    int exp;
};

struct block* create_block(int val, int exp) {
    struct block* new = (struct block*)malloc(sizeof(struct block));
    new->next = NULL;
    new->val = val;
    new->exp = exp;
    return new;
}

struct block* zero(int exp) {
    return create_block(0, exp);
}

struct list {
    struct list* next;
    struct block* data;
};

struct list* append(struct list* prev, struct block* block) {
    struct list* new = (struct list*)malloc(sizeof(struct list));
    new->next = NULL;
    new->data = block;
    prev->next = new;
    return new;
}

void carry(struct block** a, int value, int exp) {
    if(*a == NULL) {
        *a = create_block(value, exp);
        return;
    }
    value += (*a)->val;
    // Recursive carry
    if(value >= 10000) {
        value /= 10000;
        (*a)->val %= 10000;
        carry(&((*a)->next), value, (*a)->exp+1);
    }
}

// Add block b to a
void add(struct block* a, struct block* b) {
    if(a == NULL && b == NULL)
        return;

    if(a == NULL)
        create_block(b->val, b->exp);

    else if(b != NULL) {
        // If both have a block
        a->val += b->val;
        // Carry nothing extra (handles if there's overflow)
        carry(&a, 0, a->exp);
        // Add the next blocks
        add(a->next, b->next);
    }
}

void mult(struct block* a, struct block* b) {
    struct list *head = (struct list*)malloc(sizeof(struct list)), *next = head;

    while(b) {
        while(a) {

            a = a->next;
        }
        b = b->next;
    }
}

// prototypes
int expr();
int term();
int factor();
void error( char * );
int get_token();
void match( int );


// global communication variables
int current_token;
int current_attribute;

/* bounded memory calculator */
int main()
{
    int value;

    current_token = get_token();
    while ( current_token != EOS ) {
        value = expr();
        fprintf( stderr, "\nValue = %d\n", value );
    }
}

// handles addition
int expr()
{
    int value = term();
    while (1) {
        if ( current_token == '+' ) {
            match( '+' );
            value += term();
        }
        else break;
    }
    return value;
}

// handles multiplication
int term()
{
    int value = factor();
    while (1) {
        if ( current_token == '*' ) {
            match( '*' );
            value *= factor();
        }
        else break;
    }
    return value;
}

// handles brackets and numbers
int factor()
{
    int value;

    if ( current_token == '(' ) {
        match( '(' );
        value = expr();
        match( ')' );
        return value;
    }
    else if ( current_token == NUM ) {
        value = current_attribute;
        match( NUM );
        return value;
    }
    else error( "Unexpected token in factor()" );
}

/* match expected token */
void match( int expected_token )
{
    if ( current_token == expected_token ) {
        current_token = get_token();
    }
    else {
        error("Unexpected token in match" );
    }
}


/* get next token */
int get_token()
{
    int c;		// current character from the stream
    int value;	// value of a number

    while (1) {
        switch ( c = getchar() ) {
            case '+': case '*': case '(': case ')':
                return c;	// return operators and brackets as is
            case ' ': case '\t':
                continue;	// ignore spaces and tabs
            default:
                if ( isdigit(c) ) {
                    value = c - '0';
                    while ( isdigit( c = getchar() )) {
                        value = value * 10 + (c - '0');
                    }
                    ungetc( c, stdin );
                    current_attribute = value;
                    return NUM;
                }
                else if ( c == '\n' ) {
                    return EOS;
                }
                else {
                    fprintf( stderr, "{%c}", c );
                    error( "Unknown token" );
                }
        }
    }
}

// error reporting function
void error( char *message )
{
    fprintf( stderr, "Error: %s\n", message );
    exit(1);
}