#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "logic.h"

int main() {
    // Allocate Memory for Components (2 Gates, 4 Wires)
    
    // Wires: Two Inputs (A, B), Two Outputs (Sum, Carry)
    struct Wire *w_a = (struct Wire *)malloc(sizeof(struct Wire));
    struct Wire *w_b = (struct Wire *)malloc(sizeof(struct Wire));
    struct Wire *w_sum = (struct Wire *)malloc(sizeof(struct Wire));
    struct Wire *w_carry = (struct Wire *)malloc(sizeof(struct Wire));

    // Gates: One XOR for Sum, One AND for Carry
    struct Gate *g_xor = (struct Gate *)malloc(sizeof(struct Gate));
    struct Gate *g_and = (struct Gate *)malloc(sizeof(struct Gate));

    // Safety check for all allocations
    if (!w_a || !w_b || !w_sum || !w_carry || !g_xor || !g_and) {
        fprintf(stderr, "Memory allocation failed! Cannot run Half-Adder test.\n");
        // Minimal cleanup before exit
        free(w_a); free(w_b); free(w_sum); free(w_carry); free(g_xor); free(g_and);
        return 1;
    }

    // Initialize Gates
    g_xor->type = XOR;
    g_and->type = AND;

    // Connect the Half-Adder Circuit (Pointer Assignment/Wiring)
    
    // XOR Gate (Calculates SUM = A XOR B)
    g_xor->input1 = w_a;    
    g_xor->input2 = w_b;    
    g_xor->output = w_sum;  
    
    // AND Gate (Calculates CARRY = A AND B)
    g_and->input1 = w_a;    
    g_and->input2 = w_b;    
    g_and->output = w_carry; 

    printf("--- Half-Adder Simulation Results\n");
    
    // Run All Test Cases (Full Truth Table)
    
    // Test Case 1: 0 + 0 = Sum 0, Carry 0
    w_a->state = LOW;
    w_b->state = LOW;
    update_gate(g_xor); // Update XOR (Sum)
    update_gate(g_and); // Update AND (Carry)
    printf("\nTest 1 (0 + 0):\n");
    printf("  A: %d, B: %d -> Sum: %d, Carry: %d\n", 
           w_a->state, w_b->state, w_sum->state, w_carry->state);
    
    // Test Case 2: 0 + 1 = Sum 1, Carry 0
    w_a->state = LOW;
    w_b->state = HIGH;
    update_gate(g_xor); 
    update_gate(g_and); 
    printf("\nTest 2 (0 + 1):\n");
    printf("  A: %d, B: %d -> Sum: %d, Carry: %d\n", 
           w_a->state, w_b->state, w_sum->state, w_carry->state);
    
    // Test Case 3: 1 + 0 = Sum 1, Carry 0
    w_a->state = HIGH;
    w_b->state = LOW;
    update_gate(g_xor); 
    update_gate(g_and); 
    printf("\nTest 3 (1 + 0):\n");
    printf("  A: %d, B: %d -> Sum: %d, Carry: %d\n", 
           w_a->state, w_b->state, w_sum->state, w_carry->state);

    // Test Case 4: 1 + 1 = Sum 0, Carry 1
    w_a->state = HIGH;
    w_b->state = HIGH;
    update_gate(g_xor); 
    update_gate(g_and); 
    printf("\nTest 4 (1 + 1):\n");
    printf("  A: %d, B: %d -> Sum: %d, Carry: %d\n", 
           w_a->state, w_b->state, w_sum->state, w_carry->state);
    
    // Clean Up Memory (CRITICAL)
    free(w_a);
    free(w_b);
    free(w_sum);
    free(w_carry);
    free(g_xor);
    free(g_and);

    return 0;
}
