/*
 *	Project-based Learning II (CPU)
 *
 *	Program:	instruction set simulator of the Educational CPU Board
 *	File Name:	cpuboard.c
 *	Descrioption:	simulation(emulation) of an instruction
 */
#include	<stdio.h>
#include	"cpuboard.h"

/* 各種変数・関数の定義 */
static Uword instruction_decoding(Uword ir);

/* 命令語コード定義(1語目:16bit) */
#define NOP 0x00
#define HLT 0x0f
#define OUT 0x10
#define IN 	0x1f
#define RCF 0x20
#define SCF 0x2f
#define LD	0x60
#define ST	0x70
#define ADD	0xb0
#define ADC	0x90
#define SUB	0xa0
#define SBC	0x80
#define CMP	0xf0
#define AND	0xe0
#define OR	0xd0
#define EOR	0xc0
#define Ssm	0x40
#define Rsm	0x40+0x04
#define Bbc	0x30
#define JAL	0x0a
#define JR	0x0b

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
	//fprintf(stderr, "cf=%hhu, vf=%hhu, nf=%hhu, zf=%hhu \n", cpub->cf, cpub->vf, cpub->nf,cpub->zf);

	/* P0 Phase */
	cpub->mar = cpub->pc;
	cpub->pc++;

	/* P1 Phase */
	cpub->ir = (Uword)cpub->mem[0x000 + cpub->mar];	//Program領域へのアクセス
	
	/* 命令語の解析 */
	Uword ir = instruction_decoding(cpub->ir);

	/* 分岐処理 */
	switch(ir)
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
			break;
		case IN:
			fprintf(stderr, "execute IN\n");
			break;
		case RCF:
			fprintf(stderr, "execute RCF\n");
			break;
		case SCF:
			fprintf(stderr, "execute SCF\n");
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
			break;
		case EOR:
			fprintf(stderr, "execute EOR\n");
			break;
		case OR:
			fprintf(stderr, "execute OR\n");
			break;
		case AND:
			fprintf(stderr, "execute AND\n");
			break;
		case CMP:
			fprintf(stderr, "execute CMP\n");
			break;
		default:
			/* エラー処理 */
			fprintf(stderr, "error\n");
			RUN_Return = RUN_HALT;
			break;
	}
	return RUN_Return;
}

/* 命令解読を行う関数 */
static Uword instruction_decoding(Uword ir){
	Uword Decode_Return = NOP;
	/* 上位4bitの取り出し */
	Uword mask_upper4 = 0xf0;
	Uword inst_code_upper4 = ir & mask_upper4;
	/* 下位4bitの取り出し */
	Uword mask_lower4 = 0x0f;
	Uword inst_code_lower4 = ir & mask_lower4;

	fprintf(stderr, "upper:%x, lower:%x",inst_code_upper4,inst_code_lower4);

	switch (inst_code_upper4)
	{
	case 0x00:
		/* NOP,HLT,JAL,JR */
		if(ir == JAL) Decode_Return = JAL;
		else if(ir == JR) Decode_Return = JR;
		
		// 下位4bit目を取り出す
		if((ir & 0x08) == 0) Decode_Return = NOP;
		else if((ir & 0x08) == 1) Decode_Return = HLT;
		break;

	case 0x10:
		/* OUT,IN */
		// 下位4bit目を取り出す
		if((inst_code_lower4 & 0x08) == 0) Decode_Return = OUT;
		else if((inst_code_lower4 & 0x08) == 1) Decode_Return = IN;
		break;

	case 0x20:
		/* RCF,SCF */
		// 下位4bit目を取り出す
		if((inst_code_lower4 & 0x08) == 0) Decode_Return = RCF;
		else if((inst_code_lower4 & 0x08) == 1) Decode_Return = SCF;
		break;

	case 0x30:
		/* Bbc */
		Decode_Return = Bbc;
		break;

	// case 0x40:
	// 	/* Ssm,Rsm */
	// 	//下位3bit目を取り出す
	// 	if((inst_code_lower4 & 0x4) == 0) Decode_Return = Ssm;
	// 	else if((inst_code_lower4 & 0x4) == 1) Decode_Return = Rsm;
	// 	break;

	case 0x60:
		/* LD */
		Decode_Return = LD;
		break;

	case 0x70:
		/* ST */
		Decode_Return = ST;
		break;

	case 0x80:
		/* SBC */
		Decode_Return = SBC;
		break;

	case 0x90:
		/* ADC */
		Decode_Return = ADC;
		break;

	case 0xa0:
		/* SUB */
		Decode_Return = SUB;
		break;

	case 0xb0:
		/* ADD */
		Decode_Return = ADD;
		break;

	case 0xc0:
		/*EOR */
		Decode_Return = EOR;
		break;

	case 0xd0:
		/* OR */
		Decode_Return = OR;
		break;

	case 0xe0:
		/* AND */
		Decode_Return = AND;
		break;

	case 0xf0:
		/* CMP */
		Decode_Return = CMP;
		break;

	default:
		/* エラー処理 */
		fprintf(stderr, "error inst\n");
		Decode_Return = RUN_HALT;
		break;
	}
	return Decode_Return;
}