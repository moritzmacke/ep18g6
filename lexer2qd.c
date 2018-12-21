#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
//#include <sys/mman.h>
//#include <sys/stat.h>
//#include <fcntl.h>

#define __STDC_LIMIT_MACROS 1

#include <inttypes.h>

#define FALSE 0
#define TRUE 1

#define hashmult 13493690561280548289ULL

enum { RVAL_END=256, RVAL_ARRAY, RVAL_OF, RVAL_INT, RVAL_RETURN, 
       RVAL_IF, RVAL_THEN, RVAL_ELSE, RVAL_WHILE, RVAL_DO, 
       RVAL_VAR, RVAL_NOT, RVAL_OR, RVAL_ASSIGNOP };

  
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


static const int8_t letterLookup[] = { 
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255,  0,   0,   0,   0,   0,   0, 
   
	  0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,   0,   0,   0,   0,   0, 
	  0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,   0,   0,   0,   0,   0, 
  };

#define esc(_c) (_c < 32? ' ' : _c)
 
static int32_t lex(char *src) {
  

  uint64_t cur;
  
  uint64_t hash = 0;
  const uint64_t hmult = hashmult;
  
//	char *output = ""; //debug
  uint64_t rValue = 0;

	static const void *actionLookup[] = { 
    &&fail, &&fail, &&fail, &&fail, &&fail, &&fail, &&fail, &&fail, 
    &&fail, &&loop, &&loop, &&fail, &&fail, &&fail, &&fail, &&fail, 
    &&fail, &&fail, &&fail, &&fail, &&fail, &&fail, &&fail, &&fail, 
    &&fail, &&fail, &&fail, &&fail, &&fail, &&fail, &&fail, &&fail, 
    &&loop, &&fail, &&fail, &&lexc, &&cash, &&fail, &&fail, &&fail, 
    &&lexc, &&lexc, &&lexc, &&lexc, &&lexc, &&dash, &&fail, &&fail, 
    &&digt, &&digt, &&digt, &&digt, &&digt, &&digt, &&digt, &&digt, 
    &&digt, &&digt, &&coln, &&lexc, &&lexc, &&fail, &&fail, &&fail, 
    &&fail, &&altr, &&altr, &&altr, &&altr, &&altr, &&altr, &&altr, 
    &&altr, &&altr, &&altr, &&altr, &&altr, &&altr, &&altr, &&altr, 
    &&altr, &&altr, &&altr, &&altr, &&altr, &&altr, &&altr, &&altr, 
    &&altr, &&altr, &&altr, &&lexc, &&fail, &&lexc, &&fail, &&fail, 
    &&fail, &&kltr, &&altr, &&altr, &&kltr, &&kltr, &&altr, &&altr, 
    &&altr, &&kltr, &&altr, &&altr, &&altr, &&altr, &&kltr, &&kltr, 
    &&altr, &&altr, &&kltr, &&altr, &&kltr, &&altr, &&kltr, &&kltr, 
    &&altr, &&altr, &&altr, &&fail, &&fail, &&fail, &&fail, &&fail };
    

loop:
  goto *actionLookup[cur = *src++];
  
lexc:
  hash = (hash+cur)*hmult;
//    printf("%llx, LEXEM %c \n", hash, esc(cur));
  goto *actionLookup[cur = *src++];
  
coln:
  cur = *src++;
  if(cur == '=') {
    hash = (hash+RVAL_ASSIGNOP)*hashmult;
    cur = *src++;
//      printf("%llx, ASSIGN %c \n", hash, cur);
  }
  else {
    hash = (hash+':')*hashmult;
//      printf("%llx, LEXEM %c \n", hash, esc(src[-1]));
  }
  goto *actionLookup[cur];
  
kltr:
  rValue = cur * hmult;
  uint64_t lastChars = cur;
  cur = *src;
  while(letterLookup[cur] != 0) { //fast but L1 cache misses...
    rValue = (rValue+cur)*hmult;
    lastChars = (lastChars << 8) | cur;
    cur = *++src;
  }
      
  //those 5 bits can differentiate all keywords
  //not great but works...
  uint8_t l = (rValue >> 2) & 0x1f;
  if(kwLookup[l].keyword == lastChars) {
    rValue = kwLookup[l].value;
//          output = "KEYWORD";
  }
    
  hash = (hash+(int)rValue)*hmult;
//    printf("%llx, KWID %c \n", hash, esc(cur));
  src++;
  goto *actionLookup[cur];

altr:
  rValue = cur * hmult;
  cur = *src;
  while(letterLookup[cur] != 0) { //fast but L1 cache misses...
    rValue = (rValue+cur)*hmult;
    cur = *++src;
  }

  hash = (hash+(int)rValue)*hmult;
//    printf("%llx, KWID %c \n", hash, esc(cur));
  src++;
  goto *actionLookup[cur];
  

dash:
  if(*src == '-') { /*COMMENT */
     while(*++src != '\n');
  }
  else { /*LEXEMCHAR '-' */
    hash = (hash+cur)*hmult;
//    printf("%llx, LEXEM %c \n", hash, esc(cur));
  }
  goto *actionLookup[cur = *src++];
      
digt: /* DIGIT [0-9] */
//        output = "DECIMAL";
  rValue = cur - 0x30;
  uint8_t v;
  while((v = *src - 0x30) < 10) {
    rValue = (rValue*10) + v;
    src++;
  }
  rValue ^= 0x8000;
  hash = (hash+(int)rValue)*hmult;
//    printf("%llx, DECIMAL %c \n", hash, esc(cur));
  goto *actionLookup[cur = *src++];

cash: /* DOLLAR SIGN($) */
  cur = *src++;
  if(isHexDigit(cur)) { /* HEXADECIMAL */
//    output = "HEXNUM";
    rValue = hexValue(cur);
    cur = *src;
    while(isHexDigit(cur)) {
      rValue = (rValue*16) + hexValue(cur);
      cur = *++src;
    }
    rValue ^= 0x4000;
    hash = (hash+(int)rValue)*hmult;
//      printf("%llx, HEXNUM %c \n", hash, esc(cur));
    src++;
    goto *actionLookup[cur];
  }
  else {
    goto actualFail;
  }

fail: /* INVALID CHARACTER */
  if(cur == 0) { //end of file
    printf("%llx\n", hash);
    return 1;
  }
  else {
actualFail:
    fprintf(stderr, "Invalid character %c\n", cur);
    return 0;
  }

    
//      printf("%llx %s %c %c\n", hash, output, cur, nxt == '\n'? ' ' : nxt);


    
		return 1; 
}

int main(int argc, char *argv[])
{
/*
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
  input[inputSize] = '\n';
  input[inputSize+1] = 0;
  
  if(input != MAP_FAILED) {
    lex(input);
  }
  else {
    fprintf(stderr, "mmap failed.\n");
    exit(1);
  }
  */

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
  
  input[inputSize] = '\n';
  input[inputSize] = 0;
  
  fclose(in);

//  printf("%lu bytes read\n", read);
  
  lex(input);
  
  return 0;
}
