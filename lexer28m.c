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


//enum { RVAL_END=256, RVAL_ARRAY, RVAL_OF, RVAL_INT, RVAL_RETURN, 
//       RVAL_IF, RVAL_THEN, RVAL_ELSE, RVAL_WHILE, RVAL_DO, 
//       RVAL_VAR, RVAL_NOT, RVAL_OR, RVAL_ASSIGNOP };


//precomputed RVAL * hashmult
#define R_END 0x433812a62b1dc100ULL
#define R_ARRAY 0xfe7b4ab8d148dec1ULL
#define R_OF 0xb9be82cb7773fc82ULL
#define R_INT 0x7501bade1d9f1a43ULL
#define R_RETURN 0x3044f2f0c3ca3804ULL
#define R_IF 0xeb882b0369f555c5ULL
#define R_THEN 0xa6cb631610207386ULL
#define R_ELSE 0x620e9b28b64b9147ULL
#define R_WHILE 0x1d51d33b5c76af08ULL
#define R_DO 0xd8950b4e02a1ccc9ULL
#define R_VAR 0x93d84360a8ccea8aULL
#define R_NOT 0x4f1b7b734ef8084bULL
#define R_OR 0xa5eb385f523260cULL
#define R_ASSIGNOP 0xc5a1eb989b4e43cdULL
       
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
  uint64_t value;
};
  
static const struct kwStat kwLookup[] = {
    {0, 0}, /* 0 */
    {0, 0},
    {0x0000000000696e74, R_INT},
    {0, 0},
    {0x000000000000646f, R_DO},
    {0, 0},
    {0, 0},
    {0x0000006172726179, R_ARRAY},
    
    {0x000072657475726e, R_RETURN}, /* 8 */
    {0, 0},
    {0, 0},
    {0x000000007468656e, R_THEN},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    
    {0, 0}, /* 16 */
    {0, 0},
    {0x0000000000766172, R_VAR},
    {0x0000000000006966, R_IF},
    {0x00000000006e6f74, R_NOT},
    {0x0000000000006f66, R_OF},
    {0x0000007768696c65, R_WHILE},
    {0, 0},
    
    {0x0000000000006f72, R_OR}, /* 24 */
    {0, 0},
    {0x00000000656c7365, R_ELSE},
    {0, 0},
    {0, 0},
    {0x0000000000656e64, R_END},
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
    
    rValue = cur * hashmult;  //
    
    if(t == FLEX) { 
      hash = hash*hashmult + rValue;
//      printf("%llx, LEXEM %c %c\n", hash, cur, esc(nxt));
      continue;
    }
    
    if(t == FWSP) { continue; }
    
    
    if(t >= FLTR) { 
//      rValue = cur * hashmult;
//      lastChars = cur;
      //no ident in llinput > length 7 so this does not actually loop...
      //gcc removes the src++, does use src[offset] and adjusts src at end
      //turn out faster somehow...
      while(typeLookup[0][nxt] >= FNUM) {
        rValue = (rValue+nxt)*hashmult;
        cur = (cur << 8) + nxt;
        nxt = *src++;
        if(typeLookup[0][nxt] < FNUM) break;
        rValue = (rValue+nxt)*hashmult;
        cur = (cur << 8) + nxt;
        nxt = *src++;
        if(typeLookup[0][nxt] < FNUM) break;
        rValue = (rValue+nxt)*hashmult;
        cur = (cur << 8) + nxt;
        nxt = *src++;
        if(typeLookup[0][nxt] < FNUM) break;
        rValue = (rValue+nxt)*hashmult;
        cur = (cur << 8) + nxt;
        nxt = *src++;
        if(typeLookup[0][nxt] < FNUM) break;
        rValue = (rValue+nxt)*hashmult;
        cur = (cur << 8) + nxt;
        nxt = *src++;
        if(typeLookup[0][nxt] < FNUM) break;
        rValue = (rValue+nxt)*hashmult;
        cur = (cur << 8) + nxt;
        nxt = *src++;
        if(typeLookup[0][nxt] < FNUM) break;
        rValue = (rValue+nxt)*hashmult;
        cur = (cur << 8) + nxt;
        nxt = *src++;
        if(typeLookup[0][nxt] < FNUM) break;
        rValue = (rValue+nxt)*hashmult;
        cur = (cur << 8) + nxt;
        nxt = *src++;
      }

      hash *= hashmult;
      
      //those 5 bits can differentiate all keywords
      //not great but works...
      uint8_t l = (rValue >> 2) &0x1f;
      if(kwLookup[l].keyword == cur) {
        hash += kwLookup[l].value;
      }
      else {
        hash += ((int)rValue)*hashmult;
      }

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
      hash = (hash + (int)rValue)*hashmult;
    }
    else if(t >= 0b10101) { // lex(:)
//        hash = (hash+':')*hashmult;
      hash = hash*hashmult + rValue;
//        printf("%llx, LEXEM %c %c\n", hash, cur, esc(nxt));
    }
    else if(t == 0b10100) { //COL:EQU -> assign
      nxt = *src++;
      hash = hash*hashmult + R_ASSIGNOP;
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
        hash = (hash + (int)rValue)*hashmult;
//        printf("%llx, HEXNUM %c %c\n", hash, cur, esc(nxt));
        break;
        
      case 0b10001: //MIN:MIN -> comment; 
        while((nxt = *src++) != '\n');
        continue;

      case 0b10000: //MIN:*
      case 0b10010: //MIN:*
      case 0b10011: //MIN:*
        hash = (hash + '-') * hashmult;
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
