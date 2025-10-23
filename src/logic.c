#include <stdio.h>
#include "logic.h"

void update_gate(struct Gate *gate) {
    // Read inputs
    SignalState in_a = (gate->input1 != NULL) ? gate->input1->state : LOW;
    SignalState in_b = (gate->input2 != NULL) ? gate->input2->state : LOW;

    SignalState result = LOW;

    switch (gate->type) {
        case CONSTANT_LOW:
            // Constant LOW gate: always outputs LOW
            result = LOW;
            break;

        case CONSTANT_HIGH:
            // Constant HIGH gate: always outputs HIGH
            result = HIGH;
            break;
        
        case AND:
            // AND Logic (A * B): Output is HIGH if and only if BOTH inputs are HIGH.
            result = (in_a == HIGH && in_b == HIGH) ? HIGH : LOW;
            break;

        case OR:
            // OR Logic (A + B): Output is HIGH if EITHER input is HIGH.
            result = (in_a == HIGH || in_b == HIGH) ? HIGH : LOW;
            break;

        case INVERT:
            // NOT Logic (~A): Output is the inverse of the single input (in_a).
            // Note: This gate type ignores in_b.
            result = (in_a == HIGH) ? LOW : HIGH;
            break;

        case NAND:
            // NAND Logic (~(A * B)): Output is LOW only if BOTH inputs are HIGH.
            // This is the logical inverse of the AND gate.
            result = !((in_a == HIGH && in_b == HIGH)) ? HIGH : LOW;
            break;

        case NOR:
            // NOR Logic (~(A + B)): Output is HIGH only if BOTH inputs are LOW.
            // This is the logical inverse of the OR gate.
            result = !((in_a == HIGH || in_b == HIGH)) ? HIGH : LOW;
            break;

        case XOR:
            // XOR Logic (A ⊕ B): Output is HIGH if inputs are DIFFERENT (odd parity).
            result = (in_a != in_b) ? HIGH : LOW;
            break;

        case XNOR:
            // XNOR Logic (~(A ⊕ B)): Output is HIGH if inputs are THE SAME (even parity).
            // This is the logical inverse of the XOR gate.
            result = (in_a == in_b) ? HIGH : LOW;
            break;

        default:
            // Default case acts as a buffer (passes input A through) if the type is unknown.
            result = in_a;
            break;
    }

    if (gate->output != NULL) {
        gate->output->state = result;
    }
}

void print_status(const char *wire_name, struct Wire *wire) {
    if (wire != NULL) {
        printf("%s is %s\n", wire_name, (wire->state == HIGH) ? "HIGH (1)" : "LOW (0)");
    } else {
        printf("%s is uninitialized.\n", wire_name);
    }
}
