/*
 *	Project-based Learning II (CPU)
 *
 *	Program:	instruction set simulator of the Educational CPU Board
 *	File Name:	cpuboard.h
 *	Descrioption:	resource definition of the educational computer board
 */

/*=============================================================================
 *   Architectural Data Types
 *===========================================================================*/
typedef signed char	Sword;
typedef unsigned char	Uword;
typedef unsigned short	Addr;
typedef unsigned char	Bit;


/*=============================================================================
 *   CPU Board Resources
 *===========================================================================*/
#define	MEMORY_SIZE	256*2
#define	IMEMORY_SIZE	256

typedef struct iobuf {
	Bit	flag;
	Uword	buf;
} IOBuf;

typedef struct cpuboard {
	Uword	pc;
	Uword	acc;
	Uword	ix;
	Uword	mar;	//追加
	Uword	ir;		//追加
	Bit	cf, vf, nf, zf;
	IOBuf	*ibuf;
	IOBuf	obuf;
	/*
	 *   [ add here the other CPU resources if necessary ]
	 */
	Uword	mem[MEMORY_SIZE];	/* 0XX:Program, 1XX:Data */
} Cpub;


/*=============================================================================
 *   Top Function of an Instruction Simulation
 *===========================================================================*/
#define	RUN_HALT	0
#define	RUN_STEP	1
int	step(Cpub *);

/*=============================================================================
 *   命令語コード定義(1語目:16bit)
 *===========================================================================*/
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
 *   Shift Mode (2bit)
 *===========================================================================*/
#define RA 0x00
#define LA 0x01
#define RL 0x02
#define LL 0x03

/*=============================================================================
 *   命令コード1語目のB(3bit)
 *===========================================================================*/
#define ACC 0x00			/* アキュムレータ */
#define IX 0x01				/* インデックスレジスタ */
#define Immediate_d 0x02	/* 即値アドレス */
#define Program_Absolute_d 0x04		/* 絶対アドレス(プログラム領域) */
#define Data_Absolute_d 0x05		/* 絶対アドレス(データ領域) */
#define Program_IX 0x06			/* IX修飾アドレス(プログラム領域) */
#define Data_IX 0x07		/* IX修飾アドレス(データ領域) */

/*=============================================================================
 *   Branch Condition(4bit)
 *===========================================================================*/
#define BC_A  0x00
#define BC_VF 0x08
#define BC_NZ 0x01
#define BC_Z  0x09
#define BC_ZP 0x02
#define BC_N  0x0a
#define BC_P  0x03
#define BC_ZN 0x0b
#define BC_NI 0x04
#define BC_NO 0x0c
#define BC_NC 0x05
#define BC_C  0x0d
#define BC_CE 0x06
#define BC_LT 0x0e
#define BC_GT 0x07
#define BC_LE 0x0f
