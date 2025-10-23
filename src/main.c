#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "logic.h"

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
