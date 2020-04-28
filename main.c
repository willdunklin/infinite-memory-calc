#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>

// constants
#define EOS	257
#define NUM	258

// Block of 4 digits
struct block {
    struct block* next;
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
struct block* add(struct block** a, struct block* b) {
    if(*a == NULL && b == NULL)
        return NULL;

    if(*a == NULL)
        *a = b;

    else if(b != NULL) {
        // If both have a block
        (*a)->val += b->val;
        // Carry nothing extra (handles if there's overflow)
        carry(a, 0, (*a)->exp);
        // Add the next blocks
        add(&((*a)->next), b->next);
    }
    return *a;
}

struct block* mult(struct block* a, struct block* b) {
    struct list *head = (struct list*)malloc(sizeof(struct list)), *next = head;

    // TODO: Works for one block but not for multiple
    while(b) {
        while(a) {
            int exp = a->exp + b->exp;
            // Multiply the blocks
            struct block* product = create_block(a->val * b->val, exp);
            // Expand upward with carrying
            carry(&product, 0, exp);

            // Fill in zeros
            struct block *prev = product, *current;
            for(; exp > 0; --exp) {
                current = zero(exp);
                prev->next = current;
                prev = current;
            }

            append(next, product);

            a = a->next;
        }
        b = b->next;
    }

    b = NULL;
    // Sum the intermediate products
    while(head = head->next) {
        b = add(&(head->data), b);
    }
    a = b;
    return a;
}

char* block_str(struct block* a) {
    if(a == NULL)
        return '\0';

    int size = 8, i = 0;
    char* buffer = (char*)malloc(size);
    while(a) {
        if(i >= size - 5) {
            buffer = (char*)realloc(buffer, size = size * 2);
        }
        for(int j = 0; j < 4; ++j) {
            buffer[i++] = (a->val % 10) + '0';
            a->val /= 10;
        }
        a = a->next;
    }

    buffer[i] = '\0';
    strrev(buffer);

    return buffer;
}

int p10(int e) {
    int result = 1;
    for(; e > 0; --e)
        result *= 10;
    return result;
}

// prototypes
struct block* expr();
struct block* term();
struct block* factor();
void error( char * );
int get_token();
void match( int );


// global communication variables
int current_token;
struct block* current_attribute;

/* unbounded memory calculator */
int main()
{
    struct block* value;

    current_token = get_token();
    while ( current_token != EOS ) {
        value = expr();
        fprintf( stderr, "\nValue = %s\n", block_str(value) );
    }
}

// handles addition
struct block* expr()
{
    struct block* value = term();
    while (1) {
        if ( current_token == '+' ) {
            match( '+' );

            value = add(&value, term());
        }
        else break;
    }
    return value;
}

// handles multiplication
struct block* term()
{
    struct block* value = factor();
    while (1) {
        if ( current_token == '*' ) {
            match( '*' );

            value = mult(value, factor());
        }
        else break;
    }
    return value;
}

// handles brackets and numbers
struct block* factor()
{
    struct block* value;

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
    int value = 0;	// value of a number
    int i = 0, length, place;
    struct block *head = create_block(0, 0), *next = head;
    int size = 8;
    char* buffer = (char*)malloc(size);

    while (1) {
        switch ( c = getchar() ) {
            case '+': case '*': case '(': case ')':
                return c;	// return operators and brackets as is
            case ' ': case '\t':
                continue;	// ignore spaces and tabs
            default:
                if ( isdigit(c) ) {
                    ungetc(c, stdin);
                    while ( isdigit( c = getchar() )) {
                        if(i >= size - 1)
                            buffer = (char*)realloc(buffer, size = size * 2);
                        buffer[i++] = c - '0';
                    }

                    --i;
                    length = i;

                    for(; i >= 0; --i) {
                        place = (length - i) % 4;
                        // Flush the completed block
                        if ((place == 0 && length != i) || i == 0) {
                            // The last digit in the number
                            if(i == 0) {
                                // If the old block hasn't been flushed yet
                                if(place == 0 && length != 0) {
//                                    printf("%d\n", value);
                                    next->next = create_block(value, ((length - i) / 4));
                                    next = next->next;
                                    value = 0;
                                }
                                value += buffer[i] * p10(place);
                            }
//                            printf("%d\n", value);
                            next->next = create_block(value, ((length - i) / 4));
                            next = next->next;
                            value = 0;
                        }
                        value += buffer[i] * p10(place);
                    }

                    ungetc( c, stdin );
                    current_attribute = head->next;
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