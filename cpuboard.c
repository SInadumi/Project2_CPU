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
Uword sm;				/* Shift mode */
Uword bc;				/* Branch Condition */

int ST_Instruction(Cpub *);												/* ST命令 */
int ADD_Instruction(Cpub *);											/* ADD命令 */
int AND_Instruction(Cpub *);											/* AND命令 */
int OR_Instruction(Cpub *);												/* OR命令 */
int EOR_Instruction(Cpub *);											/* EOR命令 */
Uword instruction_decoding(Uword ir);									/* 命令解読機構 */
Uword Set_A_Value(char *Instruction_name, Cpub *);						/* A値のセット */
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
			break;
		case JR:
			fprintf(stderr, "execute JR\n");
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
			break;
		// case Ssm:
		// 	fprintf(stderr, "execute Ssm\n");
		// 	break;
		// case Rsm:
		// 	fprintf(stderr, "execute Rsm\n");
		// 	break;
		case LD:
			fprintf(stderr, "execute LD\n");
			break;
		case ST:
			fprintf(stderr, "execute ST\n");
			RUN_Return = ST_Instruction(cpub);
			if(RUN_Return == RUN_HALT) fprintf(stderr, "Failed: ST Instruction\n");
			break;
		case SBC:
			fprintf(stderr, "execute SBC\n");
			break;
		case ADC:
			fprintf(stderr, "execute ADC\n");
			break;
		case SUB:
			fprintf(stderr, "execute SUB\n");
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
			break;
		default:
			/* エラー処理 */
			fprintf(stderr, "Error\n");
			RUN_Return = RUN_HALT;
			break;
	}
	return RUN_Return;
}

int ST_Instruction(Cpub *cpub){	
	Uword Second_word;
	Uword A_Value = Set_A_Value("ST", cpub);		/* 1 -> IX, 0 -> ACC */

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
		fprintf(stderr,"Error: OP_B was not defined in ST Instruction\n");
		return RUN_HALT;
		break;
	}
	return RUN_STEP;
}

int ADD_Instruction(Cpub *cpub){
	Uword Second_word;
	Uword A_Value = Set_A_Value("ADD", cpub);
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
			fprintf(stderr,"Error: OP_B was not defined in ADD Instruction\n");
			return RUN_HALT;
			break;
	}

	OF = (A & B & (~C)) | ((~A) & (~B) & C);
	Write_A_Value(cpub, ALU_result);
	Set_Flags(cpub, cpub->cf, OF, ALU_result);
	return RUN_STEP;
}

int AND_Instruction(Cpub *cpub){
	Uword Second_word;
	Uword A_Value = Set_A_Value("AND", cpub);
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
			fprintf(stderr,"Error: OP_B was not defined in AND Instruction\n");
			return RUN_HALT;
			break;
	}
	Write_A_Value(cpub, ALU_result);
	Set_Flags(cpub, cpub->cf, 0, ALU_result);
	return RUN_STEP;
}

int OR_Instruction(Cpub *cpub){
	Uword Second_word;
	Uword A_Value = Set_A_Value("OR", cpub);
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
			fprintf(stderr,"Error: OP_B was not defined in OR Instruction\n");
			return RUN_HALT;
			break;
	}
	Write_A_Value(cpub, ALU_result);
	Set_Flags(cpub, cpub->cf, 0, ALU_result);
	return RUN_STEP;
}

int EOR_Instruction(Cpub *cpub){
	Uword Second_word;
	Uword A_Value = Set_A_Value("EOR", cpub);
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
			fprintf(stderr,"Error: OP_B was not defined in EOR Instruction\n");
			return RUN_HALT;
			break;
	}
	Write_A_Value(cpub, ALU_result);
	Set_Flags(cpub, cpub->cf, 0, ALU_result);
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
		/* NOP,HLT,JAL,JR */
		if(ir == JAL){
			Decode_Return = JAL;
		}
		else if(ir == JR) Decode_Return = JR;
		
		// 下位4bit目を取り出す
		if((ir & 0x08) == 0) Decode_Return = NOP;
		else if((ir & 0x08) == 8) Decode_Return = HLT;
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
		bc = inst_code_lower4 & 0x15;
		break;

	case 0x40:
		// /* Ssm,Rsm */
		// //下位3bit目を取り出す
		// if((inst_code_lower4 & 0x04) == 0) Decode_Return = Ssm;
		// else if((inst_code_lower4 & 0x04) == 4) Decode_Return = Rsm;
		// break;

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

Uword Set_A_Value(char *Instruction_name, Cpub *cpub){
	Uword Withdraw_value;
	if(strcmp(Instruction_name,"ST") == 0 && A_Instruction == 1){
		fprintf(stderr, "Warning: ST isn't allowed setting 'Instruction_A = 0'");
		A_Instruction = 0;
	}
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
	Bit Negative_Flag = ALU_result & 0x64;
	Bit Zero_Flag;
	if(ALU_result == 0) Zero_Flag = 0x01;
	else Zero_Flag =0x00;

	cpub->cf = Carry;
	cpub->vf = Over;
	cpub->nf = Negative_Flag >> 6;
	cpub->zf = Zero_Flag;
}