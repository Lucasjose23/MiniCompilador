#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#define MAXNAME 30
#define MAXNUM 5

char look;
char var;
int labelCount;


void program();
void block();
int newlabel();
int postLabel(int lbl);
void doIf();
void condition();
void skipWhite();

void init();
void nextchar();
void error(char *fmt,...);
void fatal(char *fmt,...);
void expected(char *fmt,...);
void match(char c);
void getName(char *name);
void getNum(char *num);
void emit(char *fmt,...); 
void expression(); // Simbolo não terminal (Procedimento)
void add();
void subtract();
void mult();
void div();
void factor();
void term();
void assigment();
int isBoolean(char c);
int getBoolean();
void boolOr();
void boolXor();
int isOrOp(char c);
void boolExpression();
void boolTerm();
void boolFactor();
void notFactor();
int isRelOp(char c);
void relation();
void equals();
void notEquals();
void doWhile();

void assigment()
{

		//var=getName();
		//match('=');
		//expression();
		//boolExpression();
	//	emit("MOV [%c], AX", var);
	char name[MAXNAME+1];
 	getName(name);
	 match('=');
	 expression();
	 emit("MOV [%s], AX", name);
	
}

void program()
{
	block();
	if(look != 'e')
	{
		expected ("End");
	}
	emit ("END");
}

void block()
{

	while(look!='e' && look!='l')
	{
		switch(look)
		{
			case 'i':
				doIf(); 
				break;
			 case 'w':
                doWhile();
                break;	
			default:
				assigment();
				break; 
		}
	}
}

int newlabel()
{
	return labelCount++;
}		 

int postLabel(int lbl)
{
	printf("L%d:\n", lbl);
}

void doIf()
{
	int l1,l2;
	match('i');
	//condition()
	boolExpression();
	l1=newlabel();
	l2=l1;
	emit ("JZ L%d", l1);
	block();
	/*ELSE*/
	if(look=='l')
	{
		match('l');
		l2=newlabel();
		emit("JMP L%d", l2);
		postLabel(l1);
		block();
	}
	match('e');
	postLabel(l2);
}

void condition()
{
	emit("# contition");
}

void skipWhite()
{
	if(look==' '||look=='\t')
	{
		nextchar();
	}
}
int isBoolean(char c)
{
	return(c=='T'||c=='F');
}
int getBoolean(){//VERIFICA SE ELE É UM BOOLEAN
	int boolean;
	if(!isBoolean(look))
		expected("Literal Boolean");
	boolean=(look=='T');//RETORNA SO SE FOR TRUE
	nextchar();
	return boolean;	
}

void boolOr(){//ESCREVE O ASSEMBRI DO OR
	match('|');
	boolTerm();
	emit("POP BX");
	emit("OR AX, BX");
	
}
void boolXor(){//ESCREVE O ASSEMBRI DO XOR
	match('~');
	boolTerm();
	emit("POP BX");
	emit("XOR AX, BX");
}

int isOrOp(char c){//RETORNA OR OU XOR
	return(c=='|'|| c=='~');
}

void boolExpression(){//CHAMA TERMO E VERIFICA OR E XOR
	boolTerm();
	while(isOrOp(look)){
		emit("PUSH AX");
		switch(look){
			case '|':
				boolOr();
				break;
			case '~':
				boolXor();
				break;	
		}
	}
}

void boolTerm(){
	notFactor();
	while(look=='&'){
		emit("PUSH AX");
		match('&');
		notFactor();
		emit("POP BX");
		emit("AND AX, BX");
	}
}

void boolFactor(){
	if(isBoolean(look)){
		if(getBoolean())
			emit("MOV AX, -1");
		else
			emit("MOV AX, 0");
	}else
		relation();
}

void notFactor(){//VERIFICA A QUESTAO DA NEGAÇAO
	if(look=='!'){
		match('!');
		boolFactor();
		emit("NOT AX");
		
	}else
		boolFactor();
}
int isRelOp(char c){//RETORNA QUAL É O SIMBOLO RELACIONAL
	return(c=='='||c=='#');
}

void relation(){//VERIFICA O SIMBOLO RELACIONAL 
	expression();
	if(isRelOp(look)){
		emit("PUSH AX");
		switch(look){
			case '=':
				equals();
				break;
			case '#':
				notEquals();
				break;
		}
	}
}

void equals(){//ESCREVE O ASSEMBRER DO IGUAL
	int l1,l2;
	match('=');
	l1=newlabel();
	l2=newlabel();
	expression();
	emit("POP BX");
	emit("CMP BX, AX");
	emit("JE L%d", l1);
	emit("MOV AX, 0");
	emit("JMP L%d", l2);
	postLabel(l1);
	emit("MOV AX, -1");
	postLabel(l2);
}
void notEquals()
{
        int l1, l2;

        match('#');
        l1 = newlabel();
        l2 = newlabel();
        expression();
        emit("POP BX");
        emit("CMP BX, AX");
        emit("JNE L%d", l1);
        emit("MOV AX, 0");
        emit("JMP L%d", l2);
        postLabel(l1);
        emit("MOV AX, -1");
        postLabel(l2);
}

void doWhile()
{
        int l1, l2;

        match('w');
        l1 = newlabel();
        l2 = newlabel();
        postLabel(l1);
        boolExpression();
        emit("JZ L%d", l2);
        block();
        match('e');
        emit("JMP L%d", l1);
        postLabel(l2);
}
void ident()
{
        char name[MAXNAME+1];
        getName(name);
        if (look == '(') {
                match('(');
                match(')');
                emit("CALL %s", name);
        } else
                emit("MOV AX, [%s]", name);
}
		

int main()
{
	init();
	program();
	return 0;
}

void init()
{
	labelCount = 0;
	nextchar();
	 skipWhite();
}

void nextchar()//COMEÇA A RECEBER O CODIGO DIGITADO
{
	look = getchar();
}

void error(char *fmt,...)
{
	va_list args;
	fputs("Error:", stderr);
	va_start(args,fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	
	fputc('\n', stderr);
}

void fatal(char *fmt,...)
{
	va_list args;
	fputs("Error:", stderr);
	va_start(args,fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	
	fputc('\n', stderr);
	
	exit(1);
}

void expected(char *fmt,...)
{
	va_list args;
	fputs("Error:", stderr);
	va_start(args,fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	
	fputs("expected! \n", stderr);
	
	exit(1);
}

void match(char c)//PULA O SINAL
{
	skipWhite();
	if(look!=c)
	{
		expected("'%c'",c);
	}
	nextchar();
	skipWhite();
}

/*char getName()
{
	skipWhite();
	char name;
	if(!isalpha(look))
	{
		expected("Name");
	}
	name = toupper(look);
	nextchar();
	skipWhite();
	return name;
}*/
void getName(char *name)
{
  int i;
 if (!isalpha(look))
  expected("Name");
 for (i = 0; isalnum(look); i++) {
  if (i >= MAXNAME)
   fatal("Identifier too long!");
  name[i] = toupper(look);
  nextchar();
 }
 name[i] = '\0';
  skipWhite();
}

/*char getNum()//VERIFICA SE É UM NUMERO
{
	skipWhite();
	char num;
	if(!isdigit(look))
	{
		expected("Interger");
	}
	num = look;
	nextchar();
	skipWhite();
	return num;
}*/
void getNum(char *num)
{
        int i;
 if (!isdigit(look))
  expected("Integer");
 for (i = 0; isdigit(look); i++) {
  if (i >= MAXNUM)
   fatal("Integer too long!");
  num[i] = look;
  nextchar();
 }
 num[i] = '\0';
  skipWhite();
}

void emit(char *fmt,...)// GERADOR DE CODIGO OBJETO
{
	va_list args;
	putchar('\t');
	
	va_start (args, fmt);
	vprintf(fmt, args);
	va_end(args);
	
	putchar('\n');
	
}

void factor()//0|1|..|9 ESCREVE O CODIGO OBJETO DO NUM
{
	/*if(look=='(') //<factor>:=(<expression>)|<num>|<variable>]*
	{
		match('(');
		//expression();
		boolExpression();
		match(')');
	} else if(isalpha(look))
	{
		emit("MOV AX, [%c]", getName());
	}else
		emit("MOV AX, %c", getNum());*/
	char num[MAXNUM+1];
    if (look == '(') {
                match('(');
                expression();
                match(')');
        } else if(isalpha(look)) {
                ident();
 } else {
                getNum(num);
         emit("MOV AX, %s", num);
        }	
		
	
}

void expression()
{
	term();//expression:=<term>[<addop><term>]*
	while(look == '+' || look == '-')//ANALIZA A OPERAÇAO
	{
		//emit("MOV BX, AX");
		emit("PUSH AX");//GUARDA NA PILHA
		switch(look)
		{
			case '+':
				add();
				break;
			case '-':
				subtract();
				break;
				
			default:
				expected("AddOp");
				break;
		}
	}
	//addop//
	//term();//OLHA SE É UM TERMO
}
void term()
{
	factor();// <term>:=<factor>[<mulop><factor>]*  
	while(look == '*' || look == '/')//ANALIZA A OPERAÇAO
	{
		//emit("MOV BX, AX");
		emit("PUSH AX");//GUARDA NA PILHA
		switch(look)
		{
			case '*':
				mult();
				break;
			case '/':
				div();
				break;
				
			default:
				expected("mulOp");
				break;
		}
	}
	
}
void add()
{
	match('+');
	term();
	emit("POP BX");//TIRA OQ TA NA PILHA E SALVA EM BX
	emit("ADD AX, BX");
}
void subtract()
{
	match('-');
	term();
	emit("POP BX");
	emit("SUB, AX, BX");
	emit("NEG AX");
}
void mult()
{
	match('*');
	factor();
	emit("POP BX");//TIRA OQ TA NA PILHA E SALVA EM BX
	emit("IMUL BX");
}
void div()
{
	match('/');
	factor();
	emit("POP BX");//TIRA OQ TA NA PILHA E SALVA EM BX
	emit("XCHG AX,BX");
	emit("CWD");
	emit("IDIV BX");
}



