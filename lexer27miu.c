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
       
enum { FANY = 0b00000, 
       FCSH = 0b00100, //CSH:HEX -> hex; CSH:* -> fail
       FWSP = 0b01000, //
       FLEX = 0b01100, // 
       FMIN = 0b10000, //MIN:MIN -> cmt; MIN:* -> lex('-')
       FCOL = 0b10100, //COL:EQU -> ass; COL:* -> lex(':')
       FNUM = 0b11000, //
       FLTR = 0b11100 };
       
enum { SANY = 0b10, 
       SEQU = 0b00, //EQU:* -> fail
       SMIN = 0b01, //MIN:MIN = 10001 -> cmt; MIN:* -> lex('-')
       SHEX = 0b11 };
		
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
    uint8_t t = typeLookup[0][cur];
   
    rValue = cur * hashmult;  //pulling this out is mysteriously faster?
    
    if(t == FLEX) { 
      hash = hash*hashmult + rValue;
//      printf("%llx, LEXEM %c %c\n", hash, cur, esc(nxt));
      continue;
    }
    
    if(t == FWSP) { continue; }
    
    if(t >= FLTR) { 
//      rValue = cur * hashmult;
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
    

    t |= typeLookup[1][nxt];
      
    if(t >= 0b11000) { //NUM:* -> decimal
      rValue = cur - 0x30;
      uint8_t v;
      while((v = nxt - 0x30) < 10) {
        rValue = (rValue*10) + v;
        nxt = *src++;
      }
      rValue ^= 0x8000;
      hash = hash*hashmult + (int)rValue*hashmult;
    }
    else if(t >= 0b10101) { // lex(:)
//        hash = (hash+':')*hashmult;
      hash = hash*hashmult + rValue;
//        printf("%llx, LEXEM %c %c\n", hash, cur, esc(nxt));
    }
    else if(t == 0b10100) { //COL:EQU -> assign
      nxt = *src++;
      hash = hash*hashmult + RVAL_ASSIGNOP*hashmult;
//        printf("%llx, ASSIGN %c %c\n", hash, cur, esc(nxt));
    }
    else {
      switch(t) {
        
      case 0b00111: //CSH:HEX = 00111 -> hexnum
        rValue = hexValue(nxt);
        nxt = *src++;
        while(isHexDigit(nxt)) {
          rValue = (rValue*16) + hexValue(nxt);
          nxt = *src++;
        }
        rValue ^= 0x4000;
        hash = hash*hashmult + (int)rValue*hashmult;
//        printf("%llx, HEXNUM %c %c\n", hash, cur, esc(nxt));
        break;
        
      case 0b10001: //MIN:MIN -> comment; 
        while((nxt = *src++) != '\n');
        continue;
//        goto skipmult;

      case 0b10000: //MIN:*
      case 0b10010: //MIN:*
      case 0b10011: //MIN:*
        hash = hash*hashmult + ('-' * hashmult);
        break;
        
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
