# Example Test Programs

This directory contains example seed programs for testing compiler optimization inconsistencies.

## Purpose

These programs demonstrate various scenarios where additional optimization information (dead code hints, value ranges) should help compilers optimize better, but might paradoxically result in worse optimization.

## Example Programs

### example1_simple.c

**Purpose**: Basic value range propagation

**Key Features**:
- Simple conditional with known value ranges
- Potential dead code in else branch
- Straightforward optimization opportunity

**Expected Behavior**:
When the compiler knows `value` is in range [0, 100], the else branch in `compute_value()` should be eliminated.

**Refined Version Would Add**:
```c
// After: int value = atoi(argv[1]);
if (!(value >= 0 && value <= 100)) __builtin_unreachable();
```

### example2_loops.c

**Purpose**: Loop bound propagation

**Key Features**:
- Loop with statically known bounds
- Unreachable error condition
- Array access optimization

**Expected Behavior**:
The compiler should recognize that `i >= 1000` is never true and eliminate that check.

**Refined Version Would Add**:
```c
// In sum_array(), after the for loop declaration:
if (!(size >= 1 && size <= 1000)) __builtin_unreachable();
```

### example3_branches.c

**Purpose**: Complex control flow with dead code

**Key Features**:
- Multiple nested conditionals
- Dead code in negative value branch
- Unreachable switch case

**Expected Behavior**:
With information that `value > 0` and `flag == true`, the compiler should:
1. Eliminate the `else` branch handling `value <= 0`
2. Potentially eliminate the `STATUS_INVALID` case

**Refined Version Would Add**:
```c
// At start of process_data():
if (!(value > 0)) __builtin_unreachable();
if (!flag) __builtin_unreachable();
```

## Testing These Examples

### Using the C++ Framework

```bash
cd ../..
./build/compiler_tester --generate 0  # Don't generate, use existing
./scripts/test_optimization.sh examples/seed_programs/
```

### Manual Testing

```bash
# Compile original
gcc -O3 example1_simple.c -o example1_original
ls -l example1_original

# Create refined version (manually add hints)
# ... edit file to add __builtin_unreachable() ...

# Compile refined
gcc -O3 example1_simple_refined.c -o example1_refined
ls -l example1_refined

# Compare sizes
wc -c example1_original example1_refined
```

### Viewing Assembly

```bash
# Original assembly
gcc -O3 -S example1_simple.c -o example1_original.s

# Refined assembly
gcc -O3 -S example1_simple_refined.c -o example1_refined.s

# Compare
diff -u example1_original.s example1_refined.s
```

## Creating Your Own Examples

Good test programs for this framework should:

1. **Have Deterministic Behavior**: Avoid random numbers, undefined behavior
2. **Contain Potential Dead Code**: Branches that could be eliminated with additional information
3. **Use Value Ranges**: Variables with known or derivable bounds
4. **Be Compilable**: No syntax errors, standard C
5. **Be Small**: Easier to analyze and minimize

### Template

```c
#include <stdio.h>

// Function with optimization opportunities
int process(int x) {
    // Add value range assumption
    // Refined: if (!(x >= MIN && x <= MAX)) __builtin_unreachable();

    if (x > THRESHOLD) {
        return x * 2;
    } else {
        // Potentially dead code if x is always > THRESHOLD
        return -1;
    }
}

int main(void) {
    int value = 42;  // Known constant or bounded value
    int result = process(value);
    printf("Result: %d\n", result);
    return 0;
}
```

## Expected Inconsistencies

When refined versions produce **worse** optimizations, you might see:

1. **Larger Binary Size**: Additional code generated despite hints
2. **More Instructions**: Assembly shows redundant operations
3. **Worse Register Allocation**: Less efficient register usage
4. **Retained Dead Code**: Branches not eliminated despite unreachability hints

## Analysis Tools

### objdump

```bash
objdump -d example1_original > original.asm
objdump -d example1_refined > refined.asm
diff -u original.asm refined.asm
```

### size

```bash
size example1_original example1_refined
```

### nm

```bash
nm -S example1_original
nm -S example1_refined
```

## Contributing Examples

To add new examples:

1. Create a `.c` file following the naming pattern `exampleN_description.c`
2. Add comments explaining:
   - Purpose of the test
   - Expected compiler behavior
   - What hints would be injected
3. Ensure the program compiles with `-std=c11 -Wall -Wextra`
4. Test with both GCC and Clang at various optimization levels

## References

- [Paper: "Refined Input, Degraded Output"](https://doi.org/10.1145/3656404)
- [GCC Optimization Options](https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html)
- [Clang Optimization Flags](https://clang.llvm.org/docs/CommandGuide/clang.html#code-generation-options)
- [__builtin_unreachable Documentation](https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html)

---

**Note**: These are seed programs. The framework can also generate random programs using Csmith or YarpGen for more comprehensive testing.
