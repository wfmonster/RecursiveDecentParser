//*****************************************************************************
// purpose: recursive descent parser for BullyC
// created: 
//  author: 
//*****************************************************************************
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

#include "lexer.h"

extern "C"
{
// Instantiate global variables
extern FILE *yyin;         // input stream
extern FILE *yyout;        // output stream
extern int   yyleng;       // length of current lexeme
extern char *yytext;       // text of current lexeme
extern int   yylex();      // the generated lexical analyzer
}

// Production functions
void program( int );
void statement( int );
void assignment(int );
void expr( int );
void boolean( int );
void relation ( int );
void term( int );
void factor( int );
void unary( int );
void get( int );
void output( int );
void conditional_expr( int );
void while_loop( int );

// Function declarations for checking whether the current token is
// in the first set of each production rule.
bool isFirstOfProgram( void );
bool isFirstOfStatement( void );
bool isFirstOfAssignment( void );
bool isFirstOfExpr( void );
bool isFirstOfBoolean( void );
bool isFirstOfRelation (void);
bool isFirstOfTerm( void );
bool isFirstOfFactor( void );
bool isFirstOfUnary( void );
bool isFirstOfGet ( void );
bool isFirstOfOutput( void );
bool isFirstOfConditional( void );
bool isFirstOfWhile( void );


// Function to help print the tree nesting
string spaces( int );

// Needed global variables
int currentToken;      
vector <string> variables;
bool duplicate = 0;
bool exists = 0;

//*****************************************************************************
// The main processing loop
int main( int argc, char* argv[] )
{
  // Set the input and output streams
  yyin = stdin;
  yyout = stdout;

  // Get the first token
  currentToken = yylex();
  
  // Fire up the parser!
  try {
  	if(!isFirstOfProgram())
  		throw "First token is not '{'' ";

  	program(0);

  	if(currentToken != TOK_EOF)
  		throw "end of file expected, but there is more here!";
  }
  catch (char const *errmsg) {
  	cerr << endl << "***ERROR: " << errmsg << endl;
  	return 1;
  }
  cout << endl << "=== GO BULLDOGS! Your parse was successful! ===" << endl;

  cout << "\nUser Defined Symbols:" << endl;
  for(int i=0; i < variables.size(); i++) { 
  	cout << variables[i] << '\n'; 
  }

  return 0;
}


/*****************************************************************************/
// <program> --> { {<statement>} } 
void program( int level ){  
	static int programCount = 0;
	int currentProgramCount = ++programCount;

	char const *Perr = "<statement> does not start with let | read | print | if | while";
	cout << spaces(level) << "enter <program> " << currentProgramCount << endl;

	cout << spaces(level) << "-->found " << yytext << endl;
	currentToken = yylex();

	if (isFirstOfStatement()){
		while (currentToken != TOK_EOF){
			statement( level + 1 );

			if (currentToken == TOK_CLOSEBRACE){
				cout << spaces(level) << "-->found " << yytext << endl;
				currentToken = yylex();
				break;
			}
		}
	} else { throw Perr; }

	cout << spaces(level) << "exit <program> " << currentProgramCount << endl;
}

/*****************************************************************************/
// <statement> --> <assignment> | <get> | <output> | <conditional> | <while>
// first of --> let, read, print, if, while
void statement( int level ){ 
	static int statementCount = 0;
	int currentStatementCount = ++statementCount;

	cout << spaces( level ) << "enter <statement> " << currentStatementCount << endl;

	// determine what token we have
	switch(currentToken){
		case TOK_LET:
			if(!isFirstOfAssignment()) throw "First token is not 'let'";
			assignment( level + 1 );
			break;

		case TOK_READ:
			if(!isFirstOfGet()) throw "First token is not 'read'";
			get( level + 1 );
			break;

		case TOK_PRINT:
			if(!isFirstOfOutput()) throw "First token is not 'print'";
			output( level + 1 );
			break;

		case TOK_IF:
			if(!isFirstOfConditional()) throw "First token is not 'if'";
			conditional_expr( level + 1 );
			break;

		case TOK_WHILE:
			if(!isFirstOfWhile()) throw "First token is not 'read'";
			while_loop( level + 1 );
			break;

		default:
			throw "<statement> does not start with let | read | print | if | while";
	}
	cout << spaces( level ) << "exit <statement> " << currentStatementCount << endl;
}

/*****************************************************************************/
// <assignment> --> let ID := <expression> ; 
void assignment( int level ) { 
	// TODO: we need to keep track of the variables that have been assigned to!!

	static int assignmentCount = 0;
	int currentAssignmentCount = ++assignmentCount;

	cout << spaces(level) << "enter <assignment> " << assignmentCount << endl;
	cout << spaces( level) << "-->found " << yytext << endl;
	currentToken = yylex();

	if (currentToken == TOK_IDENTIFIER){
		cout << spaces( level) << "-->found ID: " << yytext << endl;

		for(int i = 0; i < variables.size(); i++){
			if (variables[i] == yytext) duplicate = 1;
		}
		if(duplicate == 0) variables.push_back(yytext);  // remember what variables have been assigned 
		currentToken = yylex();

		if(currentToken == TOK_ASSIGN){
			cout << spaces(level) << "-->found " << yytext << endl;
			currentToken = yylex();

			if(isFirstOfExpr()){
				expr(level +1);

				if (currentToken==TOK_SEMICOLON){
					cout << spaces( level) << "-->found " << yytext << endl;
					currentToken = yylex();
				} else {
					throw "<assignment> should end with ';'";
				}
			} else {
				throw "<Expression> does not start with 'not' | '-' | '(' | 'ID' | INTLIT";
			}
		} else {
			throw "missing ':=' in assignment statement";
		}
	} else {
		throw "let assigment requires and identifier";
	}

	cout << spaces (level) << "exit <assignment> " << currentAssignmentCount << endl;
}

/*****************************************************************************/
// <expression> --> <boolean> {( and | or ) <boolean> }
void expr( int level ){
	static int exprCount = 0;
	int currentExprCount = ++exprCount;

	char const *Eerr = "<boolean> does not start with 'not' | '-' | '(' | 'ID' | INTLIT";
	cout << spaces(level) << "enter <expression> " << currentExprCount << endl;

	// we expect to see a <boolean>
	if(isFirstOfBoolean()){ boolean( level + 1); }
	else { throw Eerr; }

	// as long as the next token is and | or keep parsing <boolean>'s'
	while(currentToken == TOK_AND || currentToken == TOK_OR){
		cout << spaces( level ) << "-->found " << yytext << endl;
		currentToken = yylex();
		if(isFirstOfBoolean()){
			boolean( level + 1 );
		} else {
			throw Eerr;
		}
	}
	cout << spaces(level) << "exit <expression> " << currentExprCount << endl;
}

/*****************************************************************************/
// <boolean> --> <relation> [( < | > | == ) <relation> ]
void boolean( int level ){
	static int boolCount = 0;
	int currentBoolCount = ++boolCount;

	char const *Berr = "<relation> does not start with 'not' | '-' | '(' | 'ID' | INTLIT";
	cout << spaces( level ) << "enter <boolean> " << currentBoolCount << endl;

	// we expect to see a <relation>
	if(isFirstOfRelation()) { relation( level + 1); }
	else { throw Berr; }
	
	// Continue parsing <relation>'s' if we have  <, > or == 
	if (currentToken == TOK_LESSTHAN || currentToken == TOK_GREATERTHAN || currentToken == TOK_EQUALTO ){
		cout << spaces( level ) << "-->found " << yytext << endl;
		currentToken = yylex();

		if(isFirstOfRelation()) { relation(level + 1); }
		else{ throw Berr; }
	}
	cout << spaces(level) << "exit <boolean> " << currentBoolCount << endl;
}

/*****************************************************************************/
// <relation> --> <term> {( + | - ) <term> }  (same as expr in example)
void relation (int level ){
	static int relationCount = 0;
	int currentRelationCount = ++relationCount;

	char const *Rerr  = "<term> does not start with 'not' | '-' | '(' | 'ID' | INTLIT";
	cout << spaces(level) << "enter <relation> " << currentRelationCount << endl;

	// we expect to see a <term>
	if (isFirstOfTerm()){ term( level + 1); } 
	else { throw Rerr; }

	// As long as the next token in + or - keep parsing <term>'s'
	while (currentToken == TOK_PLUS || currentToken == TOK_MINUS){
		cout << spaces( level ) << "-->found " << yytext << endl;
		currentToken = yylex();

		if( isFirstOfTerm() ){ term(level + 1); }
		else { throw Rerr; }
	}

	cout << spaces(level) << "exit <relation> " << currentRelationCount << endl;
}

//*****************************************************************************
// <term> --> <factor> {( * | / ) <factor> } (same as term un example)
void term( int level ){ 
	static int termCount = 0;
	int currentTermCount = ++termCount;

	char const *Terr = "<factor> does not start with 'not' | '-' | '(' | 'ID' | INTLIT";
	cout << spaces( level ) << "enter <term> " << currentTermCount << endl;

	// We next expect to see a <factor>
	if (isFirstOfFactor()){ factor(level + 1); }
	else { throw Terr; }

	// As long as the next token is * or /, keep parsing <factor>'s'
	while( currentToken == TOK_MULTIPLY || currentToken == TOK_DIVIDE){
		cout << spaces( level ) << "-->found " << yytext << endl;
		currentToken = yylex();

		if(isFirstOfFactor()){ factor( level + 1); }
		else { throw Terr; }
	}
	cout << spaces( level ) << "exit <term> " << currentTermCount << endl;
}


//*****************************************************************************
// <factor> --> [ not | - ] <unary>
void factor( int level ){ 
	static int factorCount = 0; 
	int currentFactorCount = ++factorCount;

	char const *Ferr = "<Unary> does not start with '(' | 'ID' | INTLIT";
	cout << spaces(level) << "enter <factor> " << currentFactorCount << endl;

	if ( currentToken == TOK_NOT || currentToken == TOK_MINUS){
		cout << spaces(level) << "-->found" << yytext << endl;
		currentToken = yylex();

		if(isFirstOfUnary()){ unary( level + 1); }
		else { throw Ferr; }

	} else {
		if(isFirstOfUnary()){ unary( level + 1); }
		else { throw Ferr; }
	}
	cout << spaces(level) << "exit <factor> " << currentFactorCount << endl;
}


//*****************************************************************************
// <unary> --> ID | INTLIT | ( <expression> ) 
void unary( int level ){
	static int unaryCount = 0;
	int currentUnaryCount = ++unaryCount;

	cout << spaces(level) << "enter <unary> " << currentUnaryCount << endl;

	// determine what token we have
	switch(currentToken){
		case TOK_IDENTIFIER:
			cout << spaces( level ) << "-->found ID: " << yytext << endl;
			currentToken = yylex();
			break;

		case TOK_INTLIT:
			cout << spaces( level ) << "-->found INTLIT: " << yytext << endl;
    		currentToken = yylex();
			break;

		case TOK_OPENPAREN:
			// we expect ( <expr> )
			cout << spaces( level ) << "-->found ( " << endl;
			currentToken = yylex();
			if( !isFirstOfExpr()) {
				throw "first token not 'not' | '-' | '(' | 'ID' | INTLIT";
			}
			expr(level + 1);

			if(currentToken == TOK_CLOSEPAREN){
				cout << spaces( level ) << "-->found ) " << endl;
		    	currentToken = yylex();
		    } else{
		      throw "<expr> does not end with )";
		    }
			break;

		default:
			// There must be a syntax error 
			throw "<unary> does not start with 'ID' | 'INTLIT' | '('";
	}
	cout << spaces( level ) << "exit <unary> " << currentUnaryCount << endl;
}

//*****************************************************************************
// <get> --> read [ STRINGLIT ] ID;
void get( int level ){ 
	static int getCount = 0;
	int currentGetCount = ++getCount;

	cout << spaces(level) << "enter <get> " << currentGetCount << endl;
	cout << spaces( level) << "-->found " << yytext << endl;
	currentToken = yylex();

	if (currentToken== TOK_STRINGLIT) {
		cout << spaces( level ) << "-->found STRINGLIT: " << yytext << endl;
		currentToken = yylex();

		if (currentToken == TOK_IDENTIFIER){
			cout << spaces( level ) << "-->found ID: " << yytext << endl;

			// Check if the variable we are reading exists
			for(int i = 0; i <= variables.size(); i++){
				if (variables[i] == yytext) { 
					exists = 1;
					break; 
				} 
			}
			if (!exists) { throw "uninitialized identifier used in read statement"; } 
			currentToken = yylex();

			if (currentToken == TOK_SEMICOLON){
				cout << spaces( level) << "-->found " << yytext << endl;
			} else { throw "Expected a ';'";}
		}

	} else if (currentToken == TOK_IDENTIFIER){

		cout << spaces( level ) << "-->found ID: " << yytext << endl;

		// Check if the variable we are reading exists
		// Check if the variable we are reading exists
		for(int i = 0; i <= variables.size(); i++){
			if (variables[i] == yytext) { 
				exists = 1;
				break; 
			} 
		}
		if (!exists) { throw "uninitialized identifier used in read statement"; } 
		currentToken = yylex();

		if (currentToken == TOK_SEMICOLON){
			cout << spaces(level) << "-->found " << yytext << endl;
		} else { throw "Expected a ';'"; }

	} else { throw "Expected an ID or STRINGLIT"; }

	cout << spaces(level) << "exit <get> " << currentGetCount << endl;
}
	
//*****************************************************************************
// <output> --> print [ STRINGLIT ] [ ID ] ;
void output( int level ){ 
	static int outputCount = 0;
	int currentOutputCount = ++outputCount;

	cout << spaces(level) << "enter <output> " << currentOutputCount << endl;

	cout << spaces( level) << "-->found " << yytext << endl;
	currentToken = yylex();

	if (currentToken == TOK_STRINGLIT){
		cout << spaces( level ) << "-->found STRINGLIT: " << yytext << endl;
		currentToken = yylex();

		if (currentToken == TOK_IDENTIFIER){
			cout << spaces( level ) << "-->found ID: " << yytext << endl;
			// Check if the variable we are reading exists
			for(int i = 0; i <= variables.size(); i++){
				if (variables[i] == yytext) { 
					exists = 1;
					break; 
				} 
			}
			if (!exists) { throw "uninitialized identifier used in output statement"; } 
			currentToken = yylex();
		}

	} else if ( currentToken == TOK_IDENTIFIER) {
		cout << spaces( level ) << "-->found ID: " << yytext << endl;
		// Check if the variable we are reading exists
		for(int i = 0; i <= variables.size(); i++){
			if (variables[i] == yytext) { 
					exists = 1;
					break; 
			} 
		}
		if (!exists) { throw "uninitialized identifier used in output statement"; }
		currentToken = yylex();

	} else { throw "Expected a STRINGLIT or an IDENTIFIER"; }
	exists = 0; // reset exists variable.

	if (currentToken == TOK_SEMICOLON){
		cout << spaces( level) << "-->found " << yytext << endl;
		currentToken = yylex();

	} else { throw "Expected a ';'"; }

	cout << spaces(level) << "exit <output> " << currentOutputCount << endl;
}

//*****************************************************************************
// <conditional> --> if ( <expression> ) <program> [ else <program> ]
void conditional_expr( int level ){
	static int conditionalCount = 0;
	int currentConditionalCount = ++conditionalCount;

	cout << spaces(level) << "enter <conditional> " << currentConditionalCount << endl;

	cout << spaces( level) << "-->found " << yytext << endl;  // should be if
	currentToken = yylex();

	if (currentToken == TOK_OPENPAREN){
		cout << spaces( level ) << "-->found (" << endl;
		currentToken = yylex();

		if( !isFirstOfExpr()) {
			throw "first token not 'not' | '-' | '(' | 'ID' | INTLIT";
		}
		expr(level + 1);

		if(currentToken == TOK_CLOSEPAREN){
			cout << spaces( level ) << "-->found )" << endl;   
	    	currentToken = yylex();
	    } else { throw "<expr> does not end with )"; }

	    if (isFirstOfProgram()){
			program(level + 1);

			if (currentToken == TOK_ELSE){
					cout << spaces( level) << "-->found " << yytext << endl; // should be else
					currentToken = yylex();

					if(isFirstOfProgram()){ program(level + 1); }
					else { throw "First of <program> token is not {"; }
			}
		} else { throw "First token not '{'"; }
	}
	cout << spaces(level) << "exit <conditional> " << currentConditionalCount << endl;
}

//*****************************************************************************
// <while> --> while ( <expression> ) < program >
void while_loop( int level ){
	static int whileCount = 0;
	int currentWhileCount = ++whileCount;

	cout << spaces(level) << "enter <while> " << currentWhileCount << endl;
	cout << spaces( level) << "-->found " << yytext << endl; 
	currentToken = yylex();

	if (currentToken == TOK_OPENPAREN){
		cout << spaces( level ) << "-->found (" << endl;
		currentToken = yylex();

		if( !isFirstOfExpr()) {
			throw "first token not 'not' | '-' | '(' | 'ID' | INTLIT";
		}
		expr(level + 1);

		if(currentToken == TOK_CLOSEPAREN){
			cout << spaces( level ) << "-->found )" << endl;   
	    	currentToken = yylex();
	    } else { throw "<expr> does not end with )"; }

	    if (isFirstOfProgram()){ program(level + 1); }
	    else { throw "First token not '{'"; }
	}
	cout << spaces(level) << "exit <while> " << currentWhileCount << endl;
}


//*****************************************************************************
// CHECKING IF TOKEN IS IN FIRST SET OF EACH PRODUCTION RULE
//*****************************************************************************
// First Token Set --> { { }
bool isFirstOfProgram( void ){
	return currentToken == TOK_OPENBRACE;
}

//*****************************************************************************
// First Token Set --> { let, read, print, if, while }
bool isFirstOfStatement( void ){
	return currentToken == TOK_LET ||
		   currentToken == TOK_READ ||
		   currentToken == TOK_PRINT ||
		   currentToken == TOK_IF ||
		   currentToken == TOK_WHILE;
}

//*****************************************************************************
// First Token Set --> { let } 
bool isFirstOfAssignment( void ){
	return currentToken == TOK_LET;
}

//*****************************************************************************
// First Token Set -->  { not, -, (, ID, INTLIT } 
bool isFirstOfExpr( void ){
	return currentToken == TOK_NOT ||
		   currentToken == TOK_MINUS ||
		   currentToken == TOK_IDENTIFIER ||
		   currentToken == TOK_INTLIT ||
		   currentToken == TOK_OPENPAREN;
}

//*****************************************************************************
// First Token Set --> { not, -, (, ID, INTLIT }
bool isFirstOfBoolean( void ){
	return currentToken == TOK_NOT ||
		   currentToken == TOK_MINUS ||
		   currentToken == TOK_IDENTIFIER ||
		   currentToken == TOK_INTLIT ||
		   currentToken == TOK_OPENPAREN;
}

//*****************************************************************************
// First Token Set --> { not, -, (, ID, INTLIT } 
bool isFirstOfRelation( void ){
	return currentToken == TOK_NOT ||
		   currentToken == TOK_MINUS ||
		   currentToken == TOK_IDENTIFIER ||
		   currentToken == TOK_INTLIT ||
		   currentToken == TOK_OPENPAREN;
}

//*****************************************************************************
// First Token Set --> { not, -, (, ID, INTLIT } 
bool isFirstOfTerm( void ){
	return currentToken == TOK_NOT ||
		   currentToken == TOK_MINUS ||
		   currentToken == TOK_IDENTIFIER ||
		   currentToken == TOK_INTLIT ||
		   currentToken == TOK_OPENPAREN;
}

//*****************************************************************************
// First Token Set --> { not, -, (, ID, INTLIT } 
bool isFirstOfFactor( void ){
	return currentToken == TOK_NOT ||
		   currentToken == TOK_MINUS ||
		   currentToken == TOK_IDENTIFIER ||
		   currentToken == TOK_INTLIT ||
		   currentToken == TOK_OPENPAREN;
}

//*****************************************************************************
// First Token Set --> { (, ID, INTLIT } 
bool isFirstOfUnary( void ){ 
	return currentToken == TOK_IDENTIFIER ||
		   currentToken == TOK_INTLIT ||
		   currentToken == TOK_OPENPAREN;
}

//*****************************************************************************
// First Token Set --> { read } 
bool isFirstOfGet( void ){
	return currentToken == TOK_READ;
}

//*****************************************************************************
// First Token Set --> { print } 
bool isFirstOfOutput( void ){
	return currentToken == TOK_PRINT;
}

//*****************************************************************************
// First Token Set --> { if } 
bool isFirstOfConditional( void ){ 
	return currentToken == TOK_IF;
}

//*****************************************************************************
// First Token Set --> { while } 
bool isFirstOfWhile( void ){ 
	return currentToken == TOK_WHILE;
}

//*****************************************************************************

string spaces(int n){
	string str( n*2 , ' ');
	return str;
}

























