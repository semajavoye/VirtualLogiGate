#ifndef LOGIC_H
#define LOGIC_H

// Typedefs for defining some states
typedef enum { 
    CONSTANT_LOW = 0, CONSTANT_HIGH = 1,
    AND, OR, INVERT, NAND, NOR, XOR, XNOR} GateType;
typedef enum { 
    LOW = 0, 
    HIGH = 1,
    UNKNOWN = 2 // Just for Debugging or uninitialized wires
} SignalState;

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

struct Component 
{
    char *name; // Name of the component

    // internal structure
    struct Gate *gates;
    int num_gates;

    struct Component **subcomponents;
    int num_subcomponents;

    struct Wire *wires;
    int num_wires;

    // external connections
    struct Wire **inputs;
    int num_inputs;

    struct Wire **outputs;
    int num_outputs;
};

void update_gate(struct Gate *gate);
void print_status(const char *wire_name, struct Wire *wire);

#endif // LOGIC_H
