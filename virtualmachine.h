#ifndef _virtualmachine_
#define _virtualmachine_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define ERROR_INPUT 6

typedef struct quad_t Quad;

typedef unsigned char byte;

struct quad_t
{
    char *param1;
    char *param2;
    char *param3;
    char *param4;
    Quad *next;
};

typedef struct
{
    char *id_var;
    int nivel;
    byte type;
} t_variable;

typedef struct
{
    char *id_var;
    double value_numeric;
    byte type;
} t_varambiente;

extern unsigned int lenVariables;
extern t_variable *listVariables;

unsigned int lenVarambiente;
t_varambiente *listVarambiente;

void exec(Quad *lista);
void add_var(char *id, double value, byte type);
float getValue(char *lexema);
Quad *copyQuad(Quad *list);
Quad *addQuad(Quad *destine, Quad *Source);
Quad *getLabel(Quad *list, char *lexema);
Quad *genQuad(char *param1, char *param2, char *param3, char *param4);
char *genLabel();
char *genTemp();
char *removeaspas(char *str);
char *interpretaStr(char *str);
byte getType(char *lexema);
int decod_inst(char *opcode);

#endif
