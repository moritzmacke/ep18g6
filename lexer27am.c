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

#define hashmult 0xbb433812a62b1dc1ULL //13493690561280548289ULL


enum { RVAL_END=256, RVAL_ARRAY, RVAL_OF, RVAL_INT, RVAL_RETURN, 
       RVAL_IF, RVAL_THEN, RVAL_ELSE, RVAL_WHILE, RVAL_DO, 
       RVAL_VAR, RVAL_NOT, RVAL_OR, RVAL_ASSIGNOP };
       
enum Action { WSPC = 0, CMNT, ASSN, LEXC, KWID, DNUM, HNUM, FAIL };

enum { FLEX = 0b00000, 
       FCOL = 0b00100, 
       FMIN = 0b01000, 
       
       FANY = 0b01100, 
       FCSH = 0b10000, 
       FWSP = 0b10100, 

       FNUM = 0b11000, 
       FLTR = 0b11100 };
            
enum { SANY = 0b00, 
       SEQU = 0b01, 
       SMIN = 0b10, 
       SHEX = 0b11 };



static const uint8_t actionLookup[][4] = {
  {FAIL, FAIL, FAIL, FAIL},
  {FAIL, FAIL, FAIL, FAIL},
  {FAIL, FAIL, FAIL, FAIL},
  {FAIL, FAIL, FAIL, FAIL},
  {FAIL, FAIL, FAIL, FAIL},
  {FAIL, FAIL, FAIL, FAIL},
  {FAIL, FAIL, FAIL, FAIL},
  {FAIL, FAIL, FAIL, FAIL},
  {FAIL, FAIL, FAIL, FAIL},
  {WSPC, WSPC, WSPC, WSPC}, // \t
  {WSPC, WSPC, WSPC, WSPC}, // \n
  {FAIL, FAIL, FAIL, FAIL},
  {FAIL, FAIL, FAIL, FAIL},
  {FAIL, FAIL, FAIL, FAIL},
  {FAIL, FAIL, FAIL, FAIL},
  {FAIL, FAIL, FAIL, FAIL},
  
  {FAIL, FAIL, FAIL, FAIL},
  {FAIL, FAIL, FAIL, FAIL},
  {FAIL, FAIL, FAIL, FAIL},
  {FAIL, FAIL, FAIL, FAIL},
  {FAIL, FAIL, FAIL, FAIL},
  {FAIL, FAIL, FAIL, FAIL},
  {FAIL, FAIL, FAIL, FAIL},
  {FAIL, FAIL, FAIL, FAIL},
  {FAIL, FAIL, FAIL, FAIL},
  {FAIL, FAIL, FAIL, FAIL},
  {FAIL, FAIL, FAIL, FAIL},
  {FAIL, FAIL, FAIL, FAIL},
  {FAIL, FAIL, FAIL, FAIL},
  {FAIL, FAIL, FAIL, FAIL},
  {FAIL, FAIL, FAIL, FAIL},
  {FAIL, FAIL, FAIL, FAIL},
  
  {WSPC, WSPC, WSPC, WSPC}, // ' '
  {FAIL, FAIL, FAIL, FAIL},
  {FAIL, FAIL, FAIL, FAIL},
  {LEXC, LEXC, LEXC, LEXC}, // '#'
  {FAIL, FAIL, FAIL, HNUM}, // '$'
  {FAIL, FAIL, FAIL, FAIL},
  {FAIL, FAIL, FAIL, FAIL},
  {FAIL, FAIL, FAIL, FAIL},
  {LEXC, LEXC, LEXC, LEXC}, // '('
  {LEXC, LEXC, LEXC, LEXC}, // ')'
  {LEXC, LEXC, LEXC, LEXC}, // '*'
  {LEXC, LEXC, LEXC, LEXC}, // '+'
  {LEXC, LEXC, LEXC, LEXC}, // ','
  {LEXC, LEXC, CMNT, LEXC}, // '-'
  {FAIL, FAIL, FAIL, FAIL},
  {FAIL, FAIL, FAIL, FAIL},

  {DNUM, DNUM, DNUM, DNUM}, // '0'
  {DNUM, DNUM, DNUM, DNUM}, // '1'
  {DNUM, DNUM, DNUM, DNUM}, // '2'
  {DNUM, DNUM, DNUM, DNUM}, // '3'
  {DNUM, DNUM, DNUM, DNUM}, // '4'
  {DNUM, DNUM, DNUM, DNUM}, // '5'
  {DNUM, DNUM, DNUM, DNUM}, // '6'
  {DNUM, DNUM, DNUM, DNUM}, // '7'
  {DNUM, DNUM, DNUM, DNUM}, // '8'
  {DNUM, DNUM, DNUM, DNUM}, // '9'
  {LEXC, ASSN, LEXC, LEXC}, // ':'
  {LEXC, LEXC, LEXC, LEXC}, // ';'
  {LEXC, LEXC, LEXC, LEXC}, // '<'
  {FAIL, FAIL, FAIL, FAIL}, // '='
  {FAIL, FAIL, FAIL, FAIL},
  {FAIL, FAIL, FAIL, FAIL},
  
  {FAIL, FAIL, FAIL, FAIL}, 
  {KWID, KWID, KWID, KWID}, // 'A'
  {KWID, KWID, KWID, KWID}, // 'B'
  {KWID, KWID, KWID, KWID}, // 'C'
  {KWID, KWID, KWID, KWID}, // 'D'
  {KWID, KWID, KWID, KWID}, // 'E'
  {KWID, KWID, KWID, KWID}, // 'F'
  {KWID, KWID, KWID, KWID}, // 'G'
  {KWID, KWID, KWID, KWID}, // 'H'
  {KWID, KWID, KWID, KWID}, // 'I'
  {KWID, KWID, KWID, KWID}, // 'J'
  {KWID, KWID, KWID, KWID}, // 'K'
  {KWID, KWID, KWID, KWID}, // 'L'
  {KWID, KWID, KWID, KWID}, // 'M'
  {KWID, KWID, KWID, KWID}, // 'N'
  {KWID, KWID, KWID, KWID}, // 'O'
  
  {KWID, KWID, KWID, KWID}, // 'P'
  {KWID, KWID, KWID, KWID}, // 'Q'
  {KWID, KWID, KWID, KWID}, // 'R'
  {KWID, KWID, KWID, KWID}, // 'S'
  {KWID, KWID, KWID, KWID}, // 'T'
  {KWID, KWID, KWID, KWID}, // 'U'
  {KWID, KWID, KWID, KWID}, // 'V'
  {KWID, KWID, KWID, KWID}, // 'W'
  {KWID, KWID, KWID, KWID}, // 'X'
  {KWID, KWID, KWID, KWID}, // 'Y'
  {KWID, KWID, KWID, KWID}, // 'Z'
  {LEXC, LEXC, LEXC, LEXC}, // '['
  {FAIL, FAIL, FAIL, FAIL},
  {LEXC, LEXC, LEXC, LEXC}, // ']'
  {FAIL, FAIL, FAIL, FAIL},
  {FAIL, FAIL, FAIL, FAIL},
  
  {FAIL, FAIL, FAIL, FAIL},
  {KWID, KWID, KWID, KWID}, // 'a'
  {KWID, KWID, KWID, KWID}, // 'b'
  {KWID, KWID, KWID, KWID}, // 'c'
  {KWID, KWID, KWID, KWID}, // 'd'
  {KWID, KWID, KWID, KWID}, // 'e'
  {KWID, KWID, KWID, KWID}, // 'f'
  {KWID, KWID, KWID, KWID}, // 'g'
  {KWID, KWID, KWID, KWID}, // 'h'
  {KWID, KWID, KWID, KWID}, // 'i'
  {KWID, KWID, KWID, KWID}, // 'j'
  {KWID, KWID, KWID, KWID}, // 'k'
  {KWID, KWID, KWID, KWID}, // 'l'
  {KWID, KWID, KWID, KWID}, // 'm'
  {KWID, KWID, KWID, KWID}, // 'n'
  {KWID, KWID, KWID, KWID}, // 'o'
  
  {KWID, KWID, KWID, KWID}, // 'p'
  {KWID, KWID, KWID, KWID}, // 'q'
  {KWID, KWID, KWID, KWID}, // 'r'
  {KWID, KWID, KWID, KWID}, // 's'
  {KWID, KWID, KWID, KWID}, // 't'
  {KWID, KWID, KWID, KWID}, // 'u'
  {KWID, KWID, KWID, KWID}, // 'v'
  {KWID, KWID, KWID, KWID}, // 'w'
  {KWID, KWID, KWID, KWID}, // 'x'
  {KWID, KWID, KWID, KWID}, // 'y'
  {KWID, KWID, KWID, KWID}, // 'z'
  {KWID, KWID, KWID, KWID}, 
  {FAIL, FAIL, FAIL, FAIL},
  {FAIL, FAIL, FAIL, FAIL},
  {FAIL, FAIL, FAIL, FAIL},
  {FAIL, FAIL, FAIL, FAIL}
};
    
static const uint8_t typeLookup[][128] = { 
  {
  FANY, FANY, FANY, FANY, FANY, FANY, FANY, FANY, FANY, FWSP, FWSP, FANY, FANY, FANY, FANY, FANY, 
	FANY, FANY, FANY, FANY, FANY, FANY, FANY, FANY, FANY, FANY, FANY, FANY, FANY, FANY, FANY, FANY, 
	FWSP, FANY, FANY, FLEX, FCSH, FANY, FANY, FANY, FLEX, FLEX, FLEX, FLEX, FLEX, FMIN, FANY, FANY, 
	FNUM, FNUM, FNUM, FNUM, FNUM, FNUM, FNUM, FNUM, FNUM, FNUM, FCOL, FLEX, FLEX, FANY, FANY, FANY, 
	FANY, FLTR, FLTR, FLTR, FLTR, FLTR, FLTR, FLTR, FLTR, FLTR, FLTR, FLTR, FLTR, FLTR, FLTR, FLTR, 
	FLTR, FLTR, FLTR, FLTR, FLTR, FLTR, FLTR, FLTR, FLTR, FLTR, FLTR, FLEX, FANY, FLEX, FANY, FANY, 
	FANY, FLTR, FLTR, FLTR, FLTR, FLTR, FLTR, FLTR, FLTR, FLTR, FLTR, FLTR, FLTR, FLTR, FLTR, FLTR, 
	FLTR, FLTR, FLTR, FLTR, FLTR, FLTR, FLTR, FLTR, FLTR, FLTR, FLTR, FANY, FANY, FANY, FANY, FANY,
  }, 
  { 
  SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, 
	SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, 
	SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SMIN, SANY, SANY, 
	SHEX, SHEX, SHEX, SHEX, SHEX, SHEX, SHEX, SHEX, SHEX, SHEX, SANY, SANY, SANY, SEQU, SANY, SANY, 
	SANY, SHEX, SHEX, SHEX, SHEX, SHEX, SHEX, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, 
	SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, 
	SANY, SHEX, SHEX, SHEX, SHEX, SHEX, SHEX, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, 
	SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY, SANY
  }
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




static int isHexDigit(char c) {
  return ((unsigned) c -'0') < 10 || ((unsigned) (c | 32) - 'a') < 6;
}

//c must be [0-9A-Fa-f]
static uint8_t hexValue(char c) {
  uint8_t v = c - 0x30;
  if(v > 9) {
    v = (v & 0xf) + 9;
  }
  return v;
}

#define esc(_c) (_c < 32? ' ' : _c)
 
static int32_t lex(char *src) {
  
  uint64_t nxt;
  uint64_t cur;
  
  uint64_t hash = 0;
  uint64_t rValue = 0;
//  uint64_t tmp = 0;
//  uint64_t lastChars = 0;
		
  nxt = *src++;
    
  while(1) { 
    cur = nxt;
    nxt = *src++;
    uint8_t action = actionLookup[cur][typeLookup[1][nxt]];


    if(action == LEXC) { 
      hash = (hash+cur)*hashmult;
//      printf("%llx, LEXEM %c %c\n", hash, cur, esc(nxt));
      continue;
    }
    
    if(action == WSPC) { continue; }
    
    if(action == KWID) { 
      rValue = cur * hashmult;
//      lastChars = cur;
      while(typeLookup[0][nxt] >= FNUM) { //fast but L1 cache misses...
        rValue = (rValue+nxt)*hashmult;
        cur = (cur << 8) + nxt;
        nxt = *src++;
      }
        
      //those 5 bits can differentiate all keywords
      //not great but works...
      uint8_t l = (rValue >> 2) &0x1f;
      if(kwLookup[l].keyword == cur) {
        rValue = kwLookup[l].value;
      }
      hash = (hash + (int)rValue)*hashmult;
//      printf("%llx, KWID %c %c\n", hash, cur, esc(nxt));
      continue;
    }

    if(action == DNUM) { 
      rValue = cur - 0x30;
      uint8_t v;
      while((v = nxt - 0x30) < 10) {
        rValue = (rValue*10) + v;
        nxt = *src++;
      }
      rValue ^= 0x8000;
      hash = hash*hashmult + (int)rValue*hashmult;
    }
    else if(action == ASSN) { 
      nxt = *src++;
      hash = (hash+RVAL_ASSIGNOP)*hashmult;
    }
    else if(action == CMNT) { 
      while((nxt = *src++) != '\n');
    }
    else if(action == HNUM) {
      rValue = hexValue(nxt);
      nxt = *src++;
      while(isHexDigit(nxt)) {
        rValue = (rValue*16) + hexValue(nxt);
        nxt = *src++;
      }
      rValue ^= 0x4000;
      hash = hash*hashmult + (int)rValue*hashmult;
    }
    else {
      if(cur == 0) { //end of file
        printf("%lx\n", hash);
        return 1;
      }
      else {
        fprintf(stderr, "Invalid character %c\n", cur);
        return 0;
      }
    }
    

//      printf("%llx %s %c %c\n", hash, output, cur, nxt == '\n'? ' ' : nxt);

	}

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
  input = mmap(NULL, inputSize+3, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_POPULATE, fd, 0);
  input[inputSize] = '\n'; //simplifies search for end of comment
  input[inputSize+1] = 0;
  
  if(input != MAP_FAILED) {
    lex(input);
  }
  else {
    fprintf(stderr, "mmap failed.\n");
    exit(1);
  }
 
  
/*
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
  
  input = malloc(inputSize+2);
  
  size_t read = fread(input, 1, inputSize, in);
  
  input[inputSize] = input[inputSize + 1] = 0;
  
  fclose(in);
  
  lex(input);
  
  */
  
  return 0;
}
