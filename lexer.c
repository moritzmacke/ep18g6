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

uint64_t hash(char *s)
{
  uint64_t r=0;
  char *p;
  for (p=s; *p; p++)
    r = (r+*p)*hashmult;
  return r;
}

enum { RVAL_END=256, RVAL_ARRAY, RVAL_OF, RVAL_INT, RVAL_RETURN, 
       RVAL_IF, RVAL_THEN, RVAL_ELSE, RVAL_WHILE, RVAL_DO, 
       RVAL_VAR, RVAL_NOT, RVAL_OR, RVAL_ASSIGNOP };

enum State {START, F_LEXEM, S_ASSLEX, F_ASSIGN, S_DIGIT, F_DIGIT, S_HEX, S_HEX2, F_HEX, 
            S_COMMENT, S_COMMENT2, /*F_COMMENT,*/ S_KWID, F_KWID, F_FAIL};
	
enum CharClassStart {NUL, WSP, LEX, COL, NUM, CSH, MIN, LTR};
		
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
 
int eof = 0;
//char printBuffer[256];
 
size_t currentToken = 0;
size_t tokenLength = 0;
size_t inputSize = 0;
char *input = NULL;


size_t countAssign = 0;
size_t countKeyword = 0;
size_t countIdent = 0;
size_t countLexem = 0;
size_t countComment = 0;
size_t countDecimal = 0;
size_t countHexnum = 0;

char nextChar() {
  size_t pos = currentToken + tokenLength;
  return pos < inputSize? input[pos] : -1;
}

int32_t lex() {
	
//		char *output = ""; //debug
		enum State state = START;
//		uint64_t idHash = 0;
    uint64_t rValue = 0;
    uint64_t lastChars = 0;

    
		int found = FALSE;
		
		while(!found) {
			char c = nextChar();
			
			switch(state) {
			case START:
				switch(typeLookup[((uint8_t)c)]) {
				case WSP: /* WHITESPACE [\t\n ] */
					currentToken++;
					break;
				case LEX: /* CERTAIN LEXEMCHAR */
					state = F_LEXEM;
					tokenLength++;
					break;
				case COL:  /* COLON(:) */
					state = S_ASSLEX;
					tokenLength++;
					break;
				case CSH: /* DOLLAR SIGN($) */
					state = S_HEX;
					tokenLength++;
					break;
				case MIN: /* MINUS(-) */
					state = S_COMMENT;
					tokenLength++;
					break;
				case NUM: /* DIGIT [0-9] */
					state = S_DIGIT;
          rValue = c - 0x30;
					tokenLength++;
					break;
				case LTR: /* LETTER [A-Za-z] */
					state = S_KWID;
					rValue = (rValue+c)*hashmult;
          lastChars = (lastChars << 8) | c;
					tokenLength++;
					break;
				case NUL:  /* INVALID CHARACTER */
				default:
					if(currentToken + tokenLength >= inputSize) {
            eof = 1;
						return 0;
					}
					else {
						state = F_FAIL;
						break;
					}
				}
				break;
				
			case S_ASSLEX: /* either lexemchar ':' or assign op ':=' */
				if(c == '=') {
					state = F_ASSIGN;
					tokenLength++;
				}
				else {
					state = F_LEXEM;
				}
				break;
			case S_COMMENT: /* either lexemchar '-' or comment '--.' */
				if(c == '-') {
					state = S_COMMENT2;
					tokenLength++;
				}
				else {
					state = F_LEXEM;
				}
				break;
			case S_COMMENT2: /* in comment as long as no linebreak */
				if(c == '\n') {
//					state = F_COMMENT; should not return comment actually...
          countComment++;
          //rather skip
          state = START;
          currentToken += tokenLength + 1; //also skip newline
          tokenLength = 0;
          
				}
				else {
					tokenLength++;
				}
				break;
			case S_DIGIT: /* accept decimal digits */
				if(isdigit(c)) {
          rValue = rValue*10 + (c - 0x30);
					tokenLength++;
				}
				else {
					state = F_DIGIT;
				}
				break;
			case S_HEX: /* need at least one hex digit after '$' */
				if(isxdigit(c)) {
					state = S_HEX2;
          rValue = hexValue(c);
					tokenLength++;
				}
				else {
					state = F_FAIL;
				}
				break;
			case S_HEX2: /* accept hex digits */
				if(isxdigit(c)) {
          rValue = (rValue*16) + hexValue(c);
					tokenLength++;
				}
				else {
					state = F_HEX;
				}
				break;
			case S_KWID: /* combined keyword/identifier accept [0-9A-Za-z] */
				if(isalnum(c)) {
					rValue = (rValue+c)*hashmult;
          lastChars = (lastChars << 8) | c;
					tokenLength++;
				}
				else {
					state = F_KWID;
				}
				break;
				
//end states
			case F_KWID: /* found keyword or identifier*/
				found = TRUE;
        countIdent++;
//				output = "IDENT";
        //rValue contains hash
        
				for(int i=0; i<numKeywords; i++) {
					if(keywords[i] == lastChars) {
            rValue = RVAL_END + i;
//						output = kwPrint[i];
            countIdent--;
            countKeyword++;
						break;
					}
				}
        
				break;
				
			case F_ASSIGN: /* found assign op */
        countAssign++;
        found = TRUE;
        rValue = RVAL_ASSIGNOP;
//				output = "ASSIGN";
				
				break;

/* remove				
			case F_COMMENT:
				found = TRUE;
				output = "COMMENT";
				break;
*/				
			case F_DIGIT: /* found decimal integer */
        countDecimal++;
				found = TRUE;
        rValue ^= 0x8000;
//				output = "NUMBER";
				break;
				
			case F_HEX: /* found hexadecimal integer */
        countHexnum++;
				found = TRUE;
        rValue ^= 0x4000;
//				output = "HEXNUM";
				break;
				
			case F_LEXEM: /* found lexemchar */
        countLexem++;
				found = TRUE;
        rValue = input[currentToken];
//				output = "LEXCHR";
				break;

			case F_FAIL:
        fprintf(stderr, "unrecognized: %c\n", c);
				exit(1);
				
			default:
				fprintf(stderr, "invalid state\n");
				exit(1);
			}
		}
		
//debug
//    memcpy(printBuffer, &input[currentToken], tokenLength);
//    printBuffer[tokenLength] = '\0';
//    printf("[%lu] %s(%lx): %s\n", currentToken, output, rValue, printBuffer);
		
		currentToken += tokenLength;
		tokenLength = 0;
		
		
    //yylexx returns a signed int so we have to too
    //even though internally unsigned long are used...
		return (int32_t) rValue; 
	}

int main(int argc, char *argv[])
{
  
  uint64_t x, r;
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
  
  for (x=0; r=lex(), eof==0;) {
//    printf("%lx\n", r);
    x = (x+r)*hashmult;
  }
  
  printf("%llx\n",x);
  
  printf("ASSIGN: %llu\n", countAssign);
  printf("KEYWORD: %llu\n", countKeyword);
  printf("IDENT: %llu\n", countIdent);
  printf("LEXEM: %llu\n", countLexem);
  printf("COMMENT: %llu\n", countComment);
  printf("DECIMAL: %llu\n", countDecimal);
  printf("HEXNUM: %llu\n", countHexnum);
  
  
  return 0;
}
