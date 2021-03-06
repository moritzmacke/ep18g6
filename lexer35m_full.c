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


enum { ANY = 0, COL /* : */, MIN /* - */, CSH /* $ */, SPC /*[\t ]*/, LBK /* \n */, 
       LEX /* ,;()[]<#+* */, EQU /* = */, DGT /* [0-9] */, HEX /* [A-Fb] */, LTR /* [G-Zgjkmpqxz] */ };
  
static const uint8_t eqClass[] = { 
    ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, ANY, SPC, LBK, ANY, ANY, LBK, ANY, ANY, 
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
	
enum State { 
    //low byte highest bit unset -> no assign
    LP_NOP_SPC = 0, 
    LP_NOP_LEX = 256, 
    LP_NOP_COL = 512, 
    LP_NOP_MIN = 768, 
    LP_NOP_CSH = 1024, 
    LP_NOP_CMT = 1280, 
    
    
    //low byte highest bit set
    LP_MOV_SPC = 128, 
    LP_MOV_LEX = 384, 
    LP_MOV_COL = 640, 
    LP_MOV_MIN = 896, 
    LP_MOV_CSH = 1152
  };
 
enum BreakState {
    KWID = 0, // 0b0000 0000
    DNUM = 1, // 0b0000 0001
    ASSN = 2, // 0b0000 0010
    HNUM = 3, // 0b0000 0011
    FAIL = 4, // 0b0000 0100
  
    //bit unset
    BK_NOP_KWI = -144, // 0b11111111.01110000
    BK_NOP_DEC = -143, // 0b11111111.01110001
    BK_NOP_ASS = -142, // 0b11111111.01110010
    BK_NOP_HEX = -141, // 0b11111111.01110011
    BK_NOP_FAL = -140, // 0b11111111.01110100

    //
    BK_MOV_KWI = -16, // 0b11111111.11110000
    BK_MOV_DEC = -15, // 0b11111111.11110001
    BK_MOV_ASS = -14, // 0b11111111.11110010
    BK_MOV_HEX = -13, // 0b11111111.11110011
    BK_MOV_FAL = -12  // 0b11111111.11110100
  };

  
//previously tried state -= transitionTable[state][] but was bad idea, 
//saves one cmp instruction in theory but really complicates things.
//full 7-bit table... 
//try 16 bit and return offset of next state directly.
     
static const int16_t transitionTable[][128] = { 

/*0: SPC*/  
  {-140,-140,-140,-140,-140,-140,-140,-140,-140,   0,   0,-140,-140,   0,-140,-140, 
   -140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140, 
      0,-140,-140, 256,1024,-140,-140,-140, 256, 256, 256, 256, 256, 768,-140,-140, 
   -143,-143,-143,-143,-143,-143,-143,-143,-143,-143, 512, 256, 256,-140,-140,-140, 
   -140,-144,-144,-144,-144,-144,-144,-144,-144,-144,-144,-144,-144,-144,-144,-144, 
   -144,-144,-144,-144,-144,-144,-144,-144,-144,-144,-144, 256,-140, 256,-140,-140, 
   -140,-144,-144,-144,-144,-144,-144,-144,-144,-144,-144,-144,-144,-144,-144,-144, 
   -144,-144,-144,-144,-144,-144,-144,-144,-144,-144,-144,-140,-140,-140,-140,-140 }, 

/*128: SPC+MOV*/  
  {-140,-140,-140,-140,-140,-140,-140,-140,-140,   0,   0,-140,-140,   0,-140,-140, 
   -140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140, 
      0,-140,-140, 256,1024,-140,-140,-140, 256, 256, 256, 256, 256, 768,-140,-140, 
   -143,-143,-143,-143,-143,-143,-143,-143,-143,-143, 512, 256, 256,-140,-140,-140, 
   -140,-144,-144,-144,-144,-144,-144,-144,-144,-144,-144,-144,-144,-144,-144,-144, 
   -144,-144,-144,-144,-144,-144,-144,-144,-144,-144,-144, 256,-140, 256,-140,-140, 
   -140,-144,-144,-144,-144,-144,-144,-144,-144,-144,-144,-144,-144,-144,-144,-144, 
   -144,-144,-144,-144,-144,-144,-144,-144,-144,-144,-144,-140,-140,-140,-140,-140 }, 
    
/*256: LEX*/  
  { -12, -12, -12, -12, -12, -12, -12, -12, -12, 128, 128, -12, -12, 128, -12, -12, 
		-12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, 
		128, -12, -12, 384,1152, -12, -12, -12, 384, 384, 384, 384, 384, 896, -12, -12, 
		-15, -15, -15, -15, -15, -15, -15, -15, -15, -15, 640, 384, 384, -12, -12, -12, 
		-12, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, 
		-16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, 384, -12, 384, -12, -12, 
		-12, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, 
		-16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -12, -12, -12, -12, -12 },

/*384: LEX+MOV*/
  { -12, -12, -12, -12, -12, -12, -12, -12, -12, 128, 128, -12, -12, 128, -12, -12, 
		-12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, 
		128, -12, -12, 384,1152, -12, -12, -12, 384, 384, 384, 384, 384, 896, -12, -12, 
		-15, -15, -15, -15, -15, -15, -15, -15, -15, -15, 640, 384, 384, -12, -12, -12, 
		-12, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, 
		-16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, 384, -12, 384, -12, -12, 
		-12, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, 
		-16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -12, -12, -12, -12, -12 },

/*512: COL*/
  { -12, -12, -12, -12, -12, -12, -12, -12, -12, 128, 128, -12, -12, 128, -12, -12, 
		-12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, 
		128, -12, -12, 384,1152, -12, -12, -12, 384, 384, 384, 384, 384, 896, -12, -12, 
		-15, -15, -15, -15, -15, -15, -15, -15, -15, -15, 640, 384, 384,-142, -12, -12, 
		-12, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, 
		-16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, 384, -12, 384, -12, -12, 
		-12, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, 
    -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -12, -12, -12, -12, -12 }, 

/*640: COL+MOV*/
  { -12, -12, -12, -12, -12, -12, -12, -12, -12, 128, 128, -12, -12, 128, -12, -12, 
		-12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, 
		128, -12, -12, 384,1152, -12, -12, -12, 384, 384, 384, 384, 384, 896, -12, -12, 
		-15, -15, -15, -15, -15, -15, -15, -15, -15, -15, 640, 384, 384,-142, -12, -12, 
		-12, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, 
		-16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, 384, -12, 384, -12, -12, 
		-12, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, 
    -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -12, -12, -12, -12, -12 }, 

/*768: MIN*/
  { -12, -12, -12, -12, -12, -12, -12, -12, -12, 128, 128, -12, -12, 128, -12, -12, 
		-12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, 
		128, -12, -12, 384,1152, -12, -12, -12, 384, 384, 384, 384, 384,1280, -12, -12, 
		-15, -15, -15, -15, -15, -15, -15, -15, -15, -15, 640, 384, 384, -12, -12, -12, 
		-12, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, 
		-16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, 384, -12, 384, -12, -12, 
		-12, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, 
    -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -12, -12, -12, -12, -12 }, 
    
/*896: MIN+MOV*/  
  { -12, -12, -12, -12, -12, -12, -12, -12, -12, 128, 128, -12, -12, 128, -12, -12, 
		-12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, 
		128, -12, -12, 384,1152, -12, -12, -12, 384, 384, 384, 384, 384,1280, -12, -12, 
		-15, -15, -15, -15, -15, -15, -15, -15, -15, -15, 640, 384, 384, -12, -12, -12, 
		-12, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, 
		-16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, 384, -12, 384, -12, -12, 
		-12, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, 
    -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -12, -12, -12, -12, -12 }, 

/*1024: CSH*/  
  {-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140, 
	 -140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140, 
	 -140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140, 
	 -141,-141,-141,-141,-141,-141,-141,-141,-141,-141,-140,-140,-140,-140,-140,-140, 
	 -140,-141,-141,-141,-141,-141,-141,-140,-140,-140,-140,-140,-140,-140,-140,-140, 
	 -140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140, 
	 -140,-141,-141,-141,-141,-141,-141,-140,-140,-140,-140,-140,-140,-140,-140,-140, 
   -140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140 },  
    
/*1152: CSH+MOV*/  
  {-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140, 
	 -140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140, 
	 -140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140, 
	 -141,-141,-141,-141,-141,-141,-141,-141,-141,-141,-140,-140,-140,-140,-140,-140, 
	 -140,-141,-141,-141,-141,-141,-141,-140,-140,-140,-140,-140,-140,-140,-140,-140, 
	 -140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140, 
	 -140,-141,-141,-141,-141,-141,-141,-140,-140,-140,-140,-140,-140,-140,-140,-140, 
   -140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140,-140 },  

/*1280: CMT*/ 
  {1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,   0,1280,1280,   0,1280,1280, 
   1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280, 
   1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280, 
   1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280, 
   1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280, 
   1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280, 
   1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280, 
   1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280,1280 }
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

static int32_t lex(char *input) {
  
  uint64_t cur = 0;
  char *src = input;
	const int16_t *tTable = &transitionTable[0][0];
  
  uint64_t hash = 0;
  const uint64_t hmul = hashmult;
  
  uint64_t rValue = 0;
  uint64_t lastChars = 0;
  
  int64_t state = LP_NOP_SPC;
  
  while(1) {
     
    do {
//      printf("%016llx %d\n", hash, state);
 
      rValue = (cur + hash)*hmul;
      cur = *src++;
      state = tTable[state + cur];
            
      if(state & 0x80) {
        hash = rValue;
      }
//        printf("%016llx %d %c %c\n", hash, state, esc(src[-2]), esc(cur));

    } while( state >= 0);
    
//    printf("out of loop: %x %x\n", state, state & 0xf);
    
    state &= 0xf;
    
    if(state == KWID){
      uint64_t nxt = *src++;
      rValue = cur*hashmult;
      while(eqClass[nxt] >= DGT) {
        cur = (cur << 8) + nxt;
        rValue = (rValue+nxt)*hashmult;
        nxt = *src++;
      }
     
      //those 5 bits can differentiate all keywords
      //not great but works...
      uint8_t l = (rValue >> 2) &0x1f;
      if(kwLookup[l].keyword == cur) {
        rValue = kwLookup[l].value;
      }
      
      hash = (hash + (int)rValue)*hashmult;
      
//      printf("%016llx %d KWID\n", hash, state);
      
      cur = nxt;
      //bit iffy, but we know next token can't be one that breaks loop immediately
      //like ident, keyword or decimal so can take this shortcut. at least on valid input...
      state = tTable[LP_NOP_SPC + cur]; 
    }
    else if(state == DNUM) {
      rValue = cur - 0x30;
      while( (unsigned) (cur = *src) - 0x30 < 10) {
        rValue = (rValue*10) + cur - 0x30;
        src++;
      }
      hash = (hash+((int)(rValue) ^ 0x8000))*hashmult;
      
//      printf("%016llx %d DEC\n", hash, state);
      
      //something that would break loop can come immediately after number
      //39if so can't just go to next state here
      state = LP_NOP_SPC;
    }
    else if(state == ASSN) {
      hash = (hash+RVAL_ASSIGNOP)*hashmult;
      
      state = LP_NOP_SPC;
      
//      printf("%016llx %d ASS\n", hash, state);
    }
    else if(state == HNUM) {
      rValue = hexValue(cur);
      while(isHexDigit(cur = *src++)) {
        rValue = (rValue*16) + hexValue(cur);
      }
      hash = (hash+((int)(rValue) ^ 0x4000))*hashmult;
      
//      printf("%016llx %d HEX\n", hash, state);
      
      //same applies to hex number theoretically, but doesnt occur in input?
      state = tTable[LP_NOP_SPC + cur]; 
    }
    else {
      if(cur == 0) {
        printf("%llx\n", hash);
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
  input = mmap(NULL, inputSize+1, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_POPULATE, fd, 0);
  input[inputSize] = 0; 
//  input[inputSize+1] = 0;
  
  if(input != MAP_FAILED) {
    lex(input);
  }
  else {
    fprintf(stderr, "mmap failed.\n");
    exit(1);
  }
  
  return 0;
}
