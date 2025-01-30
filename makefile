CC = gcc

MYFLAGS = -g -O2 -Wall -Wno-char-subscripts

TARGET = interpretador

OBJFILES = lexico.o main.o parser.o virtualmachine.o

default: all

all:
	$(MAKE) $(TARGET)

# regras para gerar o executavel
$(TARGET): $(OBJFILES) 
	$(CC) -o $(TARGET) $(OBJFILES) 

# regras de compilação
lexico.o: lexico.c lexico.h
	$(CC) -c $(MYFLAGS) lexico.c  

main.o: main.c parser.h lexico.h virtualmachine.h
	$(CC) -c $(MYFLAGS) main.c  

parser.o: parser.c parser.h lexico.h virtualmachine.h
	$(CC) -c $(MYFLAGS) parser.c  

virtualmachine.o: virtualmachine.c virtualmachine.h
	$(CC) -c $(MYFLAGS) virtualmachine.c  

clean:
	rm -f *.o
	rm -f $(TARGET)

clear:
	rm -f *.o