#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Typedefs for defining some states
typedef enum { AND, OR, INVERT, NAND, NOR, XOR, XNOR} GateType;
typedef enum { LOW = 0, HIGH = 1 } SignalState;

struct Gate
{
    GateType type;

    // Inputs
    // "Store the adress where the wire lives in memory"
    struct Wire *input1;
    struct Wire *input2;

    // Outputs
    // "Store the adress where the wire lives in memory"
    struct Wire *output;
};

struct Wire 
{
    SignalState state;
};

void update_gate(struct Gate *gate);
void print_status(const char *wire_name, struct Wire *wire);


int main() {
    // Allocating memory for our components 

    // Wires:
    // *w_a is a pointer. malloc finds a block of memory big enough for one Wire struct and returns the adress of that block. w_a stores this addres (*)
    struct Wire *w_a = (struct Wire *)malloc(sizeof(struct Wire));
    struct Wire *w_b = (struct Wire *)malloc(sizeof(struct Wire));
    struct Wire *w_out = (struct Wire *)malloc(sizeof(struct Wire));

    struct Gate *g_nand = (struct Gate *)malloc(sizeof(struct Gate));

    // Check if memory allocation was succesfull
    if (!w_a || !w_b || !w_out || !g_nand) {
        fprintf(stderr, "Memory allocation failed!\n");
        return 1;
    }

    // Initialize our Components

    // Set our gate type
    g_nand->type = INVERT;

    // "Connect" the wires (by copying the memory adresses)
    // input1 now holds the adress of w_a wich is the wire struct.
    // gate has no need to search the Wire because it now knows the Wire's adress (stored in w_a)
    g_nand->input1 = w_a;
    g_nand->input2 = w_b;
    g_nand->output = w_out;

    // Dereference to the Wire struct and set its state to LOW
    w_a->state = LOW;

    print_status("Input A", w_a);

    update_gate(g_nand);
    print_status("Output (Input A)", w_out);

    // Add free() function for releasing the dynamically allocated memory of the wires and gates
    free(w_a);
    free(w_b);
    free(w_out);
    free(g_nand);

    return 0;
}

void update_gate(struct Gate *gate) {
    // Read inputs
    SignalState in_a = (gate->input1 != NULL) ? gate->input1->state : LOW;
    SignalState in_b = (gate->input2 != NULL) ? gate->input2->state : LOW;

    SignalState result = LOW;

    switch (gate->type) {
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
