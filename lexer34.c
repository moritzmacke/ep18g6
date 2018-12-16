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


//38 Equivalance classes
#define EQCLASSES 38

enum { NUL = 0, DGT /* [0-9] */, LTR /* [G-Zgjkmpqxz] */, COL /* : */, 
       MIN /* - */, CSH /* $ */, SPC /*[\t ]*/, LBK /* \n */, SEM /* ; */, 
       EQU /* = */, HEX /* [A-Fb] */, ANY, LCA, LCC, LCD, LCE, LCF, LCH, 
       LCI, LCL, LCN, LCO, LCR, LCS, LCT, LCU, LCV, LCW, LCY,
       LBR /* ( */, RBR, COM /* , */, LTS /* < */, HSH /* # */, LAB /* [ */, 
       RAB, PLS /* + */, STR /* * */ };
  
static const uint8_t eqClass[] = { 
    NUL, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, SPC, LBK, ANY, ANY, LBK, ANY, ANY, 
		ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, 
		SPC, ANY, ANY, HSH, CSH, ANY, ANY, ANY, LBR, RBR, STR, PLS, COM, MIN, ANY, ANY, 
		DGT, DGT, DGT, DGT, DGT, DGT, DGT, DGT, DGT, DGT, COL, SEM, LTS, EQU, ANY, ANY, 
		ANY, HEX, HEX, HEX, HEX, HEX, HEX, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, 
		LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LAB, ANY, RAB, ANY, ANY, 
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
	
  
enum State { 
    S_STRT =  0, 
	  S0_ASS, 
	  S0_CMT, 
	  S0_HEX, /* */    
    S0_END, /* e_ */
    S1_END, /* en_ */
    S1_ELS, /* el_ */
    S2_ELS, /* els_ */
    S0_ARR, /* a_ */
    S1_ARR, /* ar_ */
    
    S2_ARR = 10, /* arr_ */
    S3_ARR, /* arra_ */
    S0_OF_, /* o_ */
    S0_INT, /* i_ */
    S1_INT, /* in_ */
    S0_RET, /* r_ */
    S1_RET, /* re_ */
    S2_RET, /* ret_ */
    S3_RET, /* retu_ */
    S4_RET, /* retur_ */
    
    S0_THN = 20, /* t_ */
    S1_THN, /* th_ */
    S2_THN, /* the_ */
    S0_WHL, /* w_ */
    S1_WHL, /* wh_ */
    S2_WHL, /* whi_ */
    S3_WHL, /* whil_ */
    S0_DO_, /* d_ */
    S0_VAR, /* v_ */
    S1_VAR, /* va_ */
    
    S0_NOT = 30, /* n_ */
    S1_NOT, /* no_ */

    SA_WSP = 32, 
    SA_SEM, /* ';' */
    SA_LBR, /* '(' */
    SA_RBR, /* ')' */
    SA_COM, /* ',' */
    SA_COL, /* ':' */
    SA_LTS, /* '<' */
    SA_HSH, /* '#' */
    
    SA_LAB = 40, /* '[' */
    SA_RAB, /* ']' */
    SA_MIN, /* '-' */
    SA_PLS, /* '+' */
    SA_STR, /* '*' */
	  SA_ASS, /* */
	  SA_CMT, 
	  SA_DEC, /* */
	  SA_HEX, /* */
	  SA_IDT, /* */    
    
    SA_END = 50, /* end_ */
    SA_ELS, /* else_ */
    SA_ARR, /* array_ */
    SA_OF_, /* of_ */
    SA_OR_, /* or_ */
    SA_INT, /* int_ */
    SA_IF_, /* if_ */
    SA_RET, /* return_ */
    SA_THN, /* then_ */
    SA_WHL, /* while_ */
    
    SA_DO_ = 60, /* do_ */
    SA_VAR, /* var_ */
    SA_NOT, /* not_ */
  };    


       
//negative return "states" not actually jumped to
enum { //R_WSPC = 0, 
    R_LSEM = ';', 
    R_LLBR = '(', 
    R_LRBR = ')', 
    R_LCOM = ',', 
    R_LCOL = ':', 
    R_LLTS = '<', 
    R_LHSH = '#', 
    R_LLAB = '[', 
    R_LRAB = ']', 
    R_LMIN = '-', 
    R_LPLS = '+', 
    R_LSTR = '*', 
    R_KEND = RVAL_END, 
    R_KELS = RVAL_ELSE, 
    R_KARR = RVAL_ARRAY, 
    R_KWOF = RVAL_OF, 
    R_KWOR = RVAL_OR, 
    R_KINT = RVAL_INT, 
    R_KWIF = RVAL_IF, 
    R_KRET = RVAL_RETURN, 
    R_KTHN = RVAL_THEN, 
    R_KWHL = RVAL_WHILE, 
    R_KWDO = RVAL_DO, 
    R_KVAR = RVAL_VAR, 
    R_KNOT = RVAL_NOT, 
    R_ASSN = RVAL_ASSIGNOP,  
    
    R_WSPC = 300,
    R_DECN = 301, 
    R_HEXN = 302, 
    R_IDNT = 303, 
    
    R_FAIL = 304};
     
static const int16_t transitionTable[][38] = { 
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*S_STRT*/  {-R_FAIL,  SA_DEC,  SA_IDT,  S0_ASS,  S0_CMT,  S0_HEX,  SA_WSP,  SA_WSP,  SA_SEM, -R_FAIL,  SA_IDT, -R_FAIL, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S0_ARR,  SA_IDT,  S0_DO_,  S0_END,  SA_IDT,  SA_IDT,  S0_INT,  SA_IDT,  S0_NOT,  S0_OF_,  S0_RET,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              S0_THN,  SA_IDT,  S0_VAR,  S0_WHL,  SA_IDT,  SA_LBR,  SA_RBR,  SA_COM,  SA_LTS,  SA_HSH,  SA_LAB,  SA_RAB,
            //PLS,     STR
              SA_PLS,  SA_STR },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*S0_ASS*/	{-R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL,  SA_ASS, -R_LCOL, -R_LCOL, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
             -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
             -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL,
            //PLS,     STR
             -R_LCOL, -R_LCOL },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*S0_CMT*/	{-R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN,  SA_CMT, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
             -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
             -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN,
            //PLS,     STR
             -R_LMIN, -R_LMIN },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*S0_HEX*/	{-R_FAIL,  SA_HEX, -R_FAIL, -R_FAIL, -R_FAIL, -R_FAIL, -R_FAIL, -R_FAIL, -R_FAIL, -R_FAIL,  SA_HEX, -R_FAIL, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_HEX,  SA_HEX,  SA_HEX,  SA_HEX,  SA_HEX, -R_FAIL, -R_FAIL, -R_FAIL, -R_FAIL, -R_FAIL, -R_FAIL, -R_FAIL, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
             -R_FAIL, -R_FAIL, -R_FAIL, -R_FAIL, -R_FAIL, -R_FAIL, -R_FAIL, -R_FAIL, -R_FAIL, -R_FAIL, -R_FAIL, -R_FAIL,
            //PLS,     STR
             -R_FAIL, -R_FAIL },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*S0_END*/	{-R_IDNT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,  SA_IDT, -R_IDNT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  S1_ELS,  S1_END,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,
            //PLS,     STR
             -R_IDNT, -R_IDNT },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*S1_END*/	{-R_IDNT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,  SA_IDT, -R_IDNT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_END,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,
            //PLS,     STR
             -R_IDNT, -R_IDNT },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*S1_ELS*/	{-R_IDNT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,  SA_IDT, -R_IDNT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  S2_ELS, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,
            //PLS,     STR
             -R_IDNT, -R_IDNT },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*S2_ELS*/	{-R_IDNT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,  SA_IDT, -R_IDNT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_ELS,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,
            //PLS,     STR
             -R_IDNT, -R_IDNT },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*S0_ARR*/	{-R_IDNT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,  SA_IDT, -R_IDNT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  S1_ARR,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,
            //PLS,     STR
             -R_IDNT, -R_IDNT },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*S1_ARR*/	{-R_IDNT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,  SA_IDT, -R_IDNT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  S2_ARR,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,
            //PLS,     STR
             -R_IDNT, -R_IDNT },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*S2_ARR*/	{-R_IDNT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,  SA_IDT, -R_IDNT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S3_ARR,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,
            //PLS,     STR
             -R_IDNT, -R_IDNT },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*S3_ARR*/	{-R_IDNT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,  SA_IDT, -R_IDNT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_ARR, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,
            //PLS,     STR
             -R_IDNT, -R_IDNT },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*S0_OF_*/	{-R_IDNT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,  SA_IDT, -R_IDNT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_OF_,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_OR_,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,
            //PLS,     STR
             -R_IDNT, -R_IDNT },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*S0_INT*/	{-R_IDNT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,  SA_IDT, -R_IDNT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IF_,  SA_IDT,  SA_IDT,  SA_IDT,  S1_INT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,
            //PLS,     STR
             -R_IDNT, -R_IDNT },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*S1_INT*/	{-R_IDNT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,  SA_IDT, -R_IDNT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_INT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,
            //PLS,     STR
             -R_IDNT, -R_IDNT },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*S0_RET*/	{-R_IDNT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,  SA_IDT, -R_IDNT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  S1_RET,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,
            //PLS,     STR
             -R_IDNT, -R_IDNT },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*S1_RET*/	{-R_IDNT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,  SA_IDT, -R_IDNT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              S2_RET,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,
            //PLS,     STR
             -R_IDNT, -R_IDNT },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*S2_RET*/	{-R_IDNT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,  SA_IDT, -R_IDNT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  S3_RET,  SA_IDT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,
            //PLS,     STR
             -R_IDNT, -R_IDNT },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*S3_RET*/	{-R_IDNT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,  SA_IDT, -R_IDNT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  S4_RET,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,
            //PLS,     STR
             -R_IDNT, -R_IDNT },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*S4_RET*/	{-R_IDNT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,  SA_IDT, -R_IDNT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_RET,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,
            //PLS,     STR
             -R_IDNT, -R_IDNT },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*S0_THN*/	{-R_IDNT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,  SA_IDT, -R_IDNT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  S1_THN,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,
            //PLS,     STR
             -R_IDNT, -R_IDNT },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*S1_THN*/	{-R_IDNT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,  SA_IDT, -R_IDNT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  S2_THN,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,
            //PLS,     STR
             -R_IDNT, -R_IDNT },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*S2_THN*/	{-R_IDNT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,  SA_IDT, -R_IDNT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_THN,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,
            //PLS,     STR
             -R_IDNT, -R_IDNT },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*S0_WHL*/	{-R_IDNT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,  SA_IDT, -R_IDNT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  S1_WHL,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,
            //PLS,     STR
             -R_IDNT, -R_IDNT },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*S1_WHL*/	{-R_IDNT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,  SA_IDT, -R_IDNT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  S2_WHL,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,
            //PLS,     STR
             -R_IDNT, -R_IDNT },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*S2_WHL*/	{-R_IDNT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,  SA_IDT, -R_IDNT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  S3_WHL,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,
            //PLS,     STR
             -R_IDNT, -R_IDNT },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*S3_WHL*/	{-R_IDNT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,  SA_IDT, -R_IDNT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_WHL,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,
            //PLS,     STR
             -R_IDNT, -R_IDNT },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*S0_DO_*/	{-R_IDNT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,  SA_IDT, -R_IDNT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_DO_,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,
            //PLS,     STR
             -R_IDNT, -R_IDNT },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*S0_VAR*/	{-R_IDNT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,  SA_IDT, -R_IDNT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              S1_VAR,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,
            //PLS,     STR
             -R_IDNT, -R_IDNT },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*S1_VAR*/	{-R_IDNT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,  SA_IDT, -R_IDNT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_VAR,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,
            //PLS,     STR
             -R_IDNT, -R_IDNT },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*S0_NOT*/	{-R_IDNT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,  SA_IDT, -R_IDNT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  S1_NOT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,
            //PLS,     STR
             -R_IDNT, -R_IDNT },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*S1_NOT*/	{-R_IDNT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,  SA_IDT, -R_IDNT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_NOT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,
            //PLS,     STR
             -R_IDNT, -R_IDNT },


            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*SA_WSP*/	{-R_WSPC, -R_WSPC, -R_WSPC,  S0_ASS,  S0_CMT, -R_WSPC,  SA_WSP,  SA_WSP,  SA_SEM, -R_WSPC, -R_WSPC, -R_WSPC, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
             -R_WSPC, -R_WSPC, -R_WSPC, -R_WSPC, -R_WSPC, -R_WSPC, -R_WSPC, -R_WSPC, -R_WSPC, -R_WSPC, -R_WSPC, -R_WSPC, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
             -R_WSPC, -R_WSPC, -R_WSPC, -R_WSPC, -R_WSPC,  SA_LBR,  SA_RBR,  SA_COM,  SA_LTS,  SA_HSH,  SA_LAB,  SA_RAB,
            //PLS,     STR
              SA_PLS,  SA_STR },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX
/*SA_SEM*/	{-R_LSEM, -R_LSEM, -R_LSEM, -R_LSEM, -R_LSEM, -R_LSEM, -R_LSEM, -R_LSEM, -R_LSEM, -R_LSEM, -R_LSEM, -R_LSEM, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
             -R_LSEM, -R_LSEM, -R_LSEM, -R_LSEM, -R_LSEM, -R_LSEM, -R_LSEM, -R_LSEM, -R_LSEM, -R_LSEM, -R_LSEM, -R_LSEM, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
             -R_LSEM, -R_LSEM, -R_LSEM, -R_LSEM, -R_LSEM, -R_LSEM, -R_LSEM, -R_LSEM, -R_LSEM, -R_LSEM, -R_LSEM, -R_LSEM,
            //PLS,     STR
             -R_LSEM, -R_LSEM },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX
/*SA_LBR*/	{-R_LLBR, -R_LLBR, -R_LLBR, -R_LLBR, -R_LLBR, -R_LLBR, -R_LLBR, -R_LLBR, -R_LLBR, -R_LLBR, -R_LLBR, -R_LLBR, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
             -R_LLBR, -R_LLBR, -R_LLBR, -R_LLBR, -R_LLBR, -R_LLBR, -R_LLBR, -R_LLBR, -R_LLBR, -R_LLBR, -R_LLBR, -R_LLBR, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
             -R_LLBR, -R_LLBR, -R_LLBR, -R_LLBR, -R_LLBR, -R_LLBR, -R_LLBR, -R_LLBR, -R_LLBR, -R_LLBR, -R_LLBR, -R_LLBR,
            //PLS,     STR
             -R_LLBR, -R_LLBR },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX
/*SA_RBR*/	{-R_LRBR, -R_LRBR, -R_LRBR, -R_LRBR, -R_LRBR, -R_LRBR, -R_LRBR, -R_LRBR, -R_LRBR, -R_LRBR, -R_LRBR, -R_LRBR, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
             -R_LRBR, -R_LRBR, -R_LRBR, -R_LRBR, -R_LRBR, -R_LRBR, -R_LRBR, -R_LRBR, -R_LRBR, -R_LRBR, -R_LRBR, -R_LRBR, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
             -R_LRBR, -R_LRBR, -R_LRBR, -R_LRBR, -R_LRBR, -R_LRBR, -R_LRBR, -R_LRBR, -R_LRBR, -R_LRBR, -R_LRBR, -R_LRBR,
            //PLS,     STR
             -R_LRBR, -R_LRBR },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX
/*SA_COM*/	{-R_LCOM, -R_LCOM, -R_LCOM, -R_LCOM, -R_LCOM, -R_LCOM, -R_LCOM, -R_LCOM, -R_LCOM, -R_LCOM, -R_LCOM, -R_LCOM, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
             -R_LCOM, -R_LCOM, -R_LCOM, -R_LCOM, -R_LCOM, -R_LCOM, -R_LCOM, -R_LCOM, -R_LCOM, -R_LCOM, -R_LCOM, -R_LCOM, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
             -R_LCOM, -R_LCOM, -R_LCOM, -R_LCOM, -R_LCOM, -R_LCOM, -R_LCOM, -R_LCOM, -R_LCOM, -R_LCOM, -R_LCOM, -R_LCOM,
            //PLS,     STR
             -R_LCOM, -R_LCOM },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX
/*SA_COL*/	{-R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
             -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
             -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL, -R_LCOL,
            //PLS,     STR
             -R_LCOL, -R_LCOL },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX
/*SA_LTS*/	{-R_LLTS, -R_LLTS, -R_LLTS, -R_LLTS, -R_LLTS, -R_LLTS, -R_LLTS, -R_LLTS, -R_LLTS, -R_LLTS, -R_LLTS, -R_LLTS, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
             -R_LLTS, -R_LLTS, -R_LLTS, -R_LLTS, -R_LLTS, -R_LLTS, -R_LLTS, -R_LLTS, -R_LLTS, -R_LLTS, -R_LLTS, -R_LLTS, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
             -R_LLTS, -R_LLTS, -R_LLTS, -R_LLTS, -R_LLTS, -R_LLTS, -R_LLTS, -R_LLTS, -R_LLTS, -R_LLTS, -R_LLTS, -R_LLTS,
            //PLS,     STR
             -R_LLTS, -R_LLTS },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX
/*SA_HSH*/	{-R_LHSH, -R_LHSH, -R_LHSH, -R_LHSH, -R_LHSH, -R_LHSH, -R_LHSH, -R_LHSH, -R_LHSH, -R_LHSH, -R_LHSH, -R_LHSH, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
             -R_LHSH, -R_LHSH, -R_LHSH, -R_LHSH, -R_LHSH, -R_LHSH, -R_LHSH, -R_LHSH, -R_LHSH, -R_LHSH, -R_LHSH, -R_LHSH, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
             -R_LHSH, -R_LHSH, -R_LHSH, -R_LHSH, -R_LHSH, -R_LHSH, -R_LHSH, -R_LHSH, -R_LHSH, -R_LHSH, -R_LHSH, -R_LHSH,
            //PLS,     STR
             -R_LHSH, -R_LHSH },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX
/*SA_LAB*/	{-R_LLAB, -R_LLAB, -R_LLAB, -R_LLAB, -R_LLAB, -R_LLAB, -R_LLAB, -R_LLAB, -R_LLAB, -R_LLAB, -R_LLAB, -R_LLAB, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
             -R_LLAB, -R_LLAB, -R_LLAB, -R_LLAB, -R_LLAB, -R_LLAB, -R_LLAB, -R_LLAB, -R_LLAB, -R_LLAB, -R_LLAB, -R_LLAB, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
             -R_LLAB, -R_LLAB, -R_LLAB, -R_LLAB, -R_LLAB, -R_LLAB, -R_LLAB, -R_LLAB, -R_LLAB, -R_LLAB, -R_LLAB, -R_LLAB,
            //PLS,     STR
             -R_LLAB, -R_LLAB },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX
/*SA_RAB*/	{-R_LRAB, -R_LRAB, -R_LRAB, -R_LRAB, -R_LRAB, -R_LRAB, -R_LRAB, -R_LRAB, -R_LRAB, -R_LRAB, -R_LRAB, -R_LRAB, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
             -R_LRAB, -R_LRAB, -R_LRAB, -R_LRAB, -R_LRAB, -R_LRAB, -R_LRAB, -R_LRAB, -R_LRAB, -R_LRAB, -R_LRAB, -R_LRAB, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
             -R_LRAB, -R_LRAB, -R_LRAB, -R_LRAB, -R_LRAB, -R_LRAB, -R_LRAB, -R_LRAB, -R_LRAB, -R_LRAB, -R_LRAB, -R_LRAB,
            //PLS,     STR
             -R_LRAB, -R_LRAB },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX
/*SA_MIN*/	{-R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
             -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
             -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN, -R_LMIN,
            //PLS,     STR
             -R_LMIN, -R_LMIN },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX
/*SA_PLS*/	{-R_LPLS, -R_LPLS, -R_LPLS, -R_LPLS, -R_LPLS, -R_LPLS, -R_LPLS, -R_LPLS, -R_LPLS, -R_LPLS, -R_LPLS, -R_LPLS, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
             -R_LPLS, -R_LPLS, -R_LPLS, -R_LPLS, -R_LPLS, -R_LPLS, -R_LPLS, -R_LPLS, -R_LPLS, -R_LPLS, -R_LPLS, -R_LPLS, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
             -R_LPLS, -R_LPLS, -R_LPLS, -R_LPLS, -R_LPLS, -R_LPLS, -R_LPLS, -R_LPLS, -R_LPLS, -R_LPLS, -R_LPLS, -R_LPLS,
            //PLS,     STR
             -R_LPLS, -R_LPLS },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX
/*SA_STR*/	{-R_LSTR, -R_LSTR, -R_LSTR, -R_LSTR, -R_LSTR, -R_LSTR, -R_LSTR, -R_LSTR, -R_LSTR, -R_LSTR, -R_LSTR, -R_LSTR, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
             -R_LSTR, -R_LSTR, -R_LSTR, -R_LSTR, -R_LSTR, -R_LSTR, -R_LSTR, -R_LSTR, -R_LSTR, -R_LSTR, -R_LSTR, -R_LSTR, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
             -R_LSTR, -R_LSTR, -R_LSTR, -R_LSTR, -R_LSTR, -R_LSTR, -R_LSTR, -R_LSTR, -R_LSTR, -R_LSTR, -R_LSTR, -R_LSTR,
            //PLS,     STR
             -R_LSTR, -R_LSTR },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX
/*SA_ASS*/	{-R_ASSN, -R_ASSN, -R_ASSN, -R_ASSN, -R_ASSN, -R_ASSN, -R_ASSN, -R_ASSN, -R_ASSN, -R_ASSN, -R_ASSN, -R_ASSN, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
             -R_ASSN, -R_ASSN, -R_ASSN, -R_ASSN, -R_ASSN, -R_ASSN, -R_ASSN, -R_ASSN, -R_ASSN, -R_ASSN, -R_ASSN, -R_ASSN, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
             -R_ASSN, -R_ASSN, -R_ASSN, -R_ASSN, -R_ASSN, -R_ASSN, -R_ASSN, -R_ASSN, -R_ASSN, -R_ASSN, -R_ASSN, -R_ASSN,
            //PLS,     STR
             -R_ASSN, -R_ASSN },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX
/*SA_CMT*/	{ SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT,  SA_WSP,  SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT,  SA_CMT,
            //PLS,     STR
              SA_CMT,  SA_CMT },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX
/*SA_DEC*/	{-R_DECN,  SA_DEC, -R_DECN, -R_DECN, -R_DECN, -R_DECN, -R_DECN, -R_DECN, -R_DECN, -R_DECN, -R_DECN, -R_DECN, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
             -R_DECN, -R_DECN, -R_DECN, -R_DECN, -R_DECN, -R_DECN, -R_DECN, -R_DECN, -R_DECN, -R_DECN, -R_DECN, -R_DECN, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
             -R_DECN, -R_DECN, -R_DECN, -R_DECN, -R_DECN, -R_DECN, -R_DECN, -R_DECN, -R_DECN, -R_DECN, -R_DECN, -R_DECN,
            //PLS,     STR
             -R_DECN, -R_DECN },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*SA_HEX*/	{-R_HEXN,  SA_HEX, -R_HEXN, -R_HEXN, -R_HEXN, -R_HEXN, -R_HEXN, -R_HEXN, -R_HEXN, -R_HEXN,  SA_HEX, -R_HEXN, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_HEX,  SA_HEX,  SA_HEX,  SA_HEX,  SA_HEX, -R_HEXN, -R_HEXN, -R_HEXN, -R_HEXN, -R_HEXN, -R_HEXN, -R_HEXN, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
             -R_HEXN, -R_HEXN, -R_HEXN, -R_HEXN, -R_HEXN, -R_HEXN, -R_HEXN, -R_HEXN, -R_HEXN, -R_HEXN, -R_HEXN, -R_HEXN,
            //PLS,     STR
             -R_HEXN, -R_HEXN },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*SA_IDT*/	{-R_IDNT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,  SA_IDT, -R_IDNT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT, -R_IDNT,
            //PLS,     STR
             -R_IDNT, -R_IDNT },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*SA_END*/	{-R_KEND,  SA_IDT,  SA_IDT, -R_KEND, -R_KEND, -R_KEND, -R_KEND, -R_KEND, -R_KEND, -R_KEND,  SA_IDT, -R_IDNT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_KEND, -R_KEND, -R_KEND, -R_KEND, -R_KEND, -R_KEND, -R_KEND,
            //PLS,     STR
             -R_KEND, -R_KEND },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*SA_ELS*/	{-R_KELS,  SA_IDT,  SA_IDT, -R_KELS, -R_KELS, -R_KELS, -R_KELS, -R_KELS, -R_KELS, -R_KELS,  SA_IDT, -R_KELS, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_KELS, -R_KELS, -R_KELS, -R_KELS, -R_KELS, -R_KELS, -R_KELS,
            //PLS,     STR
             -R_KELS, -R_KELS },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*SA_ARR*/	{-R_KARR,  SA_IDT,  SA_IDT, -R_KARR, -R_KARR, -R_KARR, -R_KARR, -R_KARR, -R_KARR, -R_KARR,  SA_IDT, -R_KARR, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_KARR, -R_KARR, -R_KARR, -R_KARR, -R_KARR, -R_KARR, -R_KARR,
            //PLS,     STR
             -R_KARR, -R_KARR },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*SA_OF_*/	{-R_KWOF,  SA_IDT,  SA_IDT, -R_KWOF, -R_KWOF, -R_KWOF, -R_KWOF, -R_KWOF, -R_KWOF, -R_KWOF,  SA_IDT, -R_KWOF, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_KWOF, -R_KWOF, -R_KWOF, -R_KWOF, -R_KWOF, -R_KWOF, -R_KWOF,
            //PLS,     STR
             -R_KWOF, -R_KWOF },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*SA_OR_*/	{-R_KWOR,  SA_IDT,  SA_IDT, -R_KWOR, -R_KWOR, -R_KWOR, -R_KWOR, -R_KWOR, -R_KWOR, -R_IDNT,  SA_IDT, -R_KWOR, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_KWOR, -R_KWOR, -R_KWOR, -R_KWOR, -R_KWOR, -R_KWOR, -R_KWOR,
            //PLS,     STR
             -R_KWOR, -R_KWOR },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*SA_INT*/	{-R_KINT,  SA_IDT,  SA_IDT, -R_KINT, -R_KINT, -R_KINT, -R_KINT, -R_KINT, -R_KINT, -R_KINT,  SA_IDT, -R_KINT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_KINT, -R_KINT, -R_KINT, -R_KINT, -R_KINT, -R_KINT, -R_KINT,
            //PLS,     STR
             -R_KINT, -R_KINT },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*SA_IF_*/	{-R_KWIF,  SA_IDT,  SA_IDT, -R_KWIF, -R_KWIF, -R_KWIF, -R_KWIF, -R_KWIF, -R_KWIF, -R_KWIF,  SA_IDT, -R_KWIF, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_KWIF, -R_KWIF, -R_KWIF, -R_KWIF, -R_KWIF, -R_KWIF, -R_KWIF,
            //PLS,     STR
             -R_KWIF, -R_KWIF },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*SA_RET*/	{-R_KRET,  SA_IDT,  SA_IDT, -R_KRET, -R_KRET, -R_KRET, -R_KRET, -R_KRET, -R_KRET, -R_KRET,  SA_IDT, -R_KRET, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_KRET, -R_KRET, -R_KRET, -R_KRET, -R_KRET, -R_KRET, -R_KRET,
            //PLS,     STR
             -R_KRET, -R_KRET },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*SA_THN*/	{-R_KTHN,  SA_IDT,  SA_IDT, -R_KTHN, -R_KTHN, -R_KTHN, -R_KTHN, -R_KTHN, -R_KTHN, -R_KTHN,  SA_IDT, -R_KTHN, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT , -R_KTHN, -R_KTHN, -R_KTHN, -R_KTHN, -R_KTHN, -R_KTHN, -R_KTHN,
            //PLS,     STR
             -R_KTHN, -R_KTHN },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*SA_WHL*/	{-R_KWHL,  SA_IDT,  SA_IDT, -R_KWHL, -R_KWHL, -R_KWHL, -R_KWHL, -R_KWHL, -R_KWHL, -R_KWHL,  SA_IDT, -R_KWHL, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_KWHL, -R_KWHL, -R_KWHL, -R_KWHL, -R_KWHL, -R_KWHL, -R_KWHL,
            //PLS,     STR
             -R_KWHL, -R_KWHL },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*SA_DO_*/	{-R_KWDO,  SA_IDT,  SA_IDT, -R_KWDO, -R_KWDO, -R_KWDO, -R_KWDO, -R_KWDO, -R_KWDO, -R_KWDO,  SA_IDT, -R_KWDO, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_KWDO, -R_KWDO, -R_KWDO, -R_KWDO, -R_KWDO, -R_KWDO, -R_KWDO,
            //PLS,     STR
             -R_KWDO, -R_KWDO },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*SA_VAR*/	{-R_KVAR,  SA_IDT,  SA_IDT, -R_KVAR, -R_KVAR, -R_KVAR, -R_KVAR, -R_KVAR, -R_KVAR, -R_KVAR,  SA_IDT, -R_KVAR, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_KVAR, -R_KVAR, -R_KVAR, -R_KVAR, -R_KVAR, -R_KVAR, -R_KVAR,
            //PLS,     STR
             -R_KVAR, -R_KVAR },
             
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     SEM,     EQU,     HEX,     ANY
/*SA_NOT*/	{-R_KNOT,  SA_IDT,  SA_IDT, -R_KNOT, -R_KNOT, -R_KNOT, -R_KNOT, -R_KNOT, -R_KNOT, -R_KNOT,  SA_IDT, -R_KNOT, 
            //LCA,     LCC,     LCD,     LCE,     LCF,     LCH,     LCI,     LCL,     LCN,     LCO,     LCR,     LCS 
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, 
            //LCT,     LCU,     LCV,     LCW,     LCY,     LBR,     RBR,     COM,     LTS,     HSH,     LAB,     RAB
              SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT,  SA_IDT, -R_KNOT, -R_KNOT, -R_KNOT, -R_KNOT, -R_KNOT, -R_KNOT, -R_KNOT,
            //PLS,     STR
             -R_KNOT, -R_KNOT },
             
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
  
  uint64_t c;
  char *current = input;
//	int16_t *tTable = (int16_t *) &transitionTable[0][0];
  
  uint64_t hash = 0;
  
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
    
    state = -state;
    if(state == R_WSPC) continue;
    
    if(state <= R_ASSN) {
      hash = (hash+state)*hashmult;
    }
    else if(state == R_IDNT){
      t = token;
      rValue = 0;
      while(t < current) {
        rValue = (rValue+*t)*hashmult;
        t++;
      }
      hash = (hash+((int)rValue))*hashmult;
    }
    else if(state == R_DECN) {
      t = token;
      rValue = *t++ - 0x30;
      while(t < current) {
        rValue = (rValue*10) + *t++ - 0x30;
      }
      hash = (hash+((int)(rValue) ^ 0x8000))*hashmult;
    }
    else if(state == R_HEXN) {
      t = &token[1];
      rValue = hexValue(*t++);
      while(t < current) {
        rValue = (rValue*16) + hexValue(*t++);
      }
      hash = (hash+((int)(rValue) ^ 0x4000))*hashmult;
    }
    else {
      
      if(c == 0) {
        printf("%lx\n", hash);
      }
      else {
        fprintf(stderr, "unrecognized: \n");
      }
      break;
    }
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
