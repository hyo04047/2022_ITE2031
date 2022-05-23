#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXLINELENGTH 1000

int readAndParse(FILE *, char *, char *, char *, char *, char *);
int isNumber(char *);

char label_list[MAXLINELENGTH][7];

int dupcheck(char *label, int count);

int main(int argc, char *argv[])
{
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
        arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];

    if (argc != 3)
    {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n", argv[0]);
        exit(1);
    }

    inFileString = argv[1];
    outFileString = argv[2];

    inFilePtr = fopen(inFileString, "r");
    if (inFilePtr == NULL)
    {
        printf("error in opening %s\n", inFileString);
        exit(1);
    }

    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL)
    {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }

    int i = 0; /* count line */
     /* save label for jump */
    while(1)
    {
        if (!readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2))
        { /* reached end of file */
            break;
        }

        if (!strcmp(label, ""))
        {
            i++;
            continue;
        }

        if (strlen(label) > 6)
        {
            printf("error: too long label\n");
            exit(1);
        }

        if (dupcheck(label, i) != 0){
            printf("error: duplicate label\n");
            exit(1);
        }

        strcpy(label_list[i++], label);
    }

    /* this is how to rewind the file ptr so that you start reading from the
    beginning of the file */
    rewind(inFilePtr);

    /* after doing a readAndParse, you may want to do the following to test the
    opcode */
    int field0, field1, field2, opcode_, mccode;
    int pc = 0;
    while(1)
    {
        field0 = field1 = field2 = opcode_ = mccode = 0;
        if (!readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2))
        { /* reached end of file */
            break;
        }
        /* R-type Instruction */
        else if (!strcmp(opcode, "add") || !strcmp(opcode, "nor"))
        {
            if (!isNumber(arg0) || !isNumber(arg1) || !isNumber(arg2))
            {
                printf("error: non-numeric value for R-type\n");
                exit(1);
            }

            if (!strcmp(opcode, "add"))
                opcode_ = 0;
            else if (!strcmp(opcode, "nor"))
                opcode_ = 1;
            
            field0 = atoi(arg0);
            field1 = atoi(arg1);
            field2 = atoi(arg2);

            mccode = opcode_ << 22 | field0 << 19 | field1 << 16 | field2;
        }
        /* I-type Instruction */
        else if (!strcmp(opcode, "lw") || !strcmp(opcode, "sw") || !strcmp(opcode, "beq"))
        {
            if (!isNumber(arg0) || !isNumber(arg1))
            {
                printf("error: non-numeric value for I-type\n");
                exit(1);
            }

            if (!strcmp(opcode, "lw"))
                opcode_ = 2;
            else if (!strcmp(opcode, "sw"))
                opcode_ = 3;
            else if (!strcmp(opcode, "beq"))
                opcode_ = 4;
            
            field0 = atoi(arg0);
            field1 = atoi(arg1);

            if (isNumber(arg2))
                field2 = atoi(arg2);
            else 
            {
                field2 = dupcheck(arg2, i);
                if (field2 == 0)
                {
                    printf("error: invalid label for I-type\n");
                    exit(1);
                }
            }

            if (!strcmp(opcode, "beq") && !isNumber(arg2))
            {
                field2 = field2 - (pc + 1);
                if (field2 > 32767 || field2 < -32768)
                {
                    printf("error: invalid offset for I-type\n");
                    exit(1);
                }
            }
            /* chop off all but the lowest 16 bits for negative offset */
            if (opcode_ == 4)
                field2 &= 0x0000FFFF;

            mccode = opcode_ << 22 | field0 << 19 | field1 << 16 | field2;
        }
        /* J-type Instruction */
        else if (!strcmp(opcode, "jalr"))
        {
            if (!isNumber(arg0) || !isNumber(arg1))
            {
                printf("error: non-numeric value for J-type\n");
                exit(1);
            }

            opcode_ = 5;

            field0 = atoi(arg0);
            field1 = atoi(arg1);

            mccode = opcode_ << 22 | field0 << 19 | field1 << 16;
        }
        /* O-type Instruction */
        else if (!strcmp(opcode, "halt") || !strcmp(opcode, "noop"))
        {
            if (!strcmp(opcode, "halt"))
                opcode_ = 6;
            else
                opcode_ = 7;
            
            mccode = opcode_ << 22;
        }
        /* .fill */
        else if (!strcmp(opcode, ".fill"))
        {
            if (isNumber(arg0))
                mccode = atoi(arg0);
            else
            {
                mccode = dupcheck(arg0, i);
                if (mccode == 0)
                {
                    printf("error: no label for .fill\n");
                    exit(1);
                }
            }
        }

        else
        {
            printf("error: unrecognized opcode\n");
            printf("%s\n", opcode);
            exit(1);
        }
        
        pc++;
        fprintf(outFilePtr, "%d\n", mccode);
    }

    fclose(inFilePtr);
    fclose(outFilePtr);

    return (0);
} 
/* check duplicate label or fine label in list */
int dupcheck(char *label, int count)
{
    for(int i = 0; i < count; i++){
        if(!strcmp(label, label_list[i])){
            return i;
        }
    }
    return 0;
}

int readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0, char *arg1, char *arg2)
{
    char line[MAXLINELENGTH];
    char *ptr = line;
    /* delete prior values */
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0'; /* read the line from the assembly-language file */
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL)
    { /* reached end of file */
        return (0);
    }
    /* check for line too long (by looking for a \n) */ 
    if (strchr(line, '\n') == NULL)
    {
        /* line too long */ 
        printf("error: line too long\n");
        exit(1);
    }
    /* is there a label? */
    ptr = line;
    if (sscanf(ptr, "%[^\t\n\r ]", label))
    {
        /* successfully read label; advance pointer over the label */
        ptr += strlen(label);
    }
    /*
     * Parse the rest of the line. Would be nice to have real regular
     * expressions, but scanf will suffice. */
    sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]",
           opcode, arg0, arg1, arg2);
    return (1);
}

int isNumber(char *string)
{ /* return 1 if string is a number */
    int i;
    return ((sscanf(string, "%d", &i)) == 1);
}