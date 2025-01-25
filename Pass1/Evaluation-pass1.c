#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LABEL_LEN 20
#define MAX_LINES 100
#define MAX_LINE_LEN 50

// Structure for symbol table entry
typedef struct {
    char label[MAX_LABEL_LEN];
    int address;
} Symbol;

// Symbol table
Symbol symtab[MAX_LINES];
int symtabSize = 0;

// Function to add symbol to symbol table
void addSymbol(char *label, int address) {
    strcpy(symtab[symtabSize].label, label);
    symtab[symtabSize].address = address;
    symtabSize++;
}

// Function to find symbol in symbol table
int findSymbol(char *label) {
    for (int i = 0; i < symtabSize; i++) {
        if (strcmp(symtab[i].label, label) == 0) {
            return symtab[i].address;
        }
    }
    return -1;
}

// Function to parse line and return label, opcode, operand
void parseLine(char *line, char *label, char *opcode, char *operand) {
    sscanf(line, "%s %s %s", label, opcode, operand);
}

// Main function to process input
int main() {
    char line[MAX_LINE_LEN];
    char label[MAX_LABEL_LEN], opcode[MAX_LABEL_LEN], operand[MAX_LABEL_LEN];
    int locationCounter = 0x1000;  // Starting address

    // File pointers for input and intermediate files
    FILE *inputFile = fopen("input.txt", "r");  // Open input file
    if (!inputFile) {
        printf("Error: Could not open input file\n");
        return 1;
    }

    FILE *intermediateFile = fopen("intermediate.txt", "w");  // Open intermediate file for writing
    if (!intermediateFile) {
        printf("Error: Could not create intermediate file\n");
        fclose(inputFile);
        return 1;
    }

    // First pass to build symbol table
    while (fgets(line, sizeof(line), inputFile) != NULL) {
        // Initialize fields for each line
        label[0] = opcode[0] = operand[0] = '\0';

        // Parse line into label, opcode, operand
        parseLine(line, label, opcode, operand);

        // If line has a label, add it to symbol table
        if (label[0] != '-' && label[0] != '\0') {
            addSymbol(label, locationCounter);
        }

        // Write to intermediate file
        fprintf(intermediateFile, "%04X\t%s\t%s\t%s\n", locationCounter, label, opcode, operand);

        // Update location counter based on opcode
        if (strcmp(opcode, "RESW") == 0) {
            locationCounter += 3 * atoi(operand);  // RESW reserves 3 bytes per word
        } else if (strcmp(opcode, "RESB") == 0) {
            locationCounter += atoi(operand);      // RESB reserves bytes directly
        } else if (opcode[0] == '+') {
            locationCounter += 4;                  // Format 4 instruction
        }  else if(strcmp(opcode, "WORD") == 0) {
            locationCounter += 3;
        } else if (strcmp(opcode, "BYTE") == 0) {
            ++locationCounter;
        }
          else {
            locationCounter += 3;                  // Format 3 instruction
        }
        //

    }

    // Close input and intermediate files
    fclose(inputFile);
    fclose(intermediateFile);

    // Print symbol table
    printf("\nSymbol Table:\n");
    printf("Label\tAddress\n");
    for (int i = 0; i < symtabSize; i++) {
        printf("%s\t%04X\n", symtab[i].label, symtab[i].address);
    }

    printf("\nIntermediate File generated as intermediate.txt\n");
    return 0;
}
