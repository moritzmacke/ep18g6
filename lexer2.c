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


enum { NUL, WSP, LEX, COL, NUM, CSH, MIN, LTR };
		
const uint8_t typeLookup[] = { 
  NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, WSP, WSP, NUL, NUL, NUL, NUL, NUL, 
	NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, 
	WSP, NUL, NUL, LEX, CSH, NUL, NUL, NUL, LEX, LEX, LEX, LEX, LEX, MIN, NUL, NUL, 
	NUM, NUM, NUM, NUM, NUM, NUM, NUM, NUM, NUM, NUM, COL, LEX, LEX, NUL, NUL, NUL, 
	NUL, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, 
	LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LEX, NUL, LEX, NUL, NUL, 
	NUL, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, 
	LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, NUL, NUL, NUL, NUL, NUL, 
	NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, 
	NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, 
	NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, 
	NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, 
	NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, 
	NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, 
	NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, 
	NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL, NUL
  };
	
const int numKeywords = 13;
const uint64_t keywords[] = {
    0x0000000000656e64, /* end */  
    0x0000006172726179, /* array */ 
    0x0000000000006f66, /* of */ 
    0x0000000000696e74, /* int */ 
    0x000072657475726e, /* return */ 
    0x0000000000006966, /* if */ 
    0x000000007468656e, /* then */ 
    0x00000000656c7365, /* else */ 
    0x0000007768696c65, /* while */ 
    0x000000000000646f, /* do */ 
    0x0000000000766172, /* var */ 
    0x00000000006e6f74, /* not */ 
    0x0000000000006f72  /* or */ 
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
  char *end = &input[inputSize];
	
  uint64_t hash = 0;
  
//	char *output = ""; //debug
  uint64_t rValue = 0;
  uint64_t lastChars = 0;
		
  while(current < end) {
    c = *current++;
    uint8_t t = typeLookup[((uint8_t)c)];
    if(t == WSP) continue; 
    
    switch(t) {
      case LEX: /* CERTAIN LEXEMCHAR */
//        output = "LEXEM";
        hash = (hash+c)*hashmult;
        break;
        
      case COL:  /* COLON(:) */
        n = current < end ? *current : -1;
        if(n == '=') { /* ASSIGN*/
//          output = "ASSIGN";
          current++;
          hash = (hash+RVAL_ASSIGNOP)*hashmult;
        }
        else {  /*LEXEMCHAR */
//          output = "LEXEM";
          hash = (hash+c)*hashmult;
        }
        break;
        
      case CSH: /* DOLLAR SIGN($) */
        n = current < end ? *current : -1;
        if(isxdigit(n)) { /* HEXADECIMAL */
//          output = "HEXNUM";
          rValue = hexValue(n);
          while(++current < end) {
            c = *current;
            if(!isxdigit(c)) break;
            rValue = (rValue*16) + hexValue(c);
          }
          hash = (hash+((int)(rValue) ^ 0x4000))*hashmult;
        }
        else {
          //fail
          fprintf(stderr, "Invalid character \n");
          return 0;
        }
        break;
        
      case MIN: /* MINUS(-) */
        n = current < end ? *current : -1;
        if(n == '-') { /*COMMENT */
//          output = "COMMENT";
          while(++current < end && *current != '\n');
        }
        else { /*LEXEMCHAR */
          hash = (hash+c)*hashmult;
        }
        break;
        
      case NUM: /* DIGIT [0-9] */
//        output = "DECIMAL";
        rValue = c - 0x30;
        while(current < end) {
          c = *current;
          if(!isdigit(c)) break;
          current++;
          rValue = (rValue*10) + c - 0x30;
        }
        hash = (hash+((int)(rValue) ^ 0x8000))*hashmult;
        break;
        
      case LTR: /* LETTER [A-Za-z] */
        rValue = c * hashmult;
        lastChars = c;
        while(current < end) {
          c = *current;
          if(!isalnum(c)) break;
          current++;
          rValue = (rValue+c)*hashmult;
          lastChars = (lastChars << 8) | c;
        }

//				output = "IDENT";
        
        for(int i=0; i<numKeywords; i++) {
          if(keywords[i] == lastChars) {
            rValue = RVAL_END + i;
//						output = kwPrint[i];
            break;
          }
        }
        
        hash = (hash+((int)rValue))*hashmult;
        
        break;
        
      case NUL:  /* INVALID CHARACTER */
      default:
          fprintf(stderr, "Invalid character \n");
          return 0;

      }


		}
		
    printf("%lx\n", hash);
    
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
  
  input = malloc(inputSize);
  
  size_t read = fread(input, 1, inputSize, in);
  
  fclose(in);

//  printf("%lu bytes read\n", read);
  
  lex();
  

  
  
  return 0;
}
