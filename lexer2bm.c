#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

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
        
        //those 5 bits can differentiate all keywords
        //not great but works...
        uint8_t l = (rValue >> 2) &0x1f;
        if(kwLookup[l].keyword == lastChars) {
          rValue = kwLookup[l].value;
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

  int fd;
  struct stat filestats;

  if (argc != 2) {
    fprintf(stderr,"Usage: %s <file>\n",argv[0]);
    exit(1);
  }
  
  fd = open(argv[1], O_RDONLY);
  
  if (fd < 0) {
    fprintf(stderr, "Cannot open %s\n", argv[1]);
    exit(1);
  }
  
  fstat(fd, &filestats);
  inputSize = filestats.st_size;

  input = mmap(NULL, inputSize, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);
  
  if(input != MAP_FAILED) {
    lex();
  }
  else {
    fprintf(stderr, "mmap failed.\n");
    exit(1);
  }
  
  return 0;
}
