%{

  #define hashmult 13493690561280548289ULL

  enum { END=256, ARRAY, OF, INT, RETURN, IF, THEN, ELSE, WHILE, DO, VAR,
	 NOT, OR, ASSIGNOP };

  unsigned long hash(char *s);
  
  unsigned long x = 0;

%}
%Option noyywrap
%Option never-interactive


COMMENT     --.*
ID          [A-Za-z][A-Za-z0-9]*
DIGIT       [0-9]
INTEGER     {DIGIT}+
HEXDIGIT    [A-Fa-f0-9]
HEXNUM      \$({HEXDIGIT}+)
KEYWORD     end|array|of|int|return|if|then|else|while|do|var|not|or
LEXEM_CHAR  ;|"("|")"|,|:|"<"|#|"["|"]"|"-"|"+"|"*"
ASSIGNOP    :=
WHITESPACE  [\n\t ]

%%
end             x = (x+END)*hashmult;
array           x = (x+ARRAY)*hashmult;
of              x = (x+OF)*hashmult;
int             x = (x+INT)*hashmult;
return          x = (x+RETURN)*hashmult;
if              x = (x+IF)*hashmult;
then            x = (x+THEN)*hashmult;
else            x = (x+ELSE)*hashmult;
while           x = (x+WHILE)*hashmult;
do              x = (x+DO)*hashmult;
var             x = (x+VAR)*hashmult;
not             x = (x+NOT)*hashmult;
or              x = (x+OR)*hashmult;
{LEXEM_CHAR}    x = (x+yytext[0])*hashmult;
{ASSIGNOP}      x = (x+ASSIGNOP)*hashmult;
{ID}            x = (x+((int)hash(yytext)))*hashmult;
{INTEGER}       x = (x+((int)(strtoul(yytext, NULL, 10) ^ 0x8000)))*hashmult;
{HEXNUM}        x = (x+((int)(strtoul(yytext+1, NULL, 16) ^ 0x4000)))*hashmult;
{WHITESPACE}+   ;
{COMMENT}       ;
.               printf("Lexical error. Unrecognised input \"%s\"\n", yytext); return 0;
<<EOF>>		      return 1;
%%



unsigned long hash(char *s)
{
  unsigned long r=0;
  char *p;
  for (p=s; *p; p++)
    r = (r+*p)*hashmult;
  return r;
}

int main(int argc, char *argv[])
{

  if (argc != 2) {
    fprintf(stderr,"Usage: %s <file>\n",argv[0]);
    exit(1);
  }
  FILE *in = fopen(argv[1],"r");
  if (in == NULL) {
    perror(argv[1]);
    exit(1);
  }

  //TODO error checking
  fseek(in, 0, SEEK_END);
  size_t inputSize = ftello(in)+2;
  fseek(in, 0, SEEK_SET);
  
//  inputSize = 10000;
  
  char *input = malloc(inputSize);
  
  size_t read = fread(input, 1, inputSize, in);
  
  //two zero byts at end
  input[inputSize-2] = input[inputSize-1] = 0;
  fclose(in);
  
  yy_scan_buffer(input, inputSize);
  
  x = 0;
  yylex();
  
  printf("%lx\n",x);
  
  return 0;
}
