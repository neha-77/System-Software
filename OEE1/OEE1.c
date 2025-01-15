#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 100

// Function to add a symbol to the symbol table
void addSymbol(const char *label, int value) {
    FILE *symtab = fopen("symtab.txt", "a");
    if (!symtab) {
        printf("Error opening symtab.txt.\n");
        exit(1);
    }
    fprintf(symtab, "%s\t%04X\n", label, value);
    fclose(symtab);
}

// Function to find a symbol's value in the symbol table
int findSymbol(const char *label) {
    FILE *symtab = fopen("symtab.txt", "r");
    if (!symtab) {
        printf("Error opening symtab.txt.\n");
        exit(1);
    }
    char symLabel[MAX];
    int value;
    while (fscanf(symtab, "%s %X", symLabel, &value) != EOF) {
        if (strcmp(symLabel, label) == 0) {
            fclose(symtab);
            return value;
        }
    }
    fclose(symtab);
    return -1; // Symbol not found
}

int main() {
    FILE *input, *optab, *intermediate, *lengthFile;
    char label[MAX], opcode[MAX], operand[MAX];
    int locctr = 0, startAddr = 0, programLength;

    // File Handling
    input = fopen("input.txt", "r");
    optab = fopen("optab.txt", "r");
    intermediate = fopen("intermediate.txt", "w");
    lengthFile = fopen("length.txt", "w");

    if (!input || !optab || !intermediate || !lengthFile) {
        printf("Error opening files.\n");
        exit(1);
    }

    // Read the first line of the input
    fscanf(input, "%s %s %s", label, opcode, operand);

    // Handle START directive
    if (strcmp(opcode, "START") == 0) {
        startAddr = (int)strtol(operand, NULL, 16);
        locctr = startAddr;
        fprintf(intermediate, "        %s\t%s\t%s\n", label, opcode, operand);
    } else {
        locctr = 0;
        rewind(input); // Re-process the first line later
    }

    // Process the input file line by line
    while (fscanf(input, "%s %s %s", label, opcode, operand) != EOF) {
        if (strcmp(opcode, "END") == 0) {
            fprintf(intermediate, "        %s\t%s\t%s\n", label, opcode, operand);
            break;
        }

        int format4 = 0;

        // Check for Format 4 instruction
        if (opcode[0] == '+') {
            format4 = 1;
            memmove(opcode, opcode + 1, strlen(opcode)); // Remove '+' for OPTAB lookup
        }

        // Process ORG directive
        if (strcmp(opcode, "ORG") == 0) {
            int newLoc;
            if (strcmp(operand, "*") == 0) {
                newLoc = locctr; // Current location counter
            } else {
                newLoc = findSymbol(operand); // Find value of referenced label
                if (newLoc == -1) { // Operand is not a label; interpret as a number
                    newLoc = (int)strtol(operand, NULL, 16);
                }
            }
            fprintf(intermediate, "        %s\t%s\t%s\n", label, opcode, operand);
            locctr = newLoc; // Update location counter
            continue;
        }

        // Process EQU directive
        if (strcmp(opcode, "EQU") == 0) {
            int value;
            if (strcmp(operand, "*") == 0) {
                value = locctr; // Current location counter
            } else {
                value = findSymbol(operand); // Find value of referenced label
                if (value == -1) { // Operand is not a label; interpret as a number
                    value = (int)strtol(operand, NULL, 16);
                }
            }
            addSymbol(label, value);
            fprintf(intermediate, "        %s\t%s\t%s\n", label, opcode, operand);
            continue;
        }

        // Write label to symbol table
        if (strcmp(label, "**") != 0) {
            addSymbol(label, locctr);
        }

        // Search in OPTAB
        char optabMnemonic[MAX], optabValue[MAX];
        rewind(optab);
        int found = 0;
        while (fscanf(optab, "%s %s", optabMnemonic, optabValue) != EOF) {
            if (strcmp(opcode, optabMnemonic) == 0) {
                if (format4) {
                    fprintf(intermediate, "%04X\t%s\t+%s\t%s\n", locctr, label, opcode, operand);
                    locctr += 4; // Format 4 instruction
                } else {
                    fprintf(intermediate, "%04X\t%s\t%s\t%s\n", locctr, label, opcode, operand);
                    locctr += 3; // Format 3 instruction
                }
                found = 1;
                break;
            }
        }

        if (!found) {
            // Handle assembler directives
            if (strcmp(opcode, "WORD") == 0) {
                fprintf(intermediate, "%04X\t%s\t%s\t%s\n", locctr, label, opcode, operand);
                locctr += 3;
            } else if (strcmp(opcode, "RESW") == 0) {
                fprintf(intermediate, "%04X\t%s\t%s\t%s\n", locctr, label, opcode, operand);
                locctr += 3 * atoi(operand);
            } else if (strcmp(opcode, "RESB") == 0) {
                fprintf(intermediate, "%04X\t%s\t%s\t%s\n", locctr, label, opcode, operand);
                locctr += atoi(operand);
            } else if (strcmp(opcode, "BYTE") == 0) {
                int len = (operand[0] == 'C') ? strlen(operand) - 3 : 1;
                fprintf(intermediate, "%04X\t%s\t%s\t%s\n", locctr, label, opcode, operand);
                locctr += len;
            } else {
                printf("Error: Invalid opcode %s\n", opcode);
                exit(1);
            }
        }
    }

    // Calculate program length
    programLength = locctr - startAddr;
    fprintf(lengthFile, "%X\n", programLength);

    // Close files
    fclose(input);
    fclose(optab);
    fclose(intermediate);
    fclose(lengthFile);

    printf("Pass One completed successfully.\n");
    return 0;
}
