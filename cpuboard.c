/*
 *	Project-based Learning II (CPU)
 *
 *	Program:	instruction set simulator of the Educational CPU Board
 *	File Name:	cpuboard.c
 *	Descrioption:	simulation(emulation) of an instruction
 */
#include	<stdio.h>
#include 	<string.h>
#include	"cpuboard.h"

/* 各種変数・関数の定義 */
Uword A_Instruction;	/* 1語目Aの命令 */
Uword B_Instruction;	/* 1語目Bの命令 */
Uword ShiftMode;		/* Shift mode */
Uword BranchCondition;	/* Branch Condition */
Bit JAL_is_running = 0;	/* サブルーチン命令が実行中であるか */

int LD_Instruction(Cpub *);												/* LD命令 */
int ST_Instruction(Cpub *);												/* ST命令 */
int ADD_Instruction(Cpub *);											/* ADD命令 */
int ADC_Instruction(Cpub *);											/* ADC命令 */
int SUB_Instruction(Cpub *);											/* SUB命令 */
int SBC_Instruction(Cpub *);											/* SBC命令 */
int AND_Instruction(Cpub *);											/* AND命令 */
int OR_Instruction(Cpub *);												/* OR命令 */
int EOR_Instruction(Cpub *);											/* EOR命令 */
int CMP_Instruction(Cpub *);											/* CMP命令 */
int Shift_Instruction(Cpub *);											/* Shift命令 */
int Rotate_Instruction(Cpub *);											/* Rotation命令 */
int Branch_Instruction(Cpub *);											/* 分岐命令 */
int JAL_Instruction(Cpub *);											/* JAL命令 */
Uword instruction_decoding(Uword ir);									/* 命令解読機構 */
Uword Set_A_Value(Cpub *);						/* A値のセット */
void Write_A_Value(Cpub *cpub, Uword ALU_result);						/* ACC or IX に処理結果を書き込む */
void Set_Flags(Cpub *cpub, Bit Carry, Bit Over, Uword ALU_result);		/* CF,VF,NF,ZF のセット */

/*=============================================================================
 *   Simulation of a Single Instruction
 *===========================================================================*/
int step(Cpub *cpub)
{
	/*
	 *   [ add codes here for the simulation of an instruction ]
	 *   [ return RUN_STEP or RUN_HALT ]
	 */
	int RUN_Return = RUN_STEP;

	/* P0 Phase */
	cpub->mar = cpub->pc;
	cpub->pc++;

	/* P1 Phase */
	cpub->ir = (Uword)cpub->mem[0x000 + cpub->mar];		//Program領域へのアクセス

	/* 命令語の解析 */
	Uword IR = instruction_decoding(cpub->ir);

	/* 分岐処理 */
	switch(IR)
	{
		case NOP:
			fprintf(stderr, "execute NOP\n");
			RUN_Return = RUN_STEP;
			break;
		case HLT:
			fprintf(stderr, "execute HLT\n");
			RUN_Return = RUN_HALT;
			break;
		case JAL:
			fprintf(stderr, "execute JAL\n");
			RUN_Return = JAL_Instruction(cpub);
			if(RUN_Return == RUN_HALT) fprintf(stderr, "Failed: JAL Instruction\n");
			break;
		case JR:
			fprintf(stderr, "execute JR\n");
			// JALが実行するサブルーチン中に呼び出されたか
			if(JAL_is_running == 1){
				cpub->pc = cpub->acc;
				RUN_Return = RUN_STEP;
			}else{
				fprintf(stderr, "Error: JR Instruction is denied. Because JAL Instruction is not running.\n");
				RUN_Return = RUN_HALT;
			}
			break;
		case OUT:
			fprintf(stderr, "execute OUT\n");
			cpub->obuf.buf = cpub->acc;
			cpub->obuf.flag = 1;
			RUN_Return = RUN_STEP;
			break;
		case IN:
			fprintf(stderr, "execute IN\n");
			cpub->acc = cpub->ibuf->buf;
			cpub->ibuf->flag = 0;
			RUN_Return = RUN_STEP;
			break;
		case RCF:
			fprintf(stderr, "execute RCF\n");
			cpub->cf = 0;
			RUN_Return = RUN_STEP;
			break;
		case SCF:
			fprintf(stderr, "execute SCF\n");
			cpub->cf = 1;
			RUN_Return = RUN_STEP;
			break;
		case Bbc:
			fprintf(stderr, "execute Bbc\n");
			RUN_Return = Branch_Instruction(cpub);
			if(RUN_Return == RUN_HALT) fprintf(stderr, "Failed: Branch Instruction\n");
			break;
		case Ssm:
		 	fprintf(stderr, "execute Ssm\n");
			RUN_Return = Shift_Instruction(cpub);
			if(RUN_Return == RUN_HALT) fprintf(stderr, "Failed: Shift Instruction\n");
		 	break;
		case Rsm:
		 	fprintf(stderr, "execute Rsm\n");
			RUN_Return = Rotate_Instruction(cpub);
			if(RUN_Return == RUN_HALT) fprintf(stderr, "Failed: Rotate Instruction\n");
			break;
		case LD:
			fprintf(stderr, "execute LD\n");
			RUN_Return = LD_Instruction(cpub);
			if(RUN_Return == RUN_HALT) fprintf(stderr, "Failed: LD Instruction\n");
			break;
		case ST:
			fprintf(stderr, "execute ST\n");
			RUN_Return = ST_Instruction(cpub);
			if(RUN_Return == RUN_HALT) fprintf(stderr, "Failed: ST Instruction\n");
			break;
		case SBC:
			fprintf(stderr, "execute SBC\n");
			RUN_Return = SBC_Instruction(cpub);
			if(RUN_Return == RUN_HALT) fprintf(stderr, "Failed: SBC Instruction\n");
			break;
		case ADC:
			fprintf(stderr, "execute ADC\n");
			RUN_Return = ADC_Instruction(cpub);
			if(RUN_Return == RUN_HALT) fprintf(stderr, "Failed: ADC Instruction\n");
			break;
		case SUB:
			fprintf(stderr, "execute SUB\n");
			RUN_Return = SUB_Instruction(cpub);
			if(RUN_Return == RUN_HALT) fprintf(stderr, "Failed: SUB Instruction\n");
			break;
		case ADD:
			fprintf(stderr, "execute ADD\n");
			RUN_Return = ADD_Instruction(cpub);
			if(RUN_Return == RUN_HALT) fprintf(stderr, "Failed: ADD Instruction\n");
			break;
		case EOR:
			fprintf(stderr, "execute EOR\n");
			RUN_Return = EOR_Instruction(cpub);
			if(RUN_Return == RUN_HALT) fprintf(stderr, "Failed: EOR Instruction\n");
			break;
		case OR:
			fprintf(stderr, "execute OR\n");
			RUN_Return = OR_Instruction(cpub);
			if(RUN_Return == RUN_HALT) fprintf(stderr, "Failed: OR Instruction\n");
			break;
		case AND:
			fprintf(stderr, "execute AND\n");
			RUN_Return = AND_Instruction(cpub);
			if(RUN_Return == RUN_HALT) fprintf(stderr, "Failed: AND Instruction\n");
			break;
		case CMP:
			fprintf(stderr, "execute CMP\n");
			RUN_Return = CMP_Instruction(cpub);
			if(RUN_Return == RUN_HALT) fprintf(stderr, "Failed: CMP Instruction\n");
			break;
		default:
			/* エラー処理 */
			fprintf(stderr, "Error\n");
			RUN_Return = RUN_HALT;
			break;
	}
	return RUN_Return;
}

int LD_Instruction(Cpub *cpub){
	Uword Second_word;
	Uword ALU_result;

	/* P2 Phase */
	if(!(B_Instruction == ACC) && !(B_Instruction == IX)){
		cpub->mar = cpub->pc;
		cpub->pc++;
		Second_word = cpub->mem[0x000 + cpub->mar];
	}

	/* P2~P4 Phase */
	switch(B_Instruction)
	{
		case ACC:
			ALU_result = cpub->acc;
			break;
		case IX:
			ALU_result = cpub->ix;
			break;
		case Immediate_d:
			ALU_result = Second_word;
			break;
		case Program_Absolute_d:
			cpub->mar = Second_word;
			ALU_result = cpub->mem[0x000+cpub->mar];
			break;
		case Data_Absolute_d:
			cpub->mar = Second_word;
			ALU_result = cpub->mem[0x100+cpub->mar];
			break;
		case Program_IX:
			cpub->mar = Second_word + cpub->ix;
			ALU_result = cpub->mem[0x000+cpub->mar];
			break;
		case Data_IX:
			cpub->mar = Second_word + cpub->ix;
			ALU_result = cpub->mem[0x100+cpub->mar];
			break;
		default:
			fprintf(stderr,"Error: OP_B(%x) was not defined in LD Instruction\n",B_Instruction);
			return RUN_HALT;
			break;
	}
	Write_A_Value(cpub, ALU_result);
	return RUN_STEP;
}

int ST_Instruction(Cpub *cpub){	
	Uword Second_word;
	Uword A_Value = Set_A_Value(cpub);		/* 1 -> IX, 0 -> ACC */

	/* P2 Phase */
	cpub->mar = cpub->pc;
	cpub->pc++;
	Second_word = cpub->mem[0x000 + cpub->mar];

	/* P3,P4 Phase */
	switch (B_Instruction)
	{
	case Program_Absolute_d :
		cpub->mar = Second_word;
		cpub->mem[0x000 + cpub->mar] = A_Value;
		break;

	case Data_Absolute_d :
		cpub->mar = Second_word;
		cpub->mem[0x100 + cpub->mar] = A_Value;
		break;

	case Program_IX :
		cpub->mar = Second_word + cpub->ix;
		cpub->mem[0x000 + cpub->mar] = A_Value;
		break;

	case Data_IX :
		cpub->mar = Second_word + cpub->ix;
		cpub->mem[0x100 + cpub->mar] = A_Value;
		break;

	default:
		fprintf(stderr,"Error: OP_B(%x) was not defined in ST Instruction\n",B_Instruction);
		return RUN_HALT;
		break;
	}
	return RUN_STEP;
}

int ADD_Instruction(Cpub *cpub){
	Uword Second_word;
	Uword A_Value = Set_A_Value(cpub);
	Uword ALU_result;
	Bit OF;			/* Over Flow Flag */
	Bit A,B,C;		/* A:第1OPのMSB B:第2OPのMSB C:ALU_resultのMSB */

	/* P2 Phase */
	if(!(B_Instruction == ACC) && !(B_Instruction == IX)){
		cpub->mar = cpub->pc;
		cpub->pc++;
		Second_word = cpub->mem[0x000 + cpub->mar];
	}
	/* P2~P4 Phase */
	switch(B_Instruction)
	{
		case ACC:
			ALU_result = A_Value + cpub->acc;
			A = A_Value >> 7;
			B = cpub->acc >> 7;
			C = ALU_result >> 7;
			break;
		case IX:
			ALU_result = A_Value + cpub->ix;
			A = A_Value >> 7;
			B = cpub->ix >> 7;
			C = ALU_result >> 7;
			break;
		case Immediate_d:
			ALU_result = A_Value + Second_word;
			A = A_Value >> 7;
			B = Second_word >> 7;
			C = ALU_result >> 7;
			break;
		case Program_Absolute_d:
			cpub->mar = Second_word;
			ALU_result = A_Value + cpub->mem[0x000+cpub->mar];
			A = A_Value >> 7;
			B = cpub->mem[0x000+cpub->mar] >> 7;
			C = ALU_result >> 7;
			break;
		case Data_Absolute_d:
			cpub->mar = Second_word;
			ALU_result = A_Value + cpub->mem[0x100+cpub->mar];
			A = A_Value >> 7;
			B = cpub->mem[0x100+cpub->mar] >> 7;
			C = ALU_result >> 7;
			break;
		case Program_IX:
			cpub->mar = Second_word + cpub->ix;
			ALU_result = A_Value + cpub->mem[0x000+cpub->mar];
			A = A_Value >> 7;
			B = cpub->mem[0x000+cpub->mar] >> 7;
			C = ALU_result >> 7;
			break;
		case Data_IX:
			cpub->mar = Second_word + cpub->ix;
			ALU_result = A_Value + cpub->mem[0x100+cpub->mar];
			A = A_Value >> 7;
			B = cpub->mem[0x100+cpub->mar] >> 7;
			C = ALU_result >> 7;
			break;
		default:
			fprintf(stderr,"Error: OP_B(%x) was not defined in ADD Instruction\n",B_Instruction);
			return RUN_HALT;
			break;
	}

	OF = (A & B & (~C)) | ((~A) & (~B) & C);
	Write_A_Value(cpub, ALU_result);
	Set_Flags(cpub, cpub->cf, OF, ALU_result);
	return RUN_STEP;
}
int ADC_Instruction(Cpub *cpub){
	Uword Second_word;
	Uword A_Value = Set_A_Value(cpub);
	Uword ALU_result;
	Bit CF,OF;			/* Carry Flag, Over Flow Flag */
	Bit A,B,C;			/* A:第1OPのMSB B:第2OPのMSB C:ALU_resultのMSB */
	/* 
	 *	Except_MSB_OP1:MSBを除いた第1OP
	 *  Except_MSB_OP2:MSBを除いた第2OP
	 */
	Uword Except_MSB_OP1,Except_MSB_OP2;
	Bit CY;				/* CY:MSBよりも一つ低いビットからの桁上がり */

	/* P2 Phase */
	if(!(B_Instruction == ACC) && !(B_Instruction == IX)){
		cpub->mar = cpub->pc;
		cpub->pc++;
		Second_word = cpub->mem[0x000 + cpub->mar];
	}
	/* P2~P4 Phase */
	switch(B_Instruction)
	{
		case ACC:
			ALU_result = A_Value + cpub->acc + cpub->cf;
			A = A_Value >> 7;
			B = cpub->acc >> 7;
			C = ALU_result >> 7;
			Except_MSB_OP1 = A_Value & 0b1111111;
			Except_MSB_OP2 = cpub->acc & 0b1111111;
			break;
		case IX:
			ALU_result = A_Value + cpub->ix + cpub->cf;
			A = A_Value >> 7;
			B = cpub->ix >> 7;
			C = ALU_result >> 7;
			Except_MSB_OP1 = A_Value & 0b1111111;
			Except_MSB_OP2 = cpub->ix & 0b1111111;
			break;
		case Immediate_d:
			ALU_result = A_Value + Second_word + cpub->cf;
			A = A_Value >> 7;
			B = Second_word >> 7;
			C = ALU_result >> 7;
			Except_MSB_OP1 = A_Value & 0b1111111;
			Except_MSB_OP2 = Second_word & 0b1111111;
			break;
		case Program_Absolute_d:
			cpub->mar = Second_word;
			ALU_result = A_Value + cpub->mem[0x000+cpub->mar] + cpub->cf;
			A = A_Value >> 7;
			B = cpub->mem[0x000+cpub->mar] >> 7;
			C = ALU_result >> 7;
			Except_MSB_OP1 = A_Value & 0b1111111;
			Except_MSB_OP2 = cpub->mem[0x000+cpub->mar] & 0b1111111;
			break;
		case Data_Absolute_d:
			cpub->mar = Second_word;
			ALU_result = A_Value + cpub->mem[0x100+cpub->mar] + cpub->cf;
			A = A_Value >> 7;
			B = cpub->mem[0x100+cpub->mar] >> 7;
			C = ALU_result >> 7;
			Except_MSB_OP1 = A_Value & 0b1111111;
			Except_MSB_OP2 = cpub->mem[0x100+cpub->mar] & 0b1111111;
			break;
		case Program_IX:
			cpub->mar = Second_word + cpub->ix;
			ALU_result = A_Value + cpub->mem[0x000+cpub->mar] + cpub->cf;
			A = A_Value >> 7;
			B = cpub->mem[0x000+cpub->mar] >> 7;
			C = ALU_result >> 7;
			Except_MSB_OP1 = A_Value & 0b1111111;
			Except_MSB_OP2 = cpub->mem[0x000+cpub->mar] & 0b1111111;
			break;
		case Data_IX:
			cpub->mar = Second_word + cpub->ix;
			ALU_result = A_Value + cpub->mem[0x100+cpub->mar] + cpub->cf;
			A = A_Value >> 7;
			B = cpub->mem[0x100+cpub->mar] >> 7;
			C = ALU_result >> 7;
			Except_MSB_OP1 = A_Value & 0b1111111;
			Except_MSB_OP2 = cpub->mem[0x100+cpub->mar] & 0b1111111;
			break;
		default:
			fprintf(stderr,"Error: OP_B(%x) was not defined in ADC Instruction\n",B_Instruction);
			return RUN_HALT;
			break;
	}
	CY = (Except_MSB_OP1 + Except_MSB_OP2) >> 7;
	CF = (A & B) | (A & CY) | (B & CY);
	OF = (A & B & (~C)) | ((~A) & (~B) & C);
	Write_A_Value(cpub, ALU_result);
	Set_Flags(cpub, CF, OF, ALU_result);
	return RUN_STEP;
}

int SUB_Instruction(Cpub *cpub){
	Uword Second_word;
	Uword A_Value = Set_A_Value(cpub);
	Uword ALU_result;
	Bit OF;			/* Over Flow Flag */
	Bit A,B,C;		/* A:第1OPのMSB B:第2OPのMSB C:ALU_resultのMSB */

	/* P2 Phase */
	if(!(B_Instruction == ACC) && !(B_Instruction == IX)){
		cpub->mar = cpub->pc;
		cpub->pc++;
		Second_word = cpub->mem[0x000 + cpub->mar];
	}
	/* P2~P4 Phase */
	switch(B_Instruction)
	{
		case ACC:
			ALU_result = A_Value - cpub->acc;
			A = A_Value >> 7;
			B = cpub->acc >> 7;
			C = ALU_result >> 7;
			break;
		case IX:
			ALU_result = A_Value - cpub->ix;
			A = A_Value >> 7;
			B = cpub->ix >> 7;
			C = ALU_result >> 7;
			break;
		case Immediate_d:
			ALU_result = A_Value - Second_word;
			A = A_Value >> 7;
			B = Second_word >> 7;
			C = ALU_result >> 7;
			break;
		case Program_Absolute_d:
			cpub->mar = Second_word;
			ALU_result = A_Value - cpub->mem[0x000+cpub->mar];
			A = A_Value >> 7;
			B = cpub->mem[0x000+cpub->mar] >> 7;
			C = ALU_result >> 7;
			break;
		case Data_Absolute_d:
			cpub->mar = Second_word;
			ALU_result = A_Value - cpub->mem[0x100+cpub->mar];
			A = A_Value >> 7;
			B = cpub->mem[0x100+cpub->mar] >> 7;
			C = ALU_result >> 7;
			break;
		case Program_IX:
			cpub->mar = Second_word + cpub->ix;
			ALU_result = A_Value - cpub->mem[0x000+cpub->mar];
			A = A_Value >> 7;
			B = cpub->mem[0x000+cpub->mar] >> 7;
			C = ALU_result >> 7;
			break;
		case Data_IX:
			cpub->mar = Second_word + cpub->ix;
			ALU_result = A_Value - cpub->mem[0x100+cpub->mar];
			A = A_Value >> 7;
			B = cpub->mem[0x100+cpub->mar] >> 7;
			C = ALU_result >> 7;
			break;
		default:
			fprintf(stderr,"Error: OP_B(%x) was not defined in SUB Instruction\n",B_Instruction);
			return RUN_HALT;
			break;
	}

	OF = (A & B & (~C)) | ((~A) & (~B) & C);
	Write_A_Value(cpub, ALU_result);
	Set_Flags(cpub, cpub->cf, OF, ALU_result);
	return RUN_STEP;
}

int SBC_Instruction(Cpub *cpub){
	Uword Second_word;
	Uword A_Value = Set_A_Value(cpub);
	Uword B_Value;
	Uword ALU_result;
	Bit CF,OF;			/* Carry Flag, Over Flow Flag */
	Bit A,B,C;			/* A:第1OPのMSB B:第2OPのMSB C:ALU_resultのMSB */

	/* P2 Phase */
	if(!(B_Instruction == ACC) && !(B_Instruction == IX)){
		cpub->mar = cpub->pc;
		cpub->pc++;
		Second_word = cpub->mem[0x000 + cpub->mar];
	}
	/* P2~P4 Phase */
	switch(B_Instruction)
	{
		case ACC:
			ALU_result = A_Value - cpub->acc - cpub->cf;
			B_Value = cpub->acc - cpub->cf;
			A = A_Value >> 7;
			B = cpub->acc >> 7;
			C = ALU_result >> 7;
			break;
		case IX:
			ALU_result = A_Value - cpub->ix - cpub->cf;
			B_Value = cpub->ix - cpub->cf;
			A = A_Value >> 7;
			B = cpub->ix >> 7;
			C = ALU_result >> 7;
			break;
		case Immediate_d:
			ALU_result = A_Value - Second_word - cpub->cf;
			B_Value = Second_word - cpub->cf;
			A = A_Value >> 7;
			B = Second_word >> 7;
			C = ALU_result >> 7;
			break;
		case Program_Absolute_d:
			cpub->mar = Second_word;
			ALU_result = A_Value - cpub->mem[0x000+cpub->mar] - cpub->cf;
			B_Value = cpub->mem[0x000+cpub->mar] - cpub->cf;
			A = A_Value >> 7;
			B = cpub->mem[0x000+cpub->mar] >> 7;
			C = ALU_result >> 7;
			break;
		case Data_Absolute_d:
			cpub->mar = Second_word;
			ALU_result = A_Value - cpub->mem[0x100+cpub->mar] - cpub->cf;
			B_Value = cpub->mem[0x100+cpub->mar] - cpub->cf;
			A = A_Value >> 7;
			B = cpub->mem[0x100+cpub->mar] >> 7;
			C = ALU_result >> 7;
			break;
		case Program_IX:
			cpub->mar = Second_word + cpub->ix;
			ALU_result = A_Value - cpub->mem[0x000+cpub->mar] - cpub->cf;
			B_Value = cpub->mem[0x000+cpub->mar] - cpub->cf;
			A = A_Value >> 7;
			B = cpub->mem[0x000+cpub->mar] >> 7;
			C = ALU_result >> 7;
			break;
		case Data_IX:
			cpub->mar = Second_word + cpub->ix;
			ALU_result = A_Value - cpub->mem[0x100+cpub->mar] - cpub->cf;
			B_Value = cpub->mem[0x100+cpub->mar] - cpub->cf;
			A = A_Value >> 7;
			B = cpub->mem[0x100+cpub->mar] >> 7;
			C = ALU_result >> 7;
			break;
		default:
			fprintf(stderr,"Error: OP_B(%x) was not defined in SBC Instruction\n",B_Instruction);
			return RUN_HALT;
			break;
	}
	if(B_Value > A_Value) CF = 0x01;
	else CF = 0x00;
	OF = (A & B & (~C)) | ((~A) & (~B) & C);
	Write_A_Value(cpub, ALU_result);
	Set_Flags(cpub, CF, OF, ALU_result);
	return RUN_STEP;
}

int AND_Instruction(Cpub *cpub){
	Uword Second_word;
	Uword A_Value = Set_A_Value(cpub);
	Uword ALU_result;

	/* P2 Phase */
	if(!(B_Instruction == ACC) && !(B_Instruction == IX)){
		cpub->mar = cpub->pc;
		cpub->pc++;
		Second_word = cpub->mem[0x000 + cpub->mar];
	}

	/* P2~P4 Phase */
	switch(B_Instruction)
	{
		case ACC:
			ALU_result = A_Value & cpub->acc;
			break;
		case IX:
			ALU_result = A_Value & cpub->ix;
			break;
		case Immediate_d:
			ALU_result = A_Value & Second_word;
			break;
		case Program_Absolute_d:
			cpub->mar = Second_word;
			ALU_result = A_Value & cpub->mem[0x000+cpub->mar];
			break;
		case Data_Absolute_d:
			cpub->mar = Second_word;
			ALU_result = A_Value & cpub->mem[0x100+cpub->mar];
			break;
		case Program_IX:
			cpub->mar = Second_word + cpub->ix;
			ALU_result = A_Value & cpub->mem[0x000+cpub->mar];
			break;
		case Data_IX:
			cpub->mar = Second_word + cpub->ix;
			ALU_result = A_Value & cpub->mem[0x100+cpub->mar];
			break;
		default:
			fprintf(stderr,"Error: OP_B(%x) was not defined in AND Instruction\n",B_Instruction);
			return RUN_HALT;
			break;
	}
	Write_A_Value(cpub, ALU_result);
	Set_Flags(cpub, cpub->cf, 0, ALU_result);
	return RUN_STEP;
}

int OR_Instruction(Cpub *cpub){
	Uword Second_word;
	Uword A_Value = Set_A_Value(cpub);
	Uword ALU_result;

	/* P2 Phase */
	if(!(B_Instruction == ACC) && !(B_Instruction == IX)){
		cpub->mar = cpub->pc;
		cpub->pc++;
		Second_word = cpub->mem[0x000 + cpub->mar];
	}

	/* P2~P4 Phase */
	switch(B_Instruction)
	{
		case ACC:
			ALU_result = A_Value | cpub->acc;
			break;
		case IX:
			ALU_result = A_Value | cpub->ix;
			break;
		case Immediate_d:
			ALU_result = A_Value | Second_word;
			break;
		case Program_Absolute_d:
			cpub->mar = Second_word;
			ALU_result = A_Value | cpub->mem[0x000+cpub->mar];
			break;
		case Data_Absolute_d:
			cpub->mar = Second_word;
			ALU_result = A_Value | cpub->mem[0x100+cpub->mar];
			break;
		case Program_IX:
			cpub->mar = Second_word + cpub->ix;
			ALU_result = A_Value | cpub->mem[0x000+cpub->mar];
			break;
		case Data_IX:
			cpub->mar = Second_word + cpub->ix;
			ALU_result = A_Value | cpub->mem[0x100+cpub->mar];
			break;
		default:
			fprintf(stderr,"Error: OP_B(%x) was not defined in OR Instruction\n",B_Instruction);
			return RUN_HALT;
			break;
	}
	Write_A_Value(cpub, ALU_result);
	Set_Flags(cpub, cpub->cf, 0, ALU_result);
	return RUN_STEP;
}

int EOR_Instruction(Cpub *cpub){
	Uword Second_word;
	Uword A_Value = Set_A_Value(cpub);
	Uword ALU_result;

	/* P2 Phase */
	if(!(B_Instruction == ACC) && !(B_Instruction == IX)){
		cpub->mar = cpub->pc;
		cpub->pc++;
		Second_word = cpub->mem[0x000 + cpub->mar];
	}

	/* P2~P4 Phase */
	switch(B_Instruction)
	{
		case ACC:
			ALU_result = A_Value ^ cpub->acc;
			break;
		case IX:
			ALU_result = A_Value ^ cpub->ix;
			break;
		case Immediate_d:
			ALU_result = A_Value ^ Second_word;
			break;
		case Program_Absolute_d:
			cpub->mar = Second_word;
			ALU_result = A_Value ^ cpub->mem[0x000+cpub->mar];
			break;
		case Data_Absolute_d:
			cpub->mar = Second_word;
			ALU_result = A_Value ^ cpub->mem[0x100+cpub->mar];
			break;
		case Program_IX:
			cpub->mar = Second_word + cpub->ix;
			ALU_result = A_Value ^ cpub->mem[0x000+cpub->mar];
			break;
		case Data_IX:
			cpub->mar = Second_word + cpub->ix;
			ALU_result = A_Value ^ cpub->mem[0x100+cpub->mar];
			break;
		default:
			fprintf(stderr,"Error: OP_B(%x) was not defined in EOR Instruction\n",B_Instruction);
			return RUN_HALT;
			break;
	}
	Write_A_Value(cpub, ALU_result);
	Set_Flags(cpub, cpub->cf, 0, ALU_result);
	return RUN_STEP;
}

int CMP_Instruction(Cpub *cpub){
	Uword Second_word;
	Uword A_Value = Set_A_Value(cpub);
	Uword ALU_result;
	Bit OF;			/* Over Flow Flag */
	Bit A,B,C;		/* A:第1OPのMSB B:第2OPのMSB C:ALU_resultのMSB */

	/* P2 Phase */
	if(!(B_Instruction == ACC) && !(B_Instruction == IX)){
		cpub->mar = cpub->pc;
		cpub->pc++;
		Second_word = cpub->mem[0x000 + cpub->mar];
	}
	/* P2~P4 Phase */
	switch(B_Instruction)
	{
		case ACC:
			ALU_result = A_Value - cpub->acc;
			A = A_Value >> 7;
			B = cpub->acc >> 7;
			C = ALU_result >> 7;
			break;
		case IX:
			ALU_result = A_Value - cpub->ix;
			A = A_Value >> 7;
			B = cpub->ix >> 7;
			C = ALU_result >> 7;
			break;
		case Immediate_d:
			ALU_result = A_Value - Second_word;
			A = A_Value >> 7;
			B = Second_word >> 7;
			C = ALU_result >> 7;
			break;
		case Program_Absolute_d:
			cpub->mar = Second_word;
			ALU_result = A_Value - cpub->mem[0x000+cpub->mar];
			A = A_Value >> 7;
			B = cpub->mem[0x000+cpub->mar] >> 7;
			C = ALU_result >> 7;
			break;
		case Data_Absolute_d:
			cpub->mar = Second_word;
			ALU_result = A_Value - cpub->mem[0x100+cpub->mar];
			A = A_Value >> 7;
			B = cpub->mem[0x100+cpub->mar] >> 7;
			C = ALU_result >> 7;
			break;
		case Program_IX:
			cpub->mar = Second_word + cpub->ix;
			ALU_result = A_Value - cpub->mem[0x000+cpub->mar];
			A = A_Value >> 7;
			B = cpub->mem[0x000+cpub->mar] >> 7;
			C = ALU_result >> 7;
			break;
		case Data_IX:
			cpub->mar = Second_word + cpub->ix;
			ALU_result = A_Value - cpub->mem[0x100+cpub->mar];
			A = A_Value >> 7;
			B = cpub->mem[0x100+cpub->mar] >> 7;
			C = ALU_result >> 7;
			break;
		default:
			fprintf(stderr,"Error: OP_B(%x) was not defined in CMP Instruction\n",B_Instruction);
			return RUN_HALT;
			break;
	}

	OF = (A & B & (~C)) | ((~A) & (~B) & C);
	Write_A_Value(cpub, ALU_result);
	Set_Flags(cpub, cpub->cf, OF, ALU_result);
	return RUN_STEP;
}
int Shift_Instruction(Cpub *cpub){
	Uword A_value = Set_A_Value(cpub);
	Uword ALU_result;
	Bit CF;				/* Carry Flag */
	Bit OF = 0;			/* Over Flow Flag */
	Bit A_MSB;				/* A_MSB:A_valueのMSB or 第6ビット(LA) */
	
	switch(ShiftMode){
		case(RA):
			CF = A_value & 0b00000001;
			A_MSB = A_value & 0b10000000 >> 7;
			ALU_result = (A_value >> 1) + A_MSB;
			break;
		case(LA):
			CF = A_value & 0b10000000 >> 7;
			OF = A_value & 0b10000000 >> 7;
			ALU_result = A_value << 1;
			break;
		case(RL):
			CF = A_value & 0b00000001;
			ALU_result = A_value >> 1;
			break;
		case(LL):
			CF = A_value & 0b10000000 >> 7;
			ALU_result = A_value << 1;
			break;
		default:
			fprintf(stderr,"Error: ShiftMode(%x) was not defined in Shift Instruction\n",ShiftMode);
			return RUN_HALT;
			break;
	}
	Write_A_Value(cpub, ALU_result);
	Set_Flags(cpub,CF,OF,ALU_result);
	return RUN_STEP;
}

int Rotate_Instruction(Cpub *cpub){
	Uword A_value = Set_A_Value(cpub);
	Uword ALU_result;
	Bit CF;				/* Carry Flag */
	Bit OF = 0;			/* Over Flow Flag */
	switch(ShiftMode){
		case(RA):
			CF = A_value & 0b00000001;
			ALU_result = (A_value >> 1) + (CF << 7);
			break;
		case(LA):
			CF = A_value & 0b10000000 >> 7;
			OF = A_value & 0b10000000 >> 7;
			ALU_result = (A_value << 1) + CF;
			break;
		case(RL):
			CF = A_value & 0b00000001;
			ALU_result = (A_value >> 1) + (CF << 7);
			break;
		case(LL):
			CF = A_value & 0b10000000 >> 7;
			ALU_result = (A_value << 1) + CF;
			break;
		default:
			fprintf(stderr,"Error: ShiftMode(%x) was not defined in Rotate Instruction\n",ShiftMode);
			return RUN_HALT;
			break;
	}
	Write_A_Value(cpub, ALU_result);
	Set_Flags(cpub,CF,OF,ALU_result);
	return RUN_STEP;
}

int Branch_Instruction(Cpub *cpub){
	Uword Second_word;
	
	/* P2 Phase */
	cpub->mar = cpub->pc;
	cpub->pc++;
	Second_word = cpub->mem[0x000 + cpub->mar];
	
	/* P3 Phase */
	switch(BranchCondition)
	{
		case BC_A:
			cpub->pc = Second_word;
			break;
		case BC_VF:
			if(cpub->vf) cpub->pc = Second_word;
			break;
		case BC_NZ:
			if(!(cpub->zf)) cpub->pc = Second_word;
			break;
		case BC_Z:
			if(cpub->zf) cpub->pc = Second_word;
			break;
		case BC_ZP:
			if(!(cpub->nf)) cpub->pc = Second_word;
			break;
		case BC_N:
			if(cpub->nf) cpub->pc = Second_word;
			break;
		case BC_P:
			if(!(cpub->nf & cpub->zf)) cpub->pc = Second_word;
			break;
		case BC_ZN:
			if(cpub->nf & cpub->zf) cpub->pc = Second_word;
			break;
		case BC_NI:
			if(!(cpub->ibuf->flag)) cpub->pc = Second_word;
			break;
		case BC_NO:
			if(cpub->obuf.flag) cpub->pc = Second_word;
			break;
		case BC_NC:
			if(!(cpub->cf)) cpub->pc = Second_word;
			break;
		case BC_C:
			if(cpub->cf) cpub->pc = Second_word;
			break;
		case BC_CE:
			if(!(cpub->vf ^ cpub->nf)) cpub->pc = Second_word;
			break;
		case BC_LT:
			if(cpub->vf ^ cpub->nf) cpub->pc = Second_word;
			break;
		case BC_GT:
			if(!((cpub->vf ^ cpub->nf) | cpub->zf)) cpub->pc = Second_word;
			break;
		case BC_LE:
			if((cpub->vf ^ cpub->nf) | cpub->zf) cpub->pc = Second_word;
			break;
		default:
			fprintf(stderr,"Error: Branch_Condition(%x) was not defined in Branch Instruction\n", BranchCondition);
			return RUN_HALT;
			break;
	}
	return RUN_STEP;
}

int JAL_Instruction(Cpub *cpub){
	Uword Second_word;

	/* P2 Phase */
	cpub->mar = cpub->pc;
	cpub->pc++;
	Second_word = cpub->mem[0x000 + cpub->mar];

	/* P3 Phase */
	// サブルーチン実行後に戻る位置をaccに保存
	cpub->acc = cpub->pc;

	/* P4 Phase */
	// サブルーチン実行位置をpcに指定
	cpub->pc = Second_word;
	JAL_is_running = 1;
	return RUN_STEP;
}

/* 命令解読を行う関数 */
Uword instruction_decoding(Uword ir){

	Uword Decode_Return = NOP;

	/* 上位4bitの取り出し */
	Uword mask_upper4 = 0xf0;
	Uword inst_code_upper4 = ir & mask_upper4;
	/* 下位4bitの取り出し */
	Uword mask_lower4 = 0x0f;
	Uword inst_code_lower4 = ir & mask_lower4;

	switch (inst_code_upper4)
	{
	case 0x00:
		/* NOP,HLT,JAL,JR */// 下位4bit目を取り出す
		if(ir == 0x0a) Decode_Return = JAL;
		else if(ir == 0x0b) Decode_Return = JR;
		else if((ir & 0x08) == 0x00) Decode_Return = NOP;
		else if((ir & 0x08) == 0x08) Decode_Return = HLT;
		break;

	case 0x10:
		/* OUT,IN */
		// 下位4bit目を取り出す
		if((inst_code_lower4 & 0x08) == 0) Decode_Return = OUT;
		else if((inst_code_lower4 & 0x08) == 8) Decode_Return = IN;
		break;

	case 0x20:
		/* RCF,SCF */
		// 下位4bit目を取り出す
		if((inst_code_lower4 & 0x08) == 0) Decode_Return = RCF;
		else if((inst_code_lower4 & 0x08) == 8) Decode_Return = SCF;
		break;

	case 0x30:
		/* Bbc */
		Decode_Return = Bbc;
		BranchCondition = inst_code_lower4 & 0x15;
		break;

	case 0x40:
		/* Ssm,Rsm */
		/* 下位3bit目を取り出す */
		if((inst_code_lower4 & 0x04) == 0) Decode_Return = Ssm;
		else if((inst_code_lower4 & 0x04) == 4) Decode_Return = Rsm;
		A_Instruction = inst_code_lower4 & 0x08;
		ShiftMode = inst_code_lower4 & 0x03;
		break;

	case 0x60:
		/* LD */
		Decode_Return = LD;
		A_Instruction = inst_code_lower4 & 0x08;
		B_Instruction = inst_code_lower4 & 0x07;
		break;

	case 0x70:
		/* ST */
		Decode_Return = ST;
		A_Instruction = inst_code_lower4 & 0x08;
		B_Instruction = inst_code_lower4 & 0x07;
		break;

	case 0x80:
		/* SBC */
		Decode_Return = SBC;
		A_Instruction = inst_code_lower4 & 0x08;
		B_Instruction = inst_code_lower4 & 0x07;
		break;

	case 0x90:
		/* ADC */
		Decode_Return = ADC;
		A_Instruction = inst_code_lower4 & 0x08;
		B_Instruction = inst_code_lower4 & 0x07;
		break;

	case 0xa0:
		/* SUB */
		Decode_Return = SUB;
		A_Instruction = inst_code_lower4 & 0x08;
		B_Instruction = inst_code_lower4 & 0x07;
		break;

	case 0xb0:
		/* ADD */
		Decode_Return = ADD;
		A_Instruction = inst_code_lower4 & 0x08;
		B_Instruction = inst_code_lower4 & 0x07;
		break;

	case 0xc0:
		/*EOR */
		Decode_Return = EOR;
		A_Instruction = inst_code_lower4 & 0x08;
		B_Instruction = inst_code_lower4 & 0x07;
		break;

	case 0xd0:
		/* OR */
		Decode_Return = OR;
		A_Instruction = inst_code_lower4 & 0x08;
		B_Instruction = inst_code_lower4 & 0x07;
		break;

	case 0xe0:
		/* AND */
		Decode_Return = AND;
		A_Instruction = inst_code_lower4 & 0x08;
		B_Instruction = inst_code_lower4 & 0x07;
		break;

	case 0xf0:
		/* CMP */
		Decode_Return = CMP;
		A_Instruction = inst_code_lower4 & 0x08;
		B_Instruction = inst_code_lower4 & 0x07;
		break;

	default:
		/* エラー処理 */
		fprintf(stderr, "Error: Instruction code\n");
		Decode_Return = RUN_HALT;
		break;
	}
	return Decode_Return;
}

Uword Set_A_Value(Cpub *cpub){
	Uword Withdraw_value;
	if(A_Instruction == 0){
		Withdraw_value = cpub->acc;
	}else{
		Withdraw_value = cpub->ix;
	}
	return Withdraw_value;
}

void Write_A_Value(Cpub *cpub, Uword ALU_result){
	if(A_Instruction == 0){
		cpub->acc = ALU_result;
	}else{
		cpub->ix = ALU_result;
	}
}

void Set_Flags(Cpub *cpub, Bit Carry, Bit Over, Uword ALU_result){
	Bit Negative_Flag = ALU_result & 0x80;
	Bit Zero_Flag;
	if(ALU_result == 0) Zero_Flag = 0x01;
	else Zero_Flag =0x00;

	cpub->cf = Carry;
	cpub->vf = Over;
	cpub->nf = Negative_Flag >> 7;
	cpub->zf = Zero_Flag;
}