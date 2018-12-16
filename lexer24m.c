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


enum { WSP, COL, LEX, MIN, ANY, CSH, NUM, LTR };
		
static const uint8_t typeLookup[] = { 
  ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, WSP, WSP, ANY, ANY, ANY, ANY, ANY, 
	ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, 
	WSP, ANY, ANY, LEX, CSH, ANY, ANY, ANY, LEX, LEX, LEX, LEX, LEX, MIN, ANY, ANY, 
                                                   //:
	NUM, NUM, NUM, NUM, NUM, NUM, NUM, NUM, NUM, NUM, COL, LEX, LEX, ANY, ANY, ANY, 
	ANY, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, 
	LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LEX, ANY, LEX, ANY, ANY, 
	ANY, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, LTR, 
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
	 
struct kwStat {
  uint64_t keyword;
  int value;
};
  
static const struct kwStat kwLookup[] = {
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




//c must be [0-9A-Fa-f]
static uint8_t hexValue(char c) {
  uint8_t v = c - 0x30;
  if(v > 9) {
    v = (v & 0xf) + 9;
  }
  return v;
}

/*
static const int8_t letterLookup[] = { 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1, 
   
	-1, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 
	25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, -1, -1, -1, -1, -1, 
	-1, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 
	25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, -1, -1, -1, -1, -1, 
  
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 	
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
 	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
 	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
 	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  };

static int8_t charValue(uint8_t c) {
  return letterLookup[c];
}
*/

union reg {
    uint16_t v;
    struct {
      char nxt;
      char cur;
    } c;
  };

//#define cur in.c.cur
//#define nxt in.c.nxt
 
static int32_t lex(char *src) {
  
// can't seem to get C to place both in same register...
//  register union reg in;
  
  uint64_t nxt;
  uint64_t cur;
  
  uint64_t hash = 0;
  
//	char *output = ""; //debug
  uint64_t rValue = 0;
  uint64_t lastChars = 0;
		
    
  //buffer needs 2 extra bytes?
  nxt = *src++;
    
  while(1 /*(c = *current++) != 0*/) { //this check is slower than switch 
    cur = nxt;
    nxt = *src++;
    uint8_t t = typeLookup[cur];
    if(t == WSP) continue; //but this is faster?
    if(t <= LEX) { //also barely faster than switch...?
      if((cur ^ nxt) != (':' ^ '=')) { //ASSIGN
        hash = (hash+cur)*hashmult;
//      printf("%llx, LEXEM %c %c\n", hash, cur, nxt);
      }
      else {
        nxt = *src++;
        hash = (hash+RVAL_ASSIGNOP)*hashmult;
//      printf("%llx, ASSIGN %c %c\n", hash, cur, nxt);
      }
    }
    else if(t == LTR) { //likewise faster outside switch as very common?
      rValue = cur * hashmult;
      lastChars = cur;
      while(typeLookup[nxt] >= NUM) { //fast but L1 cache misses...
        rValue = (rValue+nxt)*hashmult;
        lastChars = (lastChars << 8) | nxt;
        nxt = *src++;
      }
        
      //those 5 bits can differentiate all keywords
      //not great but works...
      uint8_t l = (rValue >> 2) &0x1f;
      if(kwLookup[l].keyword == lastChars) {
        rValue = kwLookup[l].value;
//          output = "KEYWORD";
      }
      
      hash = (hash+(int)rValue)*hashmult;
    }
    else {
      switch(t) {        
/*
      case COL:  // COLON(:) 
        rValue = (uint8_t) cur;
        if(nxt == '=') { // ASSIGN
//          output = "ASSIGN";
          nxt = *src++;
          rValue = RVAL_ASSIGNOP;
//          hash = (hash+RVAL_ASSIGNOP)*hashmult;
        }
        // LEXEMCHAR ':' 

        break;
*/
        
      case MIN: /* MINUS(-) */
        if(nxt == '-') { /*COMMENT */
//          output = "COMMENT";
          do {
            nxt = *src++;
          } while(nxt != '\n' && nxt != '\0');
          
          continue;
        }
        
        /*LEXEMCHAR '-' */
        rValue = cur;
        break;
        
      case NUM: /* DIGIT [0-9] */
//        output = "DECIMAL";
        rValue = cur - 0x30;
        uint8_t v;
        while((v = nxt - 0x30) < 10) {
          rValue = (rValue*10) + v;
          nxt = *src++;
        }
        rValue ^= 0x8000;
        break;

      case CSH: /* DOLLAR SIGN($) */
        if(isxdigit(nxt)) { /* HEXADECIMAL */
        //doing table based lookup for hexdigit actually slower than just isxdigit seems?
//          output = "HEXNUM";
          rValue = hexValue(nxt);
          nxt = *src++;
          while(isxdigit(nxt)) {
            rValue = (rValue*16) + hexValue(nxt);
            nxt = *src++;
          }
          rValue ^= 0x4000;
//          hash = (hash+((int)(rValue) ^ 0x4000))*hashmult;
          break;
        }

      //fall through to fail

      case ANY:  /* INVALID CHARACTER */
      default:
        if(cur == 0) { //end of file
          printf("%lx\n", hash);
          return 1;
        }
        else {
          fprintf(stderr, "Invalid character %c\n", cur);
          return 0;
        }
      }
      
      hash = (hash+((int)rValue))*hashmult;
    }

      
//      printf("%llx %s %c %c\n", hash, output, cur, nxt == '\n'? ' ' : nxt);

	}
    
		return 1; 
}

int main(int argc, char *argv[])
{

  int fd;
  struct stat filestats;
  size_t inputSize = 0;
  char *input = NULL;

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

  // allowed??                vv
  input = mmap(NULL, inputSize+2, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_POPULATE, fd, 0);
  input[inputSize] = input[inputSize+1] = 0;
  
  if(input != MAP_FAILED) {
    lex(input);
  }
  else {
    fprintf(stderr, "mmap failed.\n");
    exit(1);
  }
  
  return 0;
}
