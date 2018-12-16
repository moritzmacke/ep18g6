#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>


#define __STDC_LIMIT_MACROS 1

#include <inttypes.h>

#define FALSE 0
#define TRUE 1

#define hashmult 13493690561280548289ULL

enum { RVAL_END=256, RVAL_ARRAY, RVAL_OF, RVAL_INT, RVAL_RETURN, 
       RVAL_IF, RVAL_THEN, RVAL_ELSE, RVAL_WHILE, RVAL_DO, 
       RVAL_VAR, RVAL_NOT, RVAL_OR, RVAL_ASSIGNOP };


//29 Equivalance classes
enum { NUL = 0, DGT /* [0-9] */, LTR /* [G-Zgjkmpqxz] */, COL /* : */, 
       MIN /* - */, CSH /* $ */, SPC /*[\t ]*/, LBK /* \n */, LEX /* [;(),<#[]+*] */, 
       EQU /* = */, HEX /* [A-Fb] */, ANY, LCA, LCC, LCD, LCE, LCF, LCH, 
       LCI, LCL, LCN, LCO, LCR, LCS, LCT, LCU, LCV, LCW, LCY };
  
static const uint8_t eqClass[] = { 
    NUL, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, SPC, LBK, ANY, ANY, LBK, ANY, ANY, 
		ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, 
		SPC, ANY, ANY, LEX, CSH, ANY, ANY, ANY, LEX, LEX, LEX, LEX, LEX, MIN, ANY, ANY, 
		DGT, DGT, DGT, DGT, DGT, DGT, DGT, DGT, DGT, DGT, COL, LEX, LEX, EQU, ANY, ANY, 
		ANY, HEX, HEX, HEX, HEX, HEX, HEX, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, 
		LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LEX, ANY, LEX, ANY, ANY, 
		ANY, LCA, HEX, LCC, LCD, LCE, LCF, LTR, LCH, LCI, LTR, LTR, LCL, LTR, LCN, LCO, 
		LTR, LTR, LCR, LCS, LCT, LCU, LCV, LCW, LTR, LCY, LTR, ANY, ANY, ANY, ANY, ANY, 
		ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, 
		ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, 
		ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, 
		ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, 
		ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, 
		ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, 
		ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, 
		ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY
	};
	

enum State { S_STRT =  0, 
    S_0WSP, /* any -> -S_0WSP */
	  S_0LEX, /*any -> -S_0LEX*/
	  S_0ASS, /*'=' -> S_1ASS; any -> -S_0LEX */ 
	  S_1ASS, /* */
	  S_0CMT, /*'-' -> S_1CMT; any -> -S_0LEX */ 
	  S_1CMT, /*'\n' -> -S_1CMT; any -> S_1CMT */
	  S_0DEC, /* */
	  S_0HEX, /* */ 
	  S_1HEX, /* */
	  S_0IDT, /* */
    
    S_0END, /* e_ */
    S_1END, /* en_ */
    S_2END, /* end_ */
    S_1ELS, /* el_ */
    S_2ELS, /* els_ */
    S_3ELS, /* else */
    S_0ARR, /* a_ */
    S_1ARR, /* ar_ */
    S_2ARR, /* arr_ */
    S_3ARR, /* arra_ */
    S_4ARR, /* array_ */
    S_0OF_, /* o_ */
    S_1OF_, /* of_ */
    S_1OR_, /* or_ */
    S_0INT, /* i_ */
    S_1INT, /* in_ */
    S_2INT, /* int_ */
    S_1IF_, /* if_ */
    S_0RET, /* r_ */
    S_1RET, /* re_ */
    S_2RET, /* ret_ */
    S_3RET, /* retu_ */
    S_4RET, /* retur_ */
    S_5RET, /* return_ */
    S_0THN, /* t_ */
    S_1THN, /* th_ */
    S_2THN, /* the_ */
    S_3THN, /* then_ */
    S_0WHL, /* w_ */
    S_1WHL, /* wh_ */
    S_2WHL, /* whi_ */
    S_3WHL, /* whil_ */
    S_4WHL, /* while_ */
    S_0DO_, /* d_ */
    S_1DO_, /* do_ */
    S_0VAR, /* v_ */
    S_1VAR, /* va_ */
    S_2VAR, /* var_ */
    S_0NOT, /* n_ */
    S_1NOT, /* no_ */
    S_2NOT, /* not_ */
	  S_FAIL = -128
  };    
	
static const int8_t transitionTable[][29] = {
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*O_STRT*/  { S_STRT,  S_0DEC,  S_0IDT,  S_0ASS,  S_0CMT,  S_0HEX,  S_0WSP,  S_0WSP,  S_0LEX,  S_FAIL,  S_0IDT,  S_FAIL, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0ARR,  S_0IDT,  S_0DO_,  S_0END,  S_0IDT,  S_0IDT,  S_0INT,  S_0IDT,  S_0NOT,  S_0OF_,  S_0RET,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0THN,  S_0IDT,  S_0VAR,  S_0WHL,  S_0IDT },
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX
/*S_0WSP*/	{-S_0WSP, -S_0WSP, -S_0WSP, -S_0WSP, -S_0WSP, -S_0WSP, -S_0WSP, -S_0WSP, -S_0WSP, -S_0WSP, -S_0WSP, -S_0WSP, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
             -S_0WSP, -S_0WSP, -S_0WSP, -S_0WSP, -S_0WSP, -S_0WSP, -S_0WSP, -S_0WSP, -S_0WSP, -S_0WSP, -S_0WSP, -S_0WSP, 
            //LCT,     LCU,     LCV,     LCW,     LCY
             -S_0WSP, -S_0WSP, -S_0WSP, -S_0WSP, -S_0WSP },

            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX
/*S_0LEX*/	{-S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
             -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, 
            //LCT,     LCU,     LCV,     LCW,     LCY
             -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX
/*S_0ASS*/	{-S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX,  S_1ASS, -S_0LEX, -S_0LEX, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
             -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, 
            //LCT,     LCU,     LCV,     LCW,     LCY
             -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX },

            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX
/*S_1ASS*/	{-S_1ASS, -S_1ASS, -S_1ASS, -S_1ASS, -S_1ASS, -S_1ASS, -S_1ASS, -S_1ASS, -S_1ASS, -S_1ASS, -S_1ASS, -S_1ASS, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
             -S_1ASS, -S_1ASS, -S_1ASS, -S_1ASS, -S_1ASS, -S_1ASS, -S_1ASS, -S_1ASS, -S_1ASS, -S_1ASS, -S_1ASS, -S_1ASS, 
            //LCT,     LCU,     LCV,     LCW,     LCY
             -S_1ASS, -S_1ASS, -S_1ASS, -S_1ASS, -S_1ASS },

            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX
/*S_0CMT*/	{-S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX,  S_1CMT, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
             -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, 
            //LCT,     LCU,     LCV,     LCW,     LCY
             -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX },

            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX
/*S_1CMT*/	{ S_1CMT,  S_1CMT,  S_1CMT,  S_1CMT,  S_1CMT,  S_1CMT,  S_1CMT, -S_1CMT,  S_1CMT,  S_1CMT,  S_1CMT,  S_1CMT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_1CMT,  S_1CMT,  S_1CMT,  S_1CMT,  S_1CMT,  S_1CMT,  S_1CMT,  S_1CMT,  S_1CMT,  S_1CMT,  S_1CMT,  S_1CMT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_1CMT,  S_1CMT,  S_1CMT,  S_1CMT,  S_1CMT },

            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX
/*S_0DEC*/	{-S_0DEC,  S_0DEC, -S_0DEC, -S_0DEC, -S_0DEC, -S_0DEC, -S_0DEC, -S_0DEC, -S_0DEC, -S_0DEC, -S_0DEC, -S_0DEC, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
             -S_0DEC, -S_0DEC, -S_0DEC, -S_0DEC, -S_0DEC, -S_0DEC, -S_0DEC, -S_0DEC, -S_0DEC, -S_0DEC, -S_0DEC, -S_0DEC, 
            //LCT,     LCU,     LCV,     LCW,     LCY
             -S_0DEC, -S_0DEC, -S_0DEC, -S_0DEC, -S_0DEC },

            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_0HEX*/	{ S_FAIL,  S_1HEX,  S_FAIL,  S_FAIL,  S_FAIL,  S_FAIL,  S_FAIL,  S_FAIL,  S_FAIL,  S_FAIL,  S_1HEX,  S_FAIL, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_1HEX,  S_1HEX,  S_1HEX,  S_1HEX,  S_1HEX,  S_FAIL,  S_FAIL,  S_FAIL,  S_FAIL,  S_FAIL,  S_FAIL,  S_FAIL, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_FAIL,  S_FAIL,  S_FAIL,  S_FAIL,  S_FAIL },

            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_1HEX*/	{-S_1HEX,  S_1HEX, -S_1HEX, -S_1HEX, -S_1HEX, -S_1HEX, -S_1HEX, -S_1HEX, -S_1HEX, -S_1HEX,  S_1HEX, -S_1HEX, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_1HEX,  S_1HEX,  S_1HEX,  S_1HEX,  S_1HEX, -S_1HEX, -S_1HEX, -S_1HEX, -S_1HEX, -S_1HEX, -S_1HEX, -S_1HEX, 
            //LCT,     LCU,     LCV,     LCW,     LCY
             -S_1HEX, -S_1HEX, -S_1HEX, -S_1HEX, -S_1HEX },

            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_0IDT*/	{-S_0IDT,  S_0IDT,  S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT,  S_0IDT, -S_0IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT },

            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_0END*/	{-S_0IDT,  S_0IDT,  S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT,  S_0IDT, -S_0IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_1ELS,  S_1END,  S_0IDT,  S_0IDT,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT },
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_1END*/	{-S_0IDT,  S_0IDT,  S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT,  S_0IDT, -S_0IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_2END,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT },
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_2END*/	{-S_2END,  S_0IDT,  S_0IDT, -S_2END, -S_2END, -S_2END, -S_2END, -S_2END, -S_2END, -S_2END,  S_0IDT, -S_0IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT },
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_1ELS*/	{-S_0IDT,  S_0IDT,  S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT,  S_0IDT, -S_0IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_2ELS, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT },
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_2ELS*/	{-S_0IDT,  S_0IDT,  S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT,  S_0IDT, -S_0IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_3ELS,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT },
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_3ELS*/	{-S_3ELS,  S_0IDT,  S_0IDT, -S_3ELS, -S_3ELS, -S_3ELS, -S_3ELS, -S_3ELS, -S_3ELS, -S_3ELS,  S_0IDT, -S_3ELS, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT },
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_0ARR*/	{-S_0IDT,  S_0IDT,  S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT,  S_0IDT, -S_0IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_1ARR,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT },              

            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_1ARR*/	{-S_0IDT,  S_0IDT,  S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT,  S_0IDT, -S_0IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_2ARR,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT },    
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_2ARR*/	{-S_0IDT,  S_0IDT,  S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT,  S_0IDT, -S_0IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_3ARR,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT },           

            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_3ARR*/	{-S_0IDT,  S_0IDT,  S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT,  S_0IDT, -S_0IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_4ARR }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_4ARR*/	{-S_4ARR,  S_0IDT,  S_0IDT, -S_4ARR, -S_4ARR, -S_4ARR, -S_4ARR, -S_4ARR, -S_4ARR, -S_4ARR,  S_0IDT, -S_4ARR, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_0OF_*/	{-S_0IDT,  S_0IDT,  S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT,  S_0IDT, -S_0IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_1OF_,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_1OR_,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_1OF_*/	{-S_1OF_,  S_0IDT,  S_0IDT, -S_1OF_, -S_1OF_, -S_1OF_, -S_1OF_, -S_1OF_, -S_1OF_, -S_1OF_,  S_0IDT, -S_1OF_, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_1OR_*/	{-S_1OR_,  S_0IDT,  S_0IDT, -S_1OR_, -S_1OR_, -S_1OR_, -S_1OR_, -S_1OR_, -S_1OR_, -S_0IDT,  S_0IDT, -S_1OR_, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_0INT*/	{-S_0IDT,  S_0IDT,  S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT,  S_0IDT, -S_0IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_1IF_,  S_0IDT,  S_0IDT,  S_0IDT,  S_1INT,  S_0IDT,  S_0IDT,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_1INT*/	{-S_0IDT,  S_0IDT,  S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT,  S_0IDT, -S_0IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_2INT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_2INT*/	{-S_2INT,  S_0IDT,  S_0IDT, -S_2INT, -S_2INT, -S_2INT, -S_2INT, -S_2INT, -S_2INT, -S_2INT,  S_0IDT, -S_2INT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_1IF_*/	{-S_1IF_,  S_0IDT,  S_0IDT, -S_1IF_, -S_1IF_, -S_1IF_, -S_1IF_, -S_1IF_, -S_1IF_, -S_1IF_,  S_0IDT, -S_1IF_, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_0RET*/	{-S_0IDT,  S_0IDT,  S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT,  S_0IDT, -S_0IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_1RET,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_1RET*/	{-S_0IDT,  S_0IDT,  S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT,  S_0IDT, -S_0IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_2RET,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_2RET*/	{-S_0IDT,  S_0IDT,  S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT,  S_0IDT, -S_0IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_3RET,  S_0IDT,  S_0IDT,  S_0IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_3RET*/	{-S_0IDT,  S_0IDT,  S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT,  S_0IDT, -S_0IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_4RET,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_4RET*/	{-S_0IDT,  S_0IDT,  S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT,  S_0IDT, -S_0IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_5RET,  S_0IDT,  S_0IDT,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_5RET*/	{-S_5RET,  S_0IDT,  S_0IDT, -S_5RET, -S_5RET, -S_5RET, -S_5RET, -S_5RET, -S_5RET, -S_5RET,  S_0IDT, -S_5RET, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_0THN*/	{-S_0IDT,  S_0IDT,  S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT,  S_0IDT, -S_0IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_1THN,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_1THN*/	{-S_0IDT,  S_0IDT,  S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT,  S_0IDT, -S_0IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_2THN,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_2THN*/	{-S_0IDT,  S_0IDT,  S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT,  S_0IDT, -S_0IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_3THN,  S_0IDT,  S_0IDT,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_3THN*/	{-S_3THN,  S_0IDT,  S_0IDT, -S_3THN, -S_3THN, -S_3THN, -S_3THN, -S_3THN, -S_3THN, -S_3THN,  S_0IDT, -S_3THN, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_0WHL*/	{-S_0IDT,  S_0IDT,  S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT,  S_0IDT, -S_0IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_1WHL,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_1WHL*/	{-S_0IDT,  S_0IDT,  S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT,  S_0IDT, -S_0IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_2WHL,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_2WHL*/	{-S_0IDT,  S_0IDT,  S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT,  S_0IDT, -S_0IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_3WHL,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_3WHL*/	{-S_0IDT,  S_0IDT,  S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT,  S_0IDT, -S_0IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_4WHL,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_4WHL*/	{-S_4WHL,  S_0IDT,  S_0IDT, -S_4WHL, -S_4WHL, -S_4WHL, -S_4WHL, -S_4WHL, -S_4WHL, -S_4WHL,  S_0IDT, -S_4WHL, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_0DO_*/	{-S_0IDT,  S_0IDT,  S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT,  S_0IDT, -S_0IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_1DO_,  S_0IDT,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_1DO_*/	{-S_1DO_,  S_0IDT,  S_0IDT, -S_1DO_, -S_1DO_, -S_1DO_, -S_1DO_, -S_1DO_, -S_1DO_, -S_1DO_,  S_0IDT, -S_1DO_, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_0VAR*/	{-S_0IDT,  S_0IDT,  S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT,  S_0IDT, -S_0IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_1VAR,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_1VAR*/	{-S_0IDT,  S_0IDT,  S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT,  S_0IDT, -S_0IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_2VAR,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_2VAR*/	{-S_2VAR,  S_0IDT,  S_0IDT, -S_2VAR, -S_2VAR, -S_2VAR, -S_2VAR, -S_2VAR, -S_2VAR, -S_2VAR,  S_0IDT, -S_2VAR, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_0NOT*/	{-S_0IDT,  S_0IDT,  S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT,  S_0IDT, -S_0IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_1NOT,  S_0IDT,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_1NOT*/	{-S_0IDT,  S_0IDT,  S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT,  S_0IDT, -S_0IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_2NOT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_2NOT*/	{-S_2NOT,  S_0IDT,  S_0IDT, -S_2NOT, -S_2NOT, -S_2NOT, -S_2NOT, -S_2NOT, -S_2NOT, -S_2NOT,  S_0IDT, -S_2NOT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT,  S_0IDT }, 
              
              };
  
//c must be [0-9A-Fa-f]
static uint8_t hexValue(char c) {
  uint8_t v = c - 0x30;
  if(v > 9) {
    v = (v & 0xf) + 9;
  }
  return v;
}

/*
size_t countComment = 0;
size_t countInteger = 0;
size_t countHexnum = 0;
size_t countIdent = 0;
size_t countKeyword = 0;
size_t countAssign = 0;
size_t countLexem = 0;
size_t countWhitespace = 0;

size_t charsComment = 0;
size_t charsInteger = 0;
size_t charsHexnum = 0;
size_t charsIdent = 0;
size_t charsKeyword = 0;
*/

static int32_t lex(char *input) {
  
  char c, n;
  char *current = input;
	
  uint64_t hash = 0;
  
//	char *output = ""; //debug
  uint64_t rValue = 0;
  uint64_t lastChars = 0;
  
  
  while(1) {
  
    int state = S_STRT;
    char *token = current;
    char *t;
    
    do {
      c = *current;
      uint8_t l = eqClass[c];
      state = transitionTable[state][l];
      if(state <= 0) break;
      current++;
    } while(1);
    
    
    switch(-state) {
    case S_STRT: //end of bufffer
      printf("%lx\n", hash);
      return 1;
    case S_0WSP:
//      countWhitespace++;
//      output = "WHITESPACE";
      continue;
//				break;
    case S_1CMT:
//      charsComment += current - token;
//      countComment++;
//      output = "COMMENT";
      break;
    case S_0LEX:
//      countLexem++;
//      output = "LEXEM";
      hash = (hash+*token)*hashmult;
      break;
    case S_1ASS:
//      countAssign++;
//      output = "ASSIGN";
      hash = (hash+RVAL_ASSIGNOP)*hashmult;
      break;
    case S_0DEC:
//      countInteger++;
//      charsInteger += current - token;
//      output = "DECIMAL";
      t = token;
      rValue = *t++ - 0x30;
      while(t < current) {
        rValue = (rValue*10) + *t++ - 0x30;
      }
      hash = (hash+((int)(rValue) ^ 0x8000))*hashmult;
      break;
    case S_1HEX:
//      countHexnum++;
//      charsHexnum += current - token;
//      output = "HEXNUM";
      t = &token[1];
      rValue = hexValue(*t++);
      while(t < current) {
        rValue = (rValue*16) + hexValue(*t++);
      }
      hash = (hash+((int)(rValue) ^ 0x4000))*hashmult;
      break;
      
    case S_2END: /* end */
//      charsKeyword += current - token;
//      countKeyword++;
//      output = "KW_END";
      hash = (hash+RVAL_END)*hashmult;
      break;
    case S_3ELS: /* else */
//      charsKeyword += current - token;
//      countKeyword++;
//      output = "KW_ELSE";
      hash = (hash+RVAL_ELSE)*hashmult;
      break;
    case S_4ARR: /* array */
//      charsKeyword += current - token;
//      countKeyword++;
//      output = "KW_ARRAY";
      hash = (hash+RVAL_ARRAY)*hashmult;
      break;
    case S_1OF_: /* of */
//      charsKeyword += current - token;
//      countKeyword++;
//      output = "KW_OF";
      hash = (hash+RVAL_OF)*hashmult;
      break;
    case S_1OR_: /* or */
//      countKeyword++;
//      charsKeyword += current - token;
//      output = "KW_OR";
      hash = (hash+RVAL_OR)*hashmult;
      break;
    case S_2INT: /* int */
//      charsKeyword += current - token;
//      countKeyword++;
//      output = "KW_INT";
      hash = (hash+RVAL_INT)*hashmult;
      break;
    case S_1IF_: /* if */
//      charsKeyword += current - token;
//      countKeyword++;
//      output = "KW_IF";
      hash = (hash+RVAL_IF)*hashmult;
      break;
    case S_5RET: /* return */
//      charsKeyword += current - token;
//      countKeyword++;
//      output = "KW_RETURN";
      hash = (hash+RVAL_RETURN)*hashmult;
      break;
    case S_3THN: /* then */
//      charsKeyword += current - token;
//      countKeyword++;
//      output = "KW_THEN";
      hash = (hash+RVAL_THEN)*hashmult;
      break;
    case S_4WHL: /* while */
//      charsKeyword += current - token;
//      countKeyword++;
//      output = "KW_WHILE";
      hash = (hash+RVAL_WHILE)*hashmult;
      break;
    case S_1DO_: /* do */
//      charsKeyword += current - token;
//      countKeyword++;
//      output = "KW_DO";
      hash = (hash+RVAL_DO)*hashmult;
      break;
    case S_2VAR: /* var */
//      charsKeyword += current - token;
//      countKeyword++;
//      output = "KW_VAR";
      hash = (hash+RVAL_VAR)*hashmult;
      break;
    case S_2NOT: /* not */
//      charsKeyword += current - token;
//      countKeyword++;
//      output = "KW_NOT";
      hash = (hash+RVAL_NOT)*hashmult;
      break;
      
    case S_0IDT:
//      charsIdent += current - token;
//      countIdent++;
//      output = "IDENT";
      t = token;
      rValue = 0;
      while(t < current) {
        rValue = (rValue+*t)*hashmult;
        t++;
      }
      hash = (hash+((int)rValue))*hashmult;
      break;
      
    case -S_FAIL:
      fprintf(stderr, "unrecognized: \n");
      return 0;
    default:
      fprintf(stderr, "invalid accept state");
      return 0;
    }
    
//    *current = '\0';
//    printf("%s: %s %llx\n", output, token, hash);
//    *current = c;
    
  }
  

    
	return 1; 
}

int main(int argc, char *argv[])
{

  size_t inputSize = 0;
  char *input = NULL;

  if (argc != 2) {
    fprintf(stderr,"Usage: %s <file>\n",argv[0]);
    exit(1);
  }
  
  FILE *in = fopen(argv[1],"rb");
  if (in == NULL) {
    fprintf(stderr, "Cannot open %s\n", argv[1]);
    exit(1);
  }
  
  fseek(in, 0, SEEK_END);
  inputSize = ftello(in);
  fseek(in, 0, SEEK_SET);
  
//  inputSize = 10000;
  
  input = malloc(inputSize+1);
  
  size_t read = fread(input, 1, inputSize, in);
  
  input[inputSize] = 0;
  
  fclose(in);

//  printf("%lu bytes read\n", read);
  
  lex(input);
  
  /*
  printf("COMMENT:   \t %10lu\t %10lu bytes\n", countComment, charsComment);
  printf("INTEGER:   \t %10lu\t %10lu bytes\n", countInteger, charsInteger);
  printf("HEXNUM:    \t %10lu\t %10lu bytes\n", countHexnum, charsHexnum);
  printf("IDENT:     \t %10lu\t %10lu bytes\n", countIdent, charsIdent);
  printf("KEYWORD:   \t %10lu\t %10lu bytes\n", countKeyword, charsKeyword);
  printf("ASSIGN:    \t %10lu\t %10lu bytes\n", countAssign, countAssign*2);
  printf("LEXEM:     \t %10lu\t %10lu bytes\n", countLexem, countLexem);
  printf("WHITESPACE:\t %10lu\t %10lu bytes\n", countWhitespace, countWhitespace);
  */
  
  return 0;
}
