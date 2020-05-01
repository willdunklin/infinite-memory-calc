# Infinite Memory Calculator

Calculator that processes unbounded numbers.

## Documentation 

### Operations

This calculator supports two operations:
- Addition (+)
- Multiplication (*)

Additionally precedence can be forced with parentheses.

### Precedence

The order of operations is consistent with normal standards. 

Operators from highest to lowest precedence:
- Parentheses
- Multiplication
- Addition

### Limitations

Numbers are visually represented in multiples of four digits.

So: 
```
4 = 0004
```
And 
```
123456 = 00123456
```

Other than that visual quirk there are few functional limitations to this calculator.

- The calculator only supports natural numbers.
- The calculator only supports two operations.

### Advantages

This calculator is incredibly robust with the features it does have.

It handles unknown characters gracefully and explicitly.

It is also very fast. Multiplying two 1000 digit numbers takes less than two seconds on Clarkson's Polaris under peak usage times.
