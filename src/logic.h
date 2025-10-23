#ifndef LOGIC_H
#define LOGIC_H

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

#endif // LOGIC_H