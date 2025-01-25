#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX 20

struct estab {
    char csname[10];
    char extsym[10];
    int address;
    int length;
} es[MAX];

// Function to check for duplicate control section name
int is_duplicate_csname(const char *name, int count) {
    for (int i = 0; i < count; i++) {
        if (strcmp(es[i].csname, name) == 0 && strcmp(es[i].csname, "") != 0) {
            return 1; // Duplicate found
        }
    }
    return 0; // No duplicate
}

// Function to check for duplicate external symbol name
int is_duplicate_extsym(const char *symbol, int count) {
    for (int i = 0; i < count; i++) {
        if (strcmp(es[i].extsym, symbol) == 0 && strcmp(es[i].extsym, "") != 0) {
            return 1; // Duplicate found
        }
    }
    return 0; // No duplicate
}

int main() {
    char input[10], name[10], symbol[10], objcode[10];
    int count = 0, progaddr, csaddr, add, len, start_addr, size;
    FILE *fp1, *fp2, *fp3;

    // Open files
    fp1 = fopen("input.dat", "r");
    if (fp1 == NULL) {
        printf("Error: Unable to open input file.\n");
        return 1;
    }
    fp2 = fopen("estab.dat", "w");
    if (fp2 == NULL) {
        printf("Error: Unable to open output file.\n");
        fclose(fp1);
        return 1;
    }
    fp3 = fopen("objectcode.dat", "w");
    if (fp3 == NULL) {
        printf("Error: Unable to open object code file.\n");
        fclose(fp1);
        fclose(fp2);
        return 1;
    }

    printf("Enter the location where the program has to be loaded: ");
    if (scanf("%x", &progaddr) != 1) { // Input in hexadecimal
        printf("Error: Invalid program address.\n");
        fclose(fp1);
        fclose(fp2);
        fclose(fp3);
        return 1;
    }

    csaddr = progaddr;
    fprintf(fp2, "CS_NAME\tEXT_SYM_NAME\tADDRESS\tLENGTH\n");
    fprintf(fp2, "--------------------------------------\n");
    fprintf(fp3, "Addr\tObject_Code\n");

    // Print header to the terminal
    printf("CS_NAME\tEXT_SYM_NAME\tADDRESS\tLENGTH\n");
    printf("--------------------------------------\n");

    // Process input file
    while (fscanf(fp1, "%s", input) != EOF) {
        if (strcmp(input, "H") == 0) { // Header record
            fscanf(fp1, "%s %x %x", name, &add, &len); // Read in hexadecimal

            if (is_duplicate_csname(name, count)) {
                printf("Error: Duplicate control section name '%s'.\n", name);
                exit(0); // Skip duplicate
            }

            strcpy(es[count].csname, name);
            strcpy(es[count].extsym, "");
            es[count].address = csaddr;
            es[count].length = len;

            // Print to terminal
            printf("%s\t%s\t\t%X\t%X\n", es[count].csname, es[count].extsym, es[count].address, es[count].length);
            // Write to file
            fprintf(fp2, "%s\t%s\t\t%X\t%X\n", es[count].csname, es[count].extsym, es[count].address, es[count].length);

            count++;
        } else if (strcmp(input, "D") == 0) { // Define record
            while (fscanf(fp1, "%s", symbol) == 1 && strcmp(symbol, "R") != 0) {
                fscanf(fp1, "%x", &add); // Read in hexadecimal

                if (is_duplicate_extsym(symbol, count)) {
                    printf("Error: Duplicate external symbol name '%s'.\n", symbol);
                    exit(0); // Skip duplicate
                }

                strcpy(es[count].csname, "");
                strcpy(es[count].extsym, symbol);
                es[count].address = add + csaddr;
                es[count].length = 0;

                // Print to terminal
                printf("%s\t%s\t\t%X\t%X\n", es[count].csname, es[count].extsym, es[count].address, es[count].length);

                // Write to file
                fprintf(fp2, "%s\t%s\t\t%X\t%X\n", es[count].csname, es[count].extsym, es[count].address, es[count].length);
                count++;
            }
        } else if (strcmp(input, "T") == 0) { // Text record
            fscanf(fp1, "%x %x", &start_addr, &size); // Read start address and size
            start_addr += csaddr; // Adjust start address

            for (int i = 0; i < size; i++) {
                fscanf(fp1, "%2s", objcode); // Read each byte of object code
                fprintf(fp3, "%04X\t%s\n", start_addr + i, objcode); // Write to object code file
            }
        } else if (strcmp(input, "E") == 0) { // End record
            csaddr += len; // Update CSADDR for the next control section
        }
    }

    // Print footer to terminal
    printf("--------------------------------------\n");

    // Write footer to file
    fprintf(fp2, "--------------------------------------\n");

    fclose(fp1);
    fclose(fp2);
    fclose(fp3);
    printf("Program executed successfully.\n");
    return 0;
}
