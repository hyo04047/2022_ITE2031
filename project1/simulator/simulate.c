#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NUMMEMORY 65536 /* maximum number of words in memory */ 
#define NUMREGS 8 /* number of machine registers */
#define MAXLINELENGTH 1000

typedef struct stateStruct
{
    int pc;
    int mem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
} stateType;

void printState(stateType *);
int convertNum(int num);

int main(int argc, char *argv[])
{
    char line[MAXLINELENGTH];
    stateType state;
    FILE *filePtr;

    if (argc != 2)
    {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }

    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL)
    {
        printf("error: can't open file %s", argv[1]);
        perror("fopen");
        exit(1);
    }

    /* read in the entire machine-code file into memory */
    for (state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL; state.numMemory++)
    {
        if (sscanf(line, "%d", state.mem + state.numMemory) != 1)
        {
            printf("error in reading address %d\n", state.numMemory);
            exit(1);
        }
        printf("memory[%d]=%d\n", state.numMemory, state.mem[state.numMemory]);
    }
    /* initialize registers and pc */
    for (int i = 0; i < NUMREGS; i++)
        state.reg[i] = 0;
    state.pc = 0;
    printState(&state);

    int mccode, opcode, rs, rt, rd, offset;
    int ishalt = 0, cnt = 0;
    while (1)
    {
        mccode = state.mem[state.pc];
        opcode = mccode >> 22;
        rs = mccode >> 19 & 0x0007;
        rt = mccode >> 16 & 0x0007;
        rd = mccode & 0x0007;
        offset = mccode & 0xFFFF;
        state.pc++;
        cnt++;

        switch (opcode)
        {
            case 0: /* add */
                state.reg[rd] = state.reg[rs] + state.reg[rt];
                break;
            case 1: /* nor */
                state.reg[rd] = ~state.reg[rs] | state.reg[rt];
                break;
            case 2: /* lw */
                state.reg[rt] = state.mem[convertNum(state.reg[rs] + offset)];
                break;
            case 3: /* sw */
                state.mem[convertNum(state.reg[rs] + offset)] = state.reg[rt];
                break;
            case 4: /* beq */
                if (state.reg[rs] == state.reg[rt])
                    state.pc += convertNum(offset);
                break;
            case 5: /* jalr */
                state.reg[rt] = state.pc + 1;
                state.pc = state.reg[rs];
                break;
            case 6: /* halt */
                ishalt = 1;
                break;
            case 7: /* noop */
                break;
            
            default:
                printf("error: unrecognized opcode\n");
                exit(1);
                break;
        }

        if (ishalt)
            break;
        
        printState(&state);
    }

    printf("machine halted\n");
    printf("total of %d instructions executed\n", cnt);
    printf("final state of machine:\n");
    printState(&state);

    fclose(filePtr);

    return (0);
}

void printState(stateType *statePtr)
{
    int i;
    printf("\n@@@\nstate:\n");
    printf("\tpc %d\n", statePtr->pc);
    printf("\tmemory:\n");
    for (i = 0; i < statePtr->numMemory; i++)
    {
        printf("\t\tmem[ %d ] %d\n", i, statePtr->mem[i]);
    }
    printf("\tregisters:\n");
    for (i = 0; i < NUMREGS; i++)
    {
        printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
    }
    printf("end state\n");
}

int convertNum(int num)
{
    /* convert a 16-bit number into a 32-bit Linux integer */ 
    if (num & (1 << 15))
    {
        num -= (1 << 16);
    }
    return (num);
}