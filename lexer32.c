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
    SA_WSP, /* any -> -SA_WSP */
	  SA_LEX, /*any -> -SA_LEX*/
	  S0_ASS, /*'=' -> SA_ASS; any -> -SA_LEX */ 
	  SA_ASS, /* */
	  S0_CMT, /*'-' -> SA_CMT; any -> -SA_LEX */ 
	  SA_CMT, /*'\n' -> -SA_CMT; any -> SA_CMT */
	  SA_DEC, /* */
	  S0_HEX, /* */ 
	  SA_HEX, /* */
	  SA_IDT, /* */
    
    S0_END, /* e_ */
    S1_END, /* en_ */
    SA_END, /* end_ */
    S1_ELS, /* el_ */
    S2_ELS, /* els_ */
    SA_ELS, /* else */
    S0_ARR, /* a_ */
    S1_ARR, /* ar_ */
    S2_ARR, /* arr_ */
    S3_ARR, /* arra_ */
    SA_ARR, /* array_ */
    S0_OF_, /* o_ */
    SA_OF_, /* of_ */
    SA_OR_, /* or_ */
    S0_INT, /* i_ */
    S1_INT, /* in_ */
    SA_INT, /* int_ */
    SA_IF_, /* if_ */
    S0_RET, /* r_ */
    S1_RET, /* re_ */
    S2_RET, /* ret_ */
    S3_RET, /* retu_ */
    S4_RET, /* retur_ */
    SA_RET, /* return_ */
    S0_THN, /* t_ */
    S1_THN, /* th_ */
    S2_THN, /* the_ */
    SA_THN, /* then_ */
    S0_WHL, /* w_ */
    S1_WHL, /* wh_ */
    S2_WHL, /* whi_ */
    S3_WHL, /* whil_ */
    SA_WHL, /* while_ */
    S0_DO_, /* d_ */
    SA_DO_, /* do_ */
    S0_VAR, /* v_ */
    S1_VAR, /* va_ */
    SA_VAR, /* var_ */
    S0_NOT, /* n_ */
    S1_NOT, /* no_ */
    SA_NOT, /* not_ */
	  S_FAIL = -128
  };    
	
static const int8_t transitionTable[][29] = { 
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S_STRT*/  { S_STRT,  SA_DEC,  SA_IDT,  S0_ASS,  S0_CMT,  S0_HEX,  SA_WSP,  SA_WSP,  SA_LEX,  S_FAIL,  SA_IDT,  S_FAIL, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S0_ARR,  SA_IDT,  S0_DO_,  S0_END,  SA_IDT,  SA_IDT,  S0_INT,  SA_IDT,  S0_NOT,  S0_OF_,  S0_RET,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S0_THN,  SA_IDT,  S0_VAR,  S0_WHL,  SA_IDT },

            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX
/*SA_WSP*/	{-SA_WSP, -SA_WSP, -SA_WSP, -SA_WSP, -SA_WSP, -SA_WSP, -SA_WSP, -SA_WSP, -SA_WSP, -SA_WSP, -SA_WSP, -SA_WSP, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
             -SA_WSP, -SA_WSP, -SA_WSP, -SA_WSP, -SA_WSP, -SA_WSP, -SA_WSP, -SA_WSP, -SA_WSP, -SA_WSP, -SA_WSP, -SA_WSP, 
            //LCT,     LCU,     LCV,     LCW,     LCY
             -SA_WSP, -SA_WSP, -SA_WSP, -SA_WSP, -SA_WSP },

            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX
/*SA_LEX*/	{-SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
             -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, 
            //LCT,     LCU,     LCV,     LCW,     LCY
             -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX
/*S0_ASS*/	{-SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX,  SA_ASS, -SA_LEX, -SA_LEX, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
             -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, 
            //LCT,     LCU,     LCV,     LCW,     LCY
             -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX },

            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX
/*SA_ASS*/	{-SA_ASS, -SA_ASS, -SA_ASS, -SA_ASS, -SA_ASS, -SA_ASS, -SA_ASS, -SA_ASS, -SA_ASS, -SA_ASS, -SA_ASS, -SA_ASS, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
             -SA_ASS, -SA_ASS, -SA_ASS, -SA_ASS, -SA_ASS, -SA_ASS, -SA_ASS, -SA_ASS, -SA_ASS, -SA_ASS, -SA_ASS, -SA_ASS, 
            //LCT,     LCU,     LCV,     LCW,     LCY
             -SA_ASS, -SA_ASS, -SA_ASS, -SA_ASS, -SA_ASS },

            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX
/*S0_CMT*/	{-SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX,  SA_CMT, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
             -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, 
            //LCT,     LCU,     LCV,     LCW,     LCY
             -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX, -SA_LEX },

                                                                            //
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX
/*SA_CMT*/	{ SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT,  SA_WSP,  SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT },

            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX
/*SA_DEC*/	{-SA_DEC,  SA_DEC, -SA_DEC, -SA_DEC, -SA_DEC, -SA_DEC, -SA_DEC, -SA_DEC, -SA_DEC, -SA_DEC, -SA_DEC, -SA_DEC, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
             -SA_DEC, -SA_DEC, -SA_DEC, -SA_DEC, -SA_DEC, -SA_DEC, -SA_DEC, -SA_DEC, -SA_DEC, -SA_DEC, -SA_DEC, -SA_DEC, 
            //LCT,     LCU,     LCV,     LCW,     LCY
             -SA_DEC, -SA_DEC, -SA_DEC, -SA_DEC, -SA_DEC },

            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S0_HEX*/	{ S_FAIL,  SA_HEX,  S_FAIL,  S_FAIL,  S_FAIL,  S_FAIL,  S_FAIL,  S_FAIL,  S_FAIL,  S_FAIL,  SA_HEX,  S_FAIL, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_HEX,  SA_HEX,  SA_HEX,  SA_HEX,  SA_HEX,  S_FAIL,  S_FAIL,  S_FAIL,  S_FAIL,  S_FAIL,  S_FAIL,  S_FAIL, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S_FAIL,  S_FAIL,  S_FAIL,  S_FAIL,  S_FAIL },

            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*SA_HEX*/	{-SA_HEX,  SA_HEX, -SA_HEX, -SA_HEX, -SA_HEX, -SA_HEX, -SA_HEX, -SA_HEX, -SA_HEX, -SA_HEX,  SA_HEX, -SA_HEX, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_HEX,  SA_HEX,  SA_HEX,  SA_HEX,  SA_HEX, -SA_HEX, -SA_HEX, -SA_HEX, -SA_HEX, -SA_HEX, -SA_HEX, -SA_HEX, 
            //LCT,     LCU,     LCV,     LCW,     LCY
             -SA_HEX, -SA_HEX, -SA_HEX, -SA_HEX, -SA_HEX },

            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*SA_IDT*/	{-SA_IDT,  SA_IDT,  SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT,  SA_IDT, -SA_IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT },

            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S0_END*/	{-SA_IDT,  SA_IDT,  SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT,  SA_IDT, -SA_IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  S1_ELS,  S1_END,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT },
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S1_END*/	{-SA_IDT,  SA_IDT,  SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT,  SA_IDT, -SA_IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_END,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT },
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*SA_END*/	{-SA_END,  SA_IDT,  SA_IDT, -SA_END, -SA_END, -SA_END, -SA_END, -SA_END, -SA_END, -SA_END,  SA_IDT, -SA_IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT },
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S1_ELS*/	{-SA_IDT,  SA_IDT,  SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT,  SA_IDT, -SA_IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  S2_ELS, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT },
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S2_ELS*/	{-SA_IDT,  SA_IDT,  SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT,  SA_IDT, -SA_IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_ELS,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT },
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*SA_ELS*/	{-SA_ELS,  SA_IDT,  SA_IDT, -SA_ELS, -SA_ELS, -SA_ELS, -SA_ELS, -SA_ELS, -SA_ELS, -SA_ELS,  SA_IDT, -SA_ELS, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT },
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S0_ARR*/	{-SA_IDT,  SA_IDT,  SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT,  SA_IDT, -SA_IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  S1_ARR,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT },              

            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S1_ARR*/	{-SA_IDT,  SA_IDT,  SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT,  SA_IDT, -SA_IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  S2_ARR,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT },    
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S2_ARR*/	{-SA_IDT,  SA_IDT,  SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT,  SA_IDT, -SA_IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S3_ARR,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT },           

            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S3_ARR*/	{-SA_IDT,  SA_IDT,  SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT,  SA_IDT, -SA_IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_ARR }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*SA_ARR*/	{-SA_ARR,  SA_IDT,  SA_IDT, -SA_ARR, -SA_ARR, -SA_ARR, -SA_ARR, -SA_ARR, -SA_ARR, -SA_ARR,  SA_IDT, -SA_ARR, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S0_OF_*/	{-SA_IDT,  SA_IDT,  SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT,  SA_IDT, -SA_IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_OF_,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_OR_,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*SA_OF_*/	{-SA_OF_,  SA_IDT,  SA_IDT, -SA_OF_, -SA_OF_, -SA_OF_, -SA_OF_, -SA_OF_, -SA_OF_, -SA_OF_,  SA_IDT, -SA_OF_, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*SA_OR_*/	{-SA_OR_,  SA_IDT,  SA_IDT, -SA_OR_, -SA_OR_, -SA_OR_, -SA_OR_, -SA_OR_, -SA_OR_, -SA_IDT,  SA_IDT, -SA_OR_, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S0_INT*/	{-SA_IDT,  SA_IDT,  SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT,  SA_IDT, -SA_IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IF_,  SA_IDT,  SA_IDT,  SA_IDT,  S1_INT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S1_INT*/	{-SA_IDT,  SA_IDT,  SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT,  SA_IDT, -SA_IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_INT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*SA_INT*/	{-SA_INT,  SA_IDT,  SA_IDT, -SA_INT, -SA_INT, -SA_INT, -SA_INT, -SA_INT, -SA_INT, -SA_INT,  SA_IDT, -SA_INT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*SA_IF_*/	{-SA_IF_,  SA_IDT,  SA_IDT, -SA_IF_, -SA_IF_, -SA_IF_, -SA_IF_, -SA_IF_, -SA_IF_, -SA_IF_,  SA_IDT, -SA_IF_, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S0_RET*/	{-SA_IDT,  SA_IDT,  SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT,  SA_IDT, -SA_IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  S1_RET,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S1_RET*/	{-SA_IDT,  SA_IDT,  SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT,  SA_IDT, -SA_IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              S2_RET,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S2_RET*/	{-SA_IDT,  SA_IDT,  SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT,  SA_IDT, -SA_IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  S3_RET,  SA_IDT,  SA_IDT,  SA_IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S3_RET*/	{-SA_IDT,  SA_IDT,  SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT,  SA_IDT, -SA_IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  S4_RET,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S4_RET*/	{-SA_IDT,  SA_IDT,  SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT,  SA_IDT, -SA_IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_RET,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*SA_RET*/	{-SA_RET,  SA_IDT,  SA_IDT, -SA_RET, -SA_RET, -SA_RET, -SA_RET, -SA_RET, -SA_RET, -SA_RET,  SA_IDT, -SA_RET, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S0_THN*/	{-SA_IDT,  SA_IDT,  SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT,  SA_IDT, -SA_IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  S1_THN,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S1_THN*/	{-SA_IDT,  SA_IDT,  SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT,  SA_IDT, -SA_IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  S2_THN,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S2_THN*/	{-SA_IDT,  SA_IDT,  SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT,  SA_IDT, -SA_IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_THN,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*SA_THN*/	{-SA_THN,  SA_IDT,  SA_IDT, -SA_THN, -SA_THN, -SA_THN, -SA_THN, -SA_THN, -SA_THN, -SA_THN,  SA_IDT, -SA_THN, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S0_WHL*/	{-SA_IDT,  SA_IDT,  SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT,  SA_IDT, -SA_IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  S1_WHL,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S1_WHL*/	{-SA_IDT,  SA_IDT,  SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT,  SA_IDT, -SA_IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  S2_WHL,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S2_WHL*/	{-SA_IDT,  SA_IDT,  SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT,  SA_IDT, -SA_IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  S3_WHL,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S3_WHL*/	{-SA_IDT,  SA_IDT,  SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT,  SA_IDT, -SA_IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_WHL,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*SA_WHL*/	{-SA_WHL,  SA_IDT,  SA_IDT, -SA_WHL, -SA_WHL, -SA_WHL, -SA_WHL, -SA_WHL, -SA_WHL, -SA_WHL,  SA_IDT, -SA_WHL, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S0_DO_*/	{-SA_IDT,  SA_IDT,  SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT,  SA_IDT, -SA_IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_DO_,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*SA_DO_*/	{-SA_DO_,  SA_IDT,  SA_IDT, -SA_DO_, -SA_DO_, -SA_DO_, -SA_DO_, -SA_DO_, -SA_DO_, -SA_DO_,  SA_IDT, -SA_DO_, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S0_VAR*/	{-SA_IDT,  SA_IDT,  SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT,  SA_IDT, -SA_IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S1_VAR,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S1_VAR*/	{-SA_IDT,  SA_IDT,  SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT,  SA_IDT, -SA_IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_VAR,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*SA_VAR*/	{-SA_VAR,  SA_IDT,  SA_IDT, -SA_VAR, -SA_VAR, -SA_VAR, -SA_VAR, -SA_VAR, -SA_VAR, -SA_VAR,  SA_IDT, -SA_VAR, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S0_NOT*/	{-SA_IDT,  SA_IDT,  SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT,  SA_IDT, -SA_IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  S1_NOT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*S1_NOT*/	{-SA_IDT,  SA_IDT,  SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT, -SA_IDT,  SA_IDT, -SA_IDT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_NOT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT }, 
              
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*SA_NOT*/	{-SA_NOT,  SA_IDT,  SA_IDT, -SA_NOT, -SA_NOT, -SA_NOT, -SA_NOT, -SA_NOT, -SA_NOT, -SA_NOT,  SA_IDT, -SA_NOT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT }, 
              
              };
  
//c must be [0-9A-Fa-f]
static uint8_t hexValue(char c) {
  uint8_t v = c - 0x30;
  if(v > 9) {
    v = (v & 0xf) + 9;
  }
  return v;
}

static int32_t lex(char *input) {
  
  char *src = input;
	
  uint64_t hash = 0;  
  uint64_t rValue = 0;
   
  while(1) {
  
    int state = S_STRT;
    char *token = src;
    char *t;
    
    do {
      uint8_t l = eqClass[*src];
      state = transitionTable[state][l];
    } while(state > 0 && src++);
    
    switch(-state) {
    case SA_WSP: //WHITESPACE
      break;
//				break;
//    case SA_CMT: //COMMENT somehow gets worse if I take this out?
//      continue;
    case SA_LEX: // LEXEM
      hash = (hash+*token)*hashmult;
      break;
    case SA_ASS: //ASSIGN OP
      hash = (hash+RVAL_ASSIGNOP)*hashmult;
      break;
    case SA_DEC: //DECIMAL NUMBER
      t = token;
      rValue = *t++ - 0x30;
      while(t < src) {
        rValue = (rValue*10) + *t++ - 0x30;
      }
      hash = (hash+((int)(rValue) ^ 0x8000))*hashmult;
      break;
    case SA_HEX: //HEX NUMBER
      t = &token[1];
      rValue = hexValue(*t++);
      while(t < src) {
        rValue = (rValue*16) + hexValue(*t++);
      }
      hash = (hash+((int)(rValue) ^ 0x4000))*hashmult;
      break;
      
    case SA_END: /* end */
      hash = (hash+RVAL_END)*hashmult;
      break;
    case SA_ELS: /* else */
      hash = (hash+RVAL_ELSE)*hashmult;
      break;
    case SA_ARR: /* array */
      hash = (hash+RVAL_ARRAY)*hashmult;
      break;
    case SA_OF_: /* of */
      hash = (hash+RVAL_OF)*hashmult;
      break;
    case SA_OR_: /* or */
      hash = (hash+RVAL_OR)*hashmult;
      break;
    case SA_INT: /* int */
      hash = (hash+RVAL_INT)*hashmult;
      break;
    case SA_IF_: /* if */
      hash = (hash+RVAL_IF)*hashmult;
      break;
    case SA_RET: /* return */
      hash = (hash+RVAL_RETURN)*hashmult;
      break;
    case SA_THN: /* then */
      hash = (hash+RVAL_THEN)*hashmult;
      break;
    case SA_WHL: /* while */
      hash = (hash+RVAL_WHILE)*hashmult;
      break;
    case SA_DO_: /* do */
      hash = (hash+RVAL_DO)*hashmult;
      break;
    case SA_VAR: /* var */
      hash = (hash+RVAL_VAR)*hashmult;
      break;
    case SA_NOT: /* not */
      hash = (hash+RVAL_NOT)*hashmult;
      break;
      
    case SA_IDT: //IDENTIFIER
      t = token;
      rValue = 0;
      while(t < src) {
        rValue = (rValue+*t)*hashmult;
        t++;
      }
      hash = (hash+((int)rValue))*hashmult;
      break;
      
    case S_STRT: //end of bufffer
      printf("%lx\n", hash);
      return 1;
      
    case -S_FAIL:
      fprintf(stderr, "unrecognized: \n");
      return 0;
    default:
      fprintf(stderr, "invalid accept state");
      return 0;
    }
    
//    char c = *src;
//    *src = '\0';
//    printf("%016llx: %s\n", hash, token);
//    *src = c;
    
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
  
  return 0;
}
