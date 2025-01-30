#include "virtualmachine.h"

unsigned int lenVariables = 0;
t_variable *listVariables = NULL;

void error_alloc(char *var, char *func);

void flush_in()
{
    int ch;

    while ((ch = getchar()) != EOF && ch != '\n')
        ;
}

char *genLabel()
{
    static unsigned int inc = 0;
    char *aux = (char *)malloc(sizeof(char) * (strlen("_l") + 7));
    if (!aux)
        error_alloc("aux", "genLabel");
    sprintf(aux, "%s%d", "_l", inc++);
    return aux;
}
char *genTemp()
{
    static unsigned int inc = 0;
    char *aux = (char *)malloc(sizeof(char) * (strlen("_t") + 7));
    if (!aux)
        error_alloc("aux", "genTemp");
    sprintf(aux, "%s%d", "_t", inc++);
    return aux;
}
Quad *genQuad(char *param1, char *param2, char *param3, char *param4)
{
    Quad *aux = (Quad *)malloc(sizeof(Quad));
    if (!aux)
        error_alloc("aux", "genQuad");
    aux->param1 = param1;
    aux->param2 = param2;
    aux->param3 = param3;
    aux->param4 = param4;
    aux->next = NULL;
    return aux;
}
Quad *addQuad(Quad *destine, Quad *source)
{
    Quad *aux = destine;
    if (aux)
    {
        while (aux->next)
            aux = aux->next;

        if (source)
        {
            aux->next = (Quad *)malloc(sizeof(Quad *));
            if (!aux->next)
                error_alloc("aux->next", "addQuad");
            aux->next = source;
        }
        return destine;
    }
    return source;
}

Quad *copyQuad(Quad *list)
{
    Quad *aux3, *aux2, *aux = (Quad *)malloc(sizeof(Quad));
    if (!aux)
        error_alloc("aux", "copyQuad");
    aux2 = list;
    aux3 = aux;
    while (aux2)
    {
        aux3->param1 = (char *)malloc(sizeof(char) * 6);
        if (!aux3->param1)
            error_alloc("aux3->param1", "copyQuad");
        strcpy(aux3->param1, aux2->param1);
        if (aux2->param2)
        {
            aux3->param2 = (char *)malloc(sizeof(char) * 6);
            if (!aux3->param2)
                error_alloc("aux3->param2", "copyQuad");
            strcpy(aux3->param2, aux2->param2);
        }
        else
            aux3->param2 = NULL;

        if (aux2->param3)
        {
            aux3->param3 = (char *)malloc(sizeof(char) * 256);
            if (!aux3->param3)
                error_alloc("aux3->param3", "copyQuad");
            strcpy(aux3->param3, aux2->param3);
        }
        else
            aux3->param3 = NULL;

        if (aux2->param4)
        {
            aux3->param4 = (char *)malloc(sizeof(char) * 256);
            if (!aux3->param4)
                error_alloc("aux3->param4", "copyQuad");
            strcpy(aux3->param4, aux2->param4);
        }
        else
            aux3->param4 = NULL;

        aux3->next = NULL;
        aux2 = aux2->next;
        if (aux2)
        {
            aux3->next = (Quad *)malloc(sizeof(Quad));
            if (!aux3->next)
                error_alloc("aux3->next", "copyQuad");
            aux3 = aux3->next;
        }
    }
    return aux;
}

void exec(Quad *lista)
{
    unsigned short int type, op;
    float valor;
    char str[256];
    for (Quad *aux = lista; aux; aux = aux->next)
    {
        op = decod_inst(aux->param1);
        switch (op)
        {
        case 0:
            if (strcmp(aux->param3, (char *)"V") == 0)
            {
                valor = getValue(aux->param2);
                if (valor == 0)
                    add_var(aux->param2, 0, atoi(aux->param4));

                continue;
            }
            type = getType(aux->param2);
            valor = getValue(aux->param3);
            add_var(aux->param2, valor, type);
            break;

        case 1:
            valor = getValue(aux->param3) == getValue(aux->param4);
            type = getType(aux->param2);
            add_var(aux->param2, valor, type);
            break;

        case 2:
            valor = getValue(aux->param3) > getValue(aux->param4);
            type = getType(aux->param2);
            add_var(aux->param2, valor, type);
            break;

        case 3:
            valor = getValue(aux->param3) >= getValue(aux->param4);
            type = getType(aux->param2);
            add_var(aux->param2, valor, type);
            break;

        case 4:
            valor = getValue(aux->param3) < getValue(aux->param4);
            type = getType(aux->param2);
            add_var(aux->param2, valor, type);
            break;

        case 5:
            valor = getValue(aux->param3) <= getValue(aux->param4);
            type = getType(aux->param2);
            add_var(aux->param2, valor, type);
            break;

        case 6:
            valor = getValue(aux->param3) != getValue(aux->param4);
            type = getType(aux->param2);
            add_var(aux->param2, valor, type);
            break;

        case 7:
            valor = !getValue(aux->param3);
            type = getType(aux->param2);
            add_var(aux->param2, valor, type);
            break;

        case 8:
            valor = getValue(aux->param3) && getValue(aux->param4);
            type = getType(aux->param2);
            add_var(aux->param2, valor, type);
            break;

        case 9:
            valor = getValue(aux->param3) || getValue(aux->param4);
            type = getType(aux->param2);
            add_var(aux->param2, valor, type);
            break;

        case 10:
            if (getValue(aux->param2))
                aux = getLabel(lista, aux->param3);
            else
                aux = getLabel(lista, aux->param4);

            break;

        case 11:
            aux = getLabel(lista, aux->param2);
            break;

        case 12:
            type = decod_inst(aux->param2);
            sprintf(str, "%s", aux->param3);
            if (type == 13)
            {
                if (str[0] == '\"')
                {
                    strcpy(str, removeaspas(aux->param3));
                    strcpy(str, interpretaStr(str));
                    printf("%s", str);
                }
                else if (str[0] == '_')
                {
                    type = getType(aux->param3);
                    if (type == 0)
                        printf("%.f", getValue(aux->param3));
                    else
                        printf("%.2f", getValue(aux->param3));
                }
                else
                    printf("%.2f", atof(str));

                if (aux->next)
                {
                    if (strcmp(aux->next->param1, (char *)"CALL") == 0)
                    {
                        if (strcmp(aux->next->param2, (char *)"PRINT") != 0)
                            printf("\n");
                    }
                    else
                    {
                        printf("\n");
                    }
                }
                else
                    printf("\n");
            }
            else if (type == 14)
            {
                sprintf(str, "%s", aux->param3);
                strcpy(str, removeaspas(aux->param3));
                strcpy(str, interpretaStr(str));
                printf("%s", str);
                int return_scanf = scanf("%f", &valor);
                flush_in();
                if (!return_scanf)
                {
                    printf("[ERROR]: Falha ao receber entrada de dados\n");
                    exit(ERROR_INPUT);
                }
                type = getType(aux->param4);
                add_var(aux->param4, valor, type);
            }
            else if (type == 20)
            {
                printf("\nFinalizado: retorno %.f\n", trunc(getValue(aux->param3)));
                return;
            }
            break;

        case 15:
            valor = getValue(aux->param3) + getValue(aux->param4);
            type = getType(aux->param2);
            add_var(aux->param2, valor, type);
            break;

        case 16:
            valor = getValue(aux->param3) - getValue(aux->param4);
            type = getType(aux->param2);
            add_var(aux->param2, valor, type);
            break;

        case 17:
            valor = getValue(aux->param3) * getValue(aux->param4);
            type = getType(aux->param2);
            add_var(aux->param2, valor, type);
            break;

        case 18:
            if (getValue(aux->param4) == 0)
            {
                fprintf(stderr, "\nERROR TEMPO DE EXECUÇÃO: Divisão por zero impossivel.\n\n");
                exit(12);
            }
            valor = getValue(aux->param3) / getValue(aux->param4);
            type = getType(aux->param2);
            add_var(aux->param2, valor, type);
            break;

        case 19:
            valor = (unsigned int)getValue(aux->param3) % (unsigned int)getValue(aux->param4);
            type = getType(aux->param2);
            add_var(aux->param2, valor, type);
            break;
        }
    }
    printf("\nFinalizado: sem retorno\n");
    return;
}

int decod_inst(char *opcode)
{
    if (strcmp(opcode, "=") == 0)
        return 0;
    else if (strcmp(opcode, "==") == 0)
        return 1;
    else if (strcmp(opcode, ">") == 0)
        return 2;
    else if (strcmp(opcode, ">=") == 0)
        return 3;
    else if (strcmp(opcode, "<") == 0)
        return 4;
    else if (strcmp(opcode, "<=") == 0)
        return 5;
    else if (strcmp(opcode, "!=") == 0)
        return 6;
    else if (strcmp(opcode, "!") == 0)
        return 7;
    else if (strcmp(opcode, "&&") == 0)
        return 8;
    else if (strcmp(opcode, "||") == 0)
        return 9;
    else if (strcmp(opcode, "IF") == 0)
        return 10;
    else if (strcmp(opcode, "JUMP") == 0)
        return 11;
    else if (strcmp(opcode, "CALL") == 0)
        return 12;
    else if (strcmp(opcode, "PRINT") == 0)
        return 13;
    else if (strcmp(opcode, "SCAN") == 0)
        return 14;
    else if (strcmp(opcode, "+") == 0)
        return 15;
    else if (strcmp(opcode, "-") == 0)
        return 16;
    else if (strcmp(opcode, "*") == 0)
        return 17;
    else if (strcmp(opcode, "/") == 0)
        return 18;
    else if (strcmp(opcode, "%") == 0)
        return 19;
    else if (strcmp(opcode, "RETURN") == 0)
        return 20;

    return -1;
}

void add_var(char *id, double value, byte type)
{
    if (lenVarambiente == 0)
    {
        listVarambiente = (t_varambiente *)malloc(sizeof(t_varambiente));
        if (!listVarambiente)
            error_alloc("listVarambiente", "add_var");
        lenVarambiente = 1;
        listVarambiente[0].id_var = id;
        if (type == 0)
        {
            listVarambiente[0].value_numeric = trunc(value);
            listVarambiente[0].type = 0;
        }
        else
        {
            listVarambiente[0].value_numeric = value;
            listVarambiente[0].type = 1;
        }
        return;
    }
    for (int i = 0; i < lenVarambiente; i++)
        if (strcmp(listVarambiente[i].id_var, id) == 0)
        {
            if (type == 0)
            {
                listVarambiente[i].value_numeric = trunc(value);
                listVarambiente[i].type = 0;
            }
            else
            {
                listVarambiente[i].value_numeric = value;
                listVarambiente[i].type = 1;
            }
            return;
        }

    listVarambiente = (t_varambiente *)realloc(listVarambiente, sizeof(t_varambiente) * (++lenVarambiente));
    if (!listVarambiente)
        error_alloc("listVarambiente", "add_var");
    listVarambiente[lenVarambiente - 1].id_var = id;
    listVarambiente[lenVarambiente - 1].type = type;

    if (type == 0)
        listVarambiente[lenVarambiente - 1].value_numeric = trunc(value);
    else if (type == 1)
        listVarambiente[lenVarambiente - 1].value_numeric = value;
}

byte getType(char *lexema)
{
    for (int i = 0; i < lenVarambiente; i++)
        if (strcmp(lexema, listVarambiente[i].id_var) == 0)
            return listVarambiente[i].type;

    return 1;
}

float getValue(char *lexema)
{
    for (int i = 0; i < lenVarambiente; i++)
        if (strcmp(lexema, listVarambiente[i].id_var) == 0)
            return listVarambiente[i].value_numeric;

    return atof(lexema);
}

Quad *getLabel(Quad *list, char *lexema)
{
    Quad *aux = list;
    while (aux)
    {
        if (strcmp((char *)"LABEL", aux->param1) == 0)
            if (strcmp(lexema, aux->param2) == 0)
                return aux;

        aux = aux->next;
    }
    return NULL;
}

char *removeaspas(char *str)
{
    char len = strlen(str);
    char *aux = (char *)malloc(sizeof(char) * len - 1);
    if (!aux)
        error_alloc("aux", "removeaspas");
    for (int i = 0; i < len - 2; i++)
        aux[i] = str[i + 1];

    aux[len - 2] = '\0';
    return aux;
}

char *interpretaStr(char *str)
{
    int len = strlen(str);
    char *aux = (char *)malloc(sizeof(char) * len);
    if (!aux)
        error_alloc("aux", "interpretaStr");
    int indice = 0;
    for (int i = 0; i < len; i++)
    {
        if (str[i] == '\\')
            switch (str[i + 1])
            {
            case 'n':
                aux[indice] = '\0';
                strcat(aux, "\n");
                indice++;
                i += 1;
                continue;
                break;
            case 't':
                aux[indice] = '\0';
                strcat(aux, "\t");
                indice++;
                i++;
                continue;
                break;
            default:
                aux[indice++] = str[i];
                continue;
                break;
            }

        aux[indice++] = str[i];
    }

    aux[indice] = '\0';
    return aux;
}
