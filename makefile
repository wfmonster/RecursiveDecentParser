###############################################################################
# purpose: makefile for example recursive descent parser
# created: 09-19-2017 
#  author: Joe Crumpton 
###############################################################################
LEX		=	flex
.PRECIOUS 	=	parser.cpp rules.l lexer.h 

parser.exe: lex.yy.o parser.o
	g++ -o $@ $^

lex.yy.o: lex.yy.c lexer.h
	cc -o $@ -c lex.yy.c

parser.o: parser.cpp lexer.h
	g++ -o $@ -c parser.cpp

lex.yy.c: rules.l
	$(LEX) $^

clean: 
	$(RM) *.o lex.yy.c parser.exe

