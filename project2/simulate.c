#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NUMMEMORY 65536 /* maximum number of data words in memory */
#define NUMREGS 8       /* number of machine registers */

#define ADD 0
#define NOR 1
#define LW 2
#define SW 3
#define BEQ 4
#define JALR 5 /* JALR will not implemented for this project */
#define HALT 6
#define NOOP 7

#define NOOPINSTRUCTION 0x1c00000

typedef struct IFIDStruct
{
    int instr;
    int pcPlus1;
} IFIDType;

typedef struct IDEXStruct
{
    int instr;
    int pcPlus1;
    int readRegA;
    int readRegB;
    int offset;
} IDEXType;

typedef struct EXMEMStruct
{
    int instr;
    int branchTarget;
    int aluResult;
    int readRegB;
} EXMEMType;

typedef struct MEMWBStruct
{
    int instr;
    int writeData;
} MEMWBType;

typedef struct WBENDStruct
{
    int instr;
    int writeData;
} WBENDType;

typedef struct stateStruct
{
    int pc;
    int instrMem[NUMMEMORY];
    int dataMem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
    IFIDType IFID;
    IDEXType IDEX;
    EXMEMType EXMEM;
    MEMWBType MEMWB;
    WBENDType WBEND;
    int cycles; /* number of cycles run so far */
} stateType;

#define MAXLINELENGTH 1000

void printState(stateType *);
int field0(int instruction);
int field1(int instruction);
int field2(int instruction);
int opcode(int instruction);
void printInstruction(int instr);
int convertNum(int num);

int main(int argc, char *argv[])
{
    char line[MAXLINELENGTH];
    stateType state, newState;
    FILE *filePtr;
    int regA, regB;

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
        if (sscanf(line, "%d", state.instrMem + state.numMemory) != 1)
        {
            printf("error in reading address %d\n", state.numMemory);
            exit(1);
        }
        state.dataMem[state.numMemory] = state.instrMem[state.numMemory];
        printf("memory[%d]=%d\n", state.numMemory, state.instrMem[state.numMemory]);
    }
    /* initialize registers and pc */
    for (int i = 0; i < NUMREGS; i++)
        state.reg[i] = 0;
    state.pc = 0;
    state.cycles = 0;

    state.IFID.instr = NOOPINSTRUCTION;
    state.IDEX.instr = NOOPINSTRUCTION;
    state.EXMEM.instr = NOOPINSTRUCTION;
    state.MEMWB.instr = NOOPINSTRUCTION;
    state.WBEND.instr = NOOPINSTRUCTION;
    state.IFID.pcPlus1 = 0;
    state.IDEX.offset = 0;
    state.IDEX.pcPlus1 = 0;
    state.IDEX.readRegA = 0;
    state.IDEX.readRegB = 0;
    state.EXMEM.aluResult = 0;
    state.EXMEM.branchTarget = 0;
    state.EXMEM.readRegB = 0;
    state.MEMWB.writeData = 0;
    state.WBEND.writeData = 0;

    printf("%d memory words\n", state.numMemory);
    printf("\tinstruction memory:\n");

    for(int i = 0; i < state.numMemory; i++){
        printf("\t\tinstrMem[ %d ] ", i);
        printInstruction(state.instrMem[i]);
    }

    while(1)
    {
        printState(&state);
        
        /* check for halt */
        if(opcode(state.MEMWB.instr) == HALT)
        {
            printf("machine halted\n");
            printf("total of %d cycles executed\n", state.cycles);
            exit(0);
        }

        newState = state;
        newState.cycles++;

        /* --------------------- IF stage --------------------- */
        newState.pc = state.pc + 1;
        newState.IFID.instr = state.instrMem[state.pc];
        newState.IFID.pcPlus1 = state.pc + 1;

        /* --------------------- ID stage --------------------- */
        newState.IDEX.instr = state.IFID.instr;
        newState.IDEX.pcPlus1 = state.IFID.pcPlus1;
        newState.IDEX.readRegA = state.reg[field0(state.IFID.instr)];
        newState.IDEX.readRegB = state.reg[field1(state.IFID.instr)];
        newState.IDEX.offset = convertNum(field2(state.IFID.instr));

        /* --------------------- EX stage --------------------- */
        newState.EXMEM.instr = state.IDEX.instr;
        newState.EXMEM.branchTarget = state.IDEX.offset + state.IDEX.pcPlus1;
        newState.EXMEM.readRegB = state.IDEX.readRegB;

        regA = state.IDEX.readRegA;
        regB = state.IDEX.readRegB;

        switch(opcode(newState.EXMEM.instr)){
            case ADD:
                newState.EXMEM.aluResult = regA + regB;
                break;
            case NOR:
                newState.EXMEM.aluResult = ~(regA | regB);
                break;
            case LW:
            case SW:
                newState.EXMEM.aluResult = regA + state.IDEX.offset;
                break;
            case BEQ:
                newState.EXMEM.aluResult = regA - regB;
                break;
            case JALR:
            case HALT:
            case NOOP:
                break;
            
            default:
                break;
        }

        if((opcode(state.EXMEM.instr) == BEQ) && !state.EXMEM.aluResult){
            // newState.IFID.instr = NOOPINSTRUCTION;
            // newState.IFID.pcPlus1 = 0;
            // newState.IDEX.instr = NOOPINSTRUCTION;
            // newState.IDEX.offset = 0;
            // newState.IDEX.pcPlus1 = 0;
            // newState.IDEX.readRegA = 0;
            // newState.IDEX.readRegB = 0;
            // newState.EXMEM.aluResult = 0;
            // newState.EXMEM.branchTarget = 0;
            // newState.EXMEM.instr = NOOPINSTRUCTION;
            // newState.EXMEM.readRegB = 0;

            newState.pc = state.EXMEM.branchTarget;
        }

        /* --------------------- MEM stage --------------------- */
        newState.MEMWB.instr = state.EXMEM.instr;
        
        switch(opcode(newState.MEMWB.instr)){
            case ADD:
            case NOR:
                newState.MEMWB.writeData = state.EXMEM.aluResult;
                break;
            case LW:
                newState.MEMWB.writeData = state.dataMem[state.EXMEM.aluResult];
                break;
            case SW:
                newState.dataMem[state.EXMEM.aluResult] = state.EXMEM.readRegB;
                break;
            case BEQ:
            case JALR:
            case HALT:
            case NOOP:
                break;
            default:
                break;
        }

        /* --------------------- WB stage --------------------- */
        newState.WBEND.instr = state.MEMWB.instr;
        newState.WBEND.writeData = state.MEMWB.writeData;

        switch(opcode(newState.WBEND.instr)){
            case ADD:
            case NOR:
                newState.reg[field2(newState.WBEND.instr)] = newState.WBEND.writeData;
                break;
            case LW:
                newState.reg[field1(newState.WBEND.instr)] = newState.WBEND.writeData;
            case SW:
            case BEQ:
            case JALR:
            case HALT:
            case NOOP:
                break;
            default:
                break;
        }
        state = newState; /* this is the last statement before end of the loop.
                It marks the end of the cycle and updates the
                current state with the values calculated in this
                cycle */
    }

    printf("machine halted\n");
    printf("Total of %d cycles executed\n", newState.cycles);

    fclose(filePtr);

    return (0);
}

void printState(stateType *statePtr)
{
    int i;
    printf("\n@@@\nstate before cycle %d starts\n", statePtr->cycles);
    printf("\tpc %d\n", statePtr->pc);

    printf("\tdata memory:\n");
    for (i = 0; i < statePtr->numMemory; i++)
    {
        printf("\t\tdataMem[ %d ] %d\n", i, statePtr->dataMem[i]);
    }
    printf("\tregisters:\n");
    for (i = 0; i < NUMREGS; i++)
    {
        printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
    }
    printf("\tIFID:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->IFID.instr);
    printf("\t\tpcPlus1 %d\n", statePtr->IFID.pcPlus1);
    printf("\tIDEX:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->IDEX.instr);
    printf("\t\tpcPlus1 %d\n", statePtr->IDEX.pcPlus1);
    printf("\t\treadRegA %d\n", statePtr->IDEX.readRegA);
    printf("\t\treadRegB %d\n", statePtr->IDEX.readRegB);
    printf("\t\toffset %d\n", statePtr->IDEX.offset);
    printf("\tEXMEM:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->EXMEM.instr);
    printf("\t\tbranchTarget %d\n", statePtr->EXMEM.branchTarget);
    printf("\t\taluResult %d\n", statePtr->EXMEM.aluResult);
    printf("\t\treadRegB %d\n", statePtr->EXMEM.readRegB);
    printf("\tMEMWB:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->MEMWB.instr);
    printf("\t\twriteData %d\n", statePtr->MEMWB.writeData);
    printf("\tWBEND:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->WBEND.instr);
    printf("\t\twriteData %d\n", statePtr->WBEND.writeData);
}

int field0(int instruction)
{
    return ((instruction >> 19) & 0x7);
}

int field1(int instruction)
{
    return ((instruction >> 16) & 0x7);
}

int field2(int instruction)
{
    return (instruction & 0xFFFF);
}

int opcode(int instruction)
{
    return (instruction >> 22);
}

void printInstruction(int instr)
{

    char opcodeString[10];

    if (opcode(instr) == ADD){
        strcpy(opcodeString, "add");
    }
    else if (opcode(instr) == NOR){
        strcpy(opcodeString, "nor");
    }
    else if (opcode(instr) == LW){
        strcpy(opcodeString, "lw");
    }
    else if (opcode(instr) == SW){
        strcpy(opcodeString, "sw");
    }
    else if (opcode(instr) == BEQ){
        strcpy(opcodeString, "beq");
    }
    else if (opcode(instr) == JALR){
        strcpy(opcodeString, "jalr");
    }
    else if (opcode(instr) == HALT){
        strcpy(opcodeString, "halt");
    }
    else if (opcode(instr) == NOOP){
        strcpy(opcodeString, "noop");
    }
    else{
        strcpy(opcodeString, "data");
    }
    printf("%s %d %d %d\n", opcodeString, field0(instr), field1(instr),
           field2(instr));
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