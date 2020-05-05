#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

// Fork of bounded memory calculator by Christino Tamon: https://github.com/ctamon/finite-calculator

// constants
#define EOS	257
#define NUM	258

// Block of 4 digits
// Size 4 was chosen because:
//     9999*9999 < 2^32
//     99999*99999 > 2^32
struct block {
    struct block* next;
    int val;
    int exp;
};

struct block* create_block(int val, int exp);

// Linked list of blocks
struct list {
    struct list* next;
    struct block* data;
};

struct list* append(struct list* prev, struct block* block);

void carry(struct block** a, int value, int exp);
struct block* add(struct block** a, struct block* b);
struct block* mult(struct block* a, struct block* b);

char* block_str(struct block* a);
int p10(int e);
char *string_reverse(char *str);


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

FILE *ifp, *ofp;
int done;

/* unbounded memory calculator */
int main(int argc, char* argv[])
{
    ifp = fopen(argv[1], "r");
    ofp = fopen(argv[2], "w");
    struct block* value;

    done = 0;

    while(!done) {
        current_token = get_token();
        while (current_token != EOS) {
            value = expr();
            fprintf(ofp, "%s\n", block_str(value));
        }
    }

    fclose(ifp);
    fclose(ofp);
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
    return NULL;
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
        switch ( c = getc(ifp) ) {
            case '+': case '*': case '(': case ')':
                return c;	// return operators and brackets as is
            case ' ': case '\t':
                continue;	// ignore spaces and tabs
            default:
                if ( isdigit(c) ) {
                    ungetc(c, ifp);
                    while ( isdigit( c = getc(ifp) )) {
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
                                    next->next = create_block(value, ((length - i - 1) / 4));
                                    next = next->next;
                                    value = 0;
                                }
                                value += buffer[i] * p10(place);
                                length++;
                            }
                            next->next = create_block(value, ((length - i - 1) / 4));
                            next = next->next;
                            value = 0;
                        }
                        value += buffer[i] * p10(place);
                    }

                    ungetc( c, ifp );
                    current_attribute = head->next;
                    return NUM;
                }
                else if (c == -1 ) {
                    done = 1;
                    return EOS;
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

// Allocates a new block of digits
struct block* create_block(int val, int exp) {
    struct block* new = (struct block*)malloc(sizeof(struct block));
    new->next = NULL;
    new->val = val;
    new->exp = exp;
    return new;
}

// Append list item to prev node
struct list* append(struct list* prev, struct block* block) {
    struct list* new = (struct list*)malloc(sizeof(struct list));
    new->next = NULL;
    new->data = block;
    prev->next = new;
    return new;
}

// Carry the value to block a
// Effectively rounds blocks that are too large
void carry(struct block** a, int value, int exp) {
    if(*a == NULL) {
        *a = create_block(value, exp);
        return;
    }
    value += (*a)->val;
    (*a)->val = value;
    // Recursive carry
    if(value >= 10000) {
        value /= 10000;
        (*a)->val %= 10000;
        carry(&((*a)->next), value, (*a)->exp+1);
    }
}

// Add block b to a
struct block* add(struct block** a, struct block* b) {
    // If both are null then the sum is null
    if(*a == NULL && b == NULL)
        return NULL;

    // if(a == 0 && b != 0): a + b = b
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
    // If b was null then return a anyway
    return *a;
}

// Multiply blocks a and b
struct block* mult(struct block* a, struct block* b) {
    struct list *head = (struct list*)malloc(sizeof(struct list)), *next = head;

    // Keep track of the members of a for each block in b
    struct block* a_start = a;
    // Loop through the blocks in b
    while(b) {
        a = a_start;
        // Loop throught the blocks in a
        while(a) {
            // The exp of the resulting block will be the sum of the two blocks being multiplied
            int exp = a->exp + b->exp;
            // Multiply the blocks
            struct block* product = create_block(a->val * b->val, exp);
            // Expand upward with carrying
            carry(&product, 0, exp);

            // Fill in the zeros below the product
            struct block *prev = product, *current;
            for(; exp > 0; --exp) {
                // Create a new block of zeros
                current = create_block(0, exp - 1);
                // Append the previous block chain to the zero block
                current->next = prev;
                prev = current;
            }

            // Next is the list of all products to be summed
            next = append(next, prev);

            a = a->next;
        }
        b = b->next;
    }

    // Sum the intermediate products
    head = head->next;
    while(head) {
        b = add(&(head->data), b);
        head = head->next;
    }
    a = b;
    return a;
}

// Converts a block into a c string
char* block_str(struct block* a) {
    if(a == NULL)
        return '\0';

    int size = 8, i = 0;
    char* buffer = (char*)malloc(size);
    while(a) {
        if(i >= size - 5) {
            buffer = (char*)realloc(buffer, size = size * 2);
        }
        int j;
        for(j = 0; j < 4; ++j) {
            buffer[i++] = (a->val % 10) + '0';
            a->val /= 10;
        }
        a = a->next;
    }

    buffer[i] = '\0';
    //strrev(buffer); Does not work on linux
    string_reverse(buffer);

    return buffer;
}

// Power of 10
int p10(int e) {
    int result = 1;
    for(; e > 0; --e)
        result *= 10;
    return result;
}

// string.h has a strrev() but it does not work on linux
// This implementation is based on https://stackoverflow.com/questions/8534274/is-the-strrev-function-not-available-in-linux
char* string_reverse(char* str) {
    char *p1, *p2;

    if (!str || !*str)
        return str;

    // Point p1 to the start and p2 to the end increment/decrement until the cross
    for(p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2) {
        // Swap the chars
        *p1 ^= *p2;
        *p2 ^= *p1;
        *p1 ^= *p2;
    }

    return str;
}
