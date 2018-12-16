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


enum { NUL = 0, DGT /* [0-9] */, LTR /* [G-Zg-z] */, COL /* : */, 
       MIN /* - */, CSH /* $ */, SPC /*[\t ]*/, LBK /* \n */, 
LEX /* [;(),<#[]+*] */, EQU /* = */, HEX /* [A-Fa-z] */, ANY };
	
static const uint8_t eqClass[] = { 
    NUL, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, SPC, LBK, ANY, ANY, LBK, ANY, ANY, 
		ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, 
		SPC, ANY, ANY, LEX, CSH, ANY, ANY, ANY, LEX, LEX, LEX, LEX, LEX, MIN, ANY, ANY, 
		DGT, DGT, DGT, DGT, DGT, DGT, DGT, DGT, DGT, DGT, COL, LEX, LEX, EQU, ANY, ANY, 
		ANY, HEX, HEX, HEX, HEX, HEX, HEX, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, 
		LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LEX, ANY, LEX, ANY, ANY, 
		ANY, HEX, HEX, HEX, HEX, HEX, HEX, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, 
		LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, ANY, ANY, ANY, ANY, ANY, 
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
	  S_1ASS, /*any -> -X */
	  S_0CMT, /*'-' -> S_1CMT; any -> -S_0LEX */ 
	  S_1CMT, /*'\n' -> -S_1CMT; any -> S_1CMT */
	  S_0DEC, /*DGT -> S_0DEC; any -> -S_0DEC */
	  S_0HEX, /*[0-9A-Fa-f] -> S_1HEX; any -> FAIL */ 
	  S_1HEX, /* [0-9A-Fa-f] -> S_1HEX; any -> -S_1HEX*/
	  S_0IDT, /*[0-9A-Fa-z] -> S_0IDT; any -> -S_0IDT*/
	  S_FAIL = -128
  };    
	
static const int8_t transitionTable[][12] = {
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX,     ANY
/*O_STRT*/	{ S_STRT,  S_0DEC,  S_0IDT,  S_0ASS,  S_0CMT,  S_0HEX,  S_0WSP,  S_0WSP,  S_0LEX,  S_FAIL,  S_0IDT,  S_FAIL},
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX
/*S_0WSP*/	{-S_0WSP, -S_0WSP, -S_0WSP, -S_0WSP, -S_0WSP, -S_0WSP, -S_0WSP, -S_0WSP, -S_0WSP, -S_0WSP, -S_0WSP, -S_0WSP},
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX
/*S_0LEX*/	{-S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX},
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX
/*S_0ASS*/	{-S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX,  S_1ASS, -S_0LEX, -S_0LEX},
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX
/*S_1ASS*/	{-S_1ASS, -S_1ASS, -S_1ASS, -S_1ASS, -S_1ASS, -S_1ASS, -S_1ASS, -S_1ASS, -S_1ASS, -S_1ASS, -S_1ASS, -S_1ASS},
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX
/*S_0CMT*/	{-S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX,  S_1CMT, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX, -S_0LEX},
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX
/*S_1CMT*/	{ S_1CMT,  S_1CMT,  S_1CMT,  S_1CMT,  S_1CMT,  S_1CMT,  S_1CMT, -S_1CMT,  S_1CMT,  S_1CMT,  S_1CMT,  S_1CMT},
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX
/*S_0DEC*/	{-S_0DEC,  S_0DEC, -S_0DEC, -S_0DEC, -S_0DEC, -S_0DEC, -S_0DEC, -S_0DEC, -S_0DEC, -S_0DEC, -S_0DEC, -S_0DEC},
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX
/*S_0HEX*/	{ S_FAIL,  S_1HEX,  S_FAIL,  S_FAIL,  S_FAIL,  S_FAIL,  S_FAIL,  S_FAIL,  S_FAIL,  S_FAIL,  S_1HEX,  S_FAIL},
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX
/*S_1HEX*/	{-S_1HEX,  S_1HEX, -S_1HEX, -S_1HEX, -S_1HEX, -S_1HEX, -S_1HEX, -S_1HEX, -S_1HEX, -S_1HEX,  S_1HEX, -S_1HEX},
            //NUL,     DGT,     LTR,     COL,     MIN,     CSH,     SPC,     LBK,     LEX,     EQU,     HEX
/*S_0IDT*/	{-S_0IDT,  S_0IDT,  S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT, -S_0IDT,  S_0IDT, -S_0IDT},
	};
	 
struct kwStat {
  uint64_t keyword;
  int value;
};
  
const struct kwStat kwLookup[] = {
    {0, 0}, /* 0 */
    {0, 0},
    {0x0000000000696e74, RVAL_INT},
    {0, 0},
    {0x000000000000646f, RVAL_DO},
    {0, 0},
    {0, 0},
    {0x0000006172726179, RVAL_ARRAY},
    
    {0x000072657475726e, RVAL_RETURN}, /* 8 */
    {0, 0},
    {0, 0},
    {0x000000007468656e, RVAL_THEN},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    
    {0, 0}, /* 16 */
    {0, 0},
    {0x0000000000766172, RVAL_VAR},
    {0x0000000000006966, RVAL_IF},
    {0x00000000006e6f74, RVAL_NOT},
    {0x0000000000006f66, RVAL_OF},
    {0x0000007768696c65, RVAL_WHILE},
    {0, 0},
    
    {0x0000000000006f72, RVAL_OR}, /* 24 */
    {0, 0},
    {0x00000000656c7365, RVAL_ELSE},
    {0, 0},
    {0, 0},
    {0x0000000000656e64, RVAL_END},
    {0, 0},
    {0, 0},
    {0, 0},
};

char* const kwPrint[] = {"KW_END", "KW_ARRAY", "KW_OF", "KW_INT", "KW_RETURN", "KW_IF", 
                         "KW_THEN", "KW_ELSE", "KW_WHILE", "KW_DO", "KW_VAR", "KW_NOT", "KW_OR"};

  
//c must be [0-9A-Fa-f]
uint8_t hexValue(char c) {
  uint8_t v = c - 0x30;
  if(v > 9) {
    v = (v & 0xf) + 9;
  }
  return v;
}
 
size_t inputSize = 0;
char *input = NULL;

int32_t lex() {
  
  char c, n;
  char *current = input;
	
  uint64_t hash = 0;
  
	char *output = ""; //debug
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
      output = "WHITESPACE";
      continue;
//				break;
    case S_1CMT:
      output = "COMMENT";
      break;
    case S_0LEX:
      output = "LEXEM";
      hash = (hash+*token)*hashmult;
      break;
    case S_1ASS:
      output = "ASSIGN";
      hash = (hash+RVAL_ASSIGNOP)*hashmult;
      break;
    case S_0DEC:
      output = "DECIMAL";
      t = token;
      rValue = *t++ - 0x30;
      while(t < current) {
        rValue = (rValue*10) + *t++ - 0x30;
      }
      hash = (hash+((int)(rValue) ^ 0x8000))*hashmult;
      break;
    case S_1HEX:
      output = "HEXNUM";
      t = &token[1];
      rValue = hexValue(*t++);
      while(t < current) {
        rValue = (rValue*16) + hexValue(*t++);
      }
      hash = (hash+((int)(rValue) ^ 0x4000))*hashmult;
      break;
    case S_0IDT:
      output = "KWID";
      t = token;
      rValue = 0;
      lastChars = 0;
      while(t < current) {
        lastChars = (lastChars << 8) | *t;
        rValue = (rValue+*t)*hashmult;
        t++;
      }

      uint8_t l = (rValue >> 2) &0x1f;
      if(kwLookup[l].keyword == lastChars) {
        rValue = kwLookup[l].value;
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
    
//    printf("[" + currentToken + "] " + output + ": " + token);	
  }
  

    
	return 1; 
}

int main(int argc, char *argv[])
{

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
  
  lex();
  

  
  
  return 0;
}
