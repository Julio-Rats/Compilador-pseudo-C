#include "parser.h"

// Variavel que indica o nivel atual de variaveis
static u_int8_t nivel_variaveis = 0;

void error_alloc(char *var, char *func)
{
    fprintf(stderr, "\nERROR ao alocar memoria para '%s' na função '%s'\n\n", var, func);
    exit(ERROR_MEM);
}

void error(TToken consome)
{
    printf("\nError de Compilação:  Linha %d, Coluna %d", token_atual.linha, token_atual.coluna);
    printf("\n\tEsperava \'%s\' Mas Foi Recebido \'%s\'\n\n", decod_Token(consome), token_atual.lexema);
    fclose(file_src);
    exit(ERROR_CONSUME);
}

char *consome_token(TToken consome)
{
    //  Validação do Token, processo do Lexima
    if (consome == token_atual.ttoken)
    {
        char *lexema = (char *)malloc(sizeof(char) * STR_LEN);
        if (!lexema)
            error_alloc("lexema", "consome_token");

        strcpy(lexema, token_atual.lexema);
        token_atual = getToken();
        return lexema;
    }
    error(consome);
    return NULL;
}

t_valuereturns parser()
{
    t_valuereturns parse;
    rewind(file_src);
    token_atual = getToken(); // Inicio do Lexico
    parse = function();
    consome_token(FOE);
    return parse;
}

t_valuereturns function()
{
    t_valuereturns funct;
    type();
    consome_token(IDENT);
    consome_token(ABRIPAR);
    arglist();
    consome_token(FECHAPAR);
    funct = bloco(NULL, NULL);
    return funct;
}

void arglist()
{
    if ((token_atual.ttoken == FLOAT) || (token_atual.ttoken == INT))
    {
        arg();
        restoArglist();
    }
}

void arg()
{
    u_int8_t tipo = type();
    Token token = token_atual;
    consome_token(IDENT);
    if (nivel_variaveis != 1)
        nivel_variaveis = 1;

    add_id(token, tipo);
    nivel_variaveis = 0;
}

void restoArglist()
{
    if (token_atual.ttoken == VIRG)
    {
        consome_token(VIRG);
        arglist();
    }
}

u_int8_t type()
{
    if (token_atual.ttoken == INT)
    {
        consome_token(INT);
        return 0;
    }
    consome_token(FLOAT);
    return 1;
}

t_valuereturns bloco(char *jump_cont, char *jump_exit)
{
    t_valuereturns stmList;
    consome_token(ABRICHAV);
    nivel_variaveis++;
    stmList = stmtList(jump_cont, jump_exit);
    deleta_variaveis();
    nivel_variaveis--;
    consome_token(FECHACHAV);
    return stmList;
}

t_valuereturns stmtList(char *jump_cont, char *jump_exit)
{
    t_valuereturns stmtL, aux;
    stmtL.listQuad = NULL;
    if ((token_atual.ttoken == FOR) || (token_atual.ttoken == PRINT) || (token_atual.ttoken == SCAN) || (token_atual.ttoken == WHILE) || (token_atual.ttoken == NOT) || (token_atual.ttoken == ABRIPAR) || (token_atual.ttoken == SOMA) || (token_atual.ttoken == SUB) || (token_atual.ttoken == IDENT) || (token_atual.ttoken == NUMINT) || (token_atual.ttoken == NUMFLOAT) || (token_atual.ttoken == IF) || (token_atual.ttoken == ABRICHAV) || (token_atual.ttoken == PONTVIRG) || (token_atual.ttoken == BREAK) || (token_atual.ttoken == CONTINUE) || (token_atual.ttoken == RETURN))
    {
        stmtL = stmt(jump_cont, jump_exit);
        aux = stmtList(jump_cont, jump_exit);
        stmtL.listQuad = addQuad(stmtL.listQuad, aux.listQuad);
    }
    else if ((token_atual.ttoken == FLOAT) || (token_atual.ttoken == INT))
    {
        stmtL = declaration();
        aux = stmtList(jump_cont, jump_exit);
        stmtL.listQuad = addQuad(stmtL.listQuad, aux.listQuad);
    }
    return stmtL;
}

t_valuereturns stmt(char *jump_cont, char *jump_exit)
{
    t_valuereturns aux;
    aux.listQuad = NULL;
    if (token_atual.ttoken == FOR)
        aux = forStmt();
    else if ((token_atual.ttoken == PRINT) || (token_atual.ttoken == SCAN))
        aux = ioStmt();
    else if (token_atual.ttoken == WHILE)
        aux = whileStmt();
    else if ((token_atual.ttoken == NOT) || (token_atual.ttoken == ABRIPAR) || (token_atual.ttoken == SOMA) || (token_atual.ttoken == SUB) || (token_atual.ttoken == IDENT) || (token_atual.ttoken == NUMINT) || (token_atual.ttoken == NUMFLOAT))
    {
        aux = expr();
        consome_token(PONTVIRG);
    }
    else if (token_atual.ttoken == IF)
        aux = ifStmt(jump_cont, jump_exit);
    else if (token_atual.ttoken == ABRICHAV)
        aux = bloco(jump_cont, jump_exit);
    else if (token_atual.ttoken == CONTINUE)
    {
        if (!jump_cont)
        {
            fprintf(stderr, "\nERROR: DECLARAÇÃO INVALIDA 'CONTINUE', LINHA %d coluna %d\n\n", token_atual.linha, token_atual.coluna);
            fclose(file_src);
            exit(ERROR_DECLAR);
        }
        consome_token(CONTINUE);
        Quad *quadCom = genQuad((char *)"JUMP", jump_cont, NULL, NULL);
        aux.listQuad = addQuad(aux.listQuad, quadCom);
        consome_token(PONTVIRG);
    }
    else if (token_atual.ttoken == BREAK)
    {
        if (!jump_cont)
        {
            fprintf(stderr, "\nERROR: DECLARAÇÃO INVALIDA 'BREAK', LINHA %d coluna %d\n\n", token_atual.linha, token_atual.coluna);
            fclose(file_src);
            exit(ERROR_DECLAR);
        }
        consome_token(BREAK);
        Quad *quadCom = genQuad((char *)"JUMP", jump_exit, NULL, NULL);
        aux.listQuad = addQuad(aux.listQuad, quadCom);
        consome_token(PONTVIRG);
    }
    else if (token_atual.ttoken == RETURN)
    {
        consome_token(RETURN);
        aux = expr();
        consome_token(PONTVIRG);
        Quad *quadCom = genQuad((char *)"CALL", (char *)"RETURN", aux.NameResult, NULL);
        aux.listQuad = addQuad(aux.listQuad, quadCom);
    }
    else
        consome_token(PONTVIRG);

    return aux;
}

t_valuereturns declaration()
{
    t_valuereturns declar;
    u_int8_t vartype = type();
    declar = identList(vartype);
    consome_token(PONTVIRG);
    return declar;
}

t_valuereturns identList(u_int8_t vartype)
{
    t_valuereturns identL;
    Token atual = token_atual;
    char *lexema = consome_token(IDENT);
    add_id(atual, vartype);
    identL = restoIdentList(vartype);
    Quad *quadCom = NULL;
    if (vartype == 0)
        quadCom = genQuad((char *)"=", busca_variaveis(lexema), (char *)"V", (char *)"0");
    else
        quadCom = genQuad((char *)"=", busca_variaveis(lexema), (char *)"V", (char *)"1");

    identL.listQuad = addQuad(identL.listQuad, quadCom);
    return identL;
}

t_valuereturns restoIdentList(u_int8_t vartype)
{
    t_valuereturns restI;
    restI.listQuad = NULL;
    if (token_atual.ttoken == VIRG)
    {
        consome_token(VIRG);
        Token token = token_atual;
        char *lexema = consome_token(IDENT);
        add_id(token, vartype);
        restI = restoIdentList(vartype);
        Quad *quadCom = NULL;
        if (vartype == 0)
            quadCom = genQuad((char *)"=", busca_variaveis(lexema), (char *)"V", (char *)"0");
        else
            quadCom = genQuad((char *)"=", busca_variaveis(lexema), (char *)"V", (char *)"1");

        restI.listQuad = addQuad(restI.listQuad, quadCom);
    }
    return restI;
}

t_valuereturns forStmt()
{
    t_valuereturns aux, aux2, aux3, aux4, aux5;
    consome_token(FOR);
    consome_token(ABRIPAR);
    aux = optExpr();
    consome_token(PONTVIRG);
    aux2 = optExpr();
    aux5.listQuad = copyQuad(aux2.listQuad);
    consome_token(PONTVIRG);
    aux3 = optExpr();
    consome_token(FECHAPAR);
    char *labReturn = genLabel();
    char *labCont = genLabel();
    char *labExit = genLabel();
    aux4 = stmt(labCont, labExit);
    Quad *q1 = genQuad((char *)"IF", aux2.NameResult, labReturn, labExit);
    Quad *q1_5 = copyQuad(q1);
    Quad *q2 = genQuad((char *)"LABEL", labReturn, NULL, NULL);
    Quad *q3 = genQuad((char *)"LABEL", labExit, NULL, NULL);
    Quad *q4 = genQuad((char *)"LABEL", labCont, NULL, NULL);
    aux.listQuad = addQuad(aux.listQuad, aux2.listQuad);
    aux.listQuad = addQuad(aux.listQuad, q1);
    aux.listQuad = addQuad(aux.listQuad, q2);
    aux.listQuad = addQuad(aux.listQuad, aux4.listQuad);
    aux.listQuad = addQuad(aux.listQuad, q4);
    aux.listQuad = addQuad(aux.listQuad, aux3.listQuad);
    aux.listQuad = addQuad(aux.listQuad, aux5.listQuad);
    aux.listQuad = addQuad(aux.listQuad, q1_5);
    aux.listQuad = addQuad(aux.listQuad, q3);
    return aux;
}

t_valuereturns optExpr()
{
    t_valuereturns optExpr;
    optExpr.listQuad = NULL;
    if ((token_atual.ttoken == NOT) || (token_atual.ttoken == ABRIPAR) || (token_atual.ttoken == SOMA) || (token_atual.ttoken == SUB) || (token_atual.ttoken == IDENT) || (token_atual.ttoken == NUMINT) || (token_atual.ttoken == NUMFLOAT))
    {
        optExpr = expr();
    }
    return optExpr;
}

t_valuereturns ioStmt()
{
    t_valuereturns ioStmt;
    ioStmt.listQuad = NULL;
    if (token_atual.ttoken == SCAN)
    {
        consome_token(SCAN);
        consome_token(ABRIPAR);
        char *str = consome_token(STR);
        consome_token(VIRG);
        Token atual = token_atual;
        consome_token(IDENT);
        if (!busca_variaveis(atual.lexema))
        {
            fprintf(stderr, "\nError de Compilação: Linha %d, Coluna %d\n\tUso de Variavel Não Declarada,  Variavel = \'%s\'\n\n", atual.linha, atual.coluna, atual.lexema);
            fclose(file_src);
            exit(ERROR_COMP);
        }
        Quad *quadCom = genQuad((char *)"CALL", (char *)"SCAN", str, busca_variaveis(atual.lexema));
        ioStmt.listQuad = addQuad(ioStmt.listQuad, quadCom);
        consome_token(FECHAPAR);
        consome_token(PONTVIRG);
    }
    else
    {
        consome_token(PRINT);
        consome_token(ABRIPAR);
        ioStmt = outList();
        consome_token(FECHAPAR);
        consome_token(PONTVIRG);
    }
    return ioStmt;
}

t_valuereturns outList()
{
    t_valuereturns outL, aux2;
    outL = out();
    aux2 = restOutList();
    outL.listQuad = addQuad(outL.listQuad, aux2.listQuad);
    outL.NameResult = aux2.NameResult;
    return outL;
}

t_valuereturns out()
{
    t_valuereturns aux;
    aux.listQuad = NULL;
    char *lexema;
    Quad *quadCom;
    if (token_atual.ttoken == STR)
    {
        lexema = consome_token(STR);
        quadCom = genQuad((char *)"CALL", (char *)"PRINT", lexema, NULL);
        aux.NameResult = lexema;
    }
    else if (token_atual.ttoken == NUMFLOAT)
    {
        lexema = consome_token(NUMFLOAT);
        quadCom = genQuad((char *)"CALL", (char *)"PRINT", lexema, NULL);
        aux.NameResult = lexema;
    }
    else if (token_atual.ttoken == NUMINT)
    {
        lexema = consome_token(NUMINT);
        quadCom = genQuad((char *)"CALL", (char *)"PRINT", lexema, NULL);
        aux.NameResult = lexema;
    }
    else
    {
        Token atual = token_atual;
        consome_token(IDENT);
        if (!busca_variaveis(atual.lexema))
        {
            fprintf(stderr, "\nError de Compilação: Linha %d, Coluna %d\n\tUso de Variavel Não Declarada,  Variavel = \'%s\'\n\n", atual.linha, atual.coluna, atual.lexema);
            fclose(file_src);
            exit(ERROR_COMP);
        }
        quadCom = genQuad((char *)"CALL", (char *)"PRINT", busca_variaveis(atual.lexema), NULL);
        aux.NameResult = busca_variaveis(atual.lexema);
    }
    aux.listQuad = addQuad(aux.listQuad, quadCom);
    return aux;
}

t_valuereturns restOutList()
{
    t_valuereturns restOut, restOut2;
    restOut.listQuad = NULL;
    if (token_atual.ttoken == VIRG)
    {
        consome_token(VIRG);
        restOut = out();
        restOut2 = restOutList();
        restOut.listQuad = addQuad(restOut.listQuad, restOut2.listQuad);
        restOut.NameResult = restOut2.NameResult;
    }
    return restOut;
}

t_valuereturns whileStmt()
{
    t_valuereturns aux, aux2, aux3;
    consome_token(WHILE);
    consome_token(ABRIPAR);
    aux = expr();
    consome_token(FECHAPAR);
    aux3.listQuad = copyQuad(aux.listQuad);
    char *labReturn = genLabel();
    char *labCont = genLabel();
    char *labExit = genLabel();
    aux2 = stmt(labCont, labExit);
    Quad *q1 = genQuad((char *)"IF", aux.NameResult, labReturn, labExit);
    Quad *q1_5 = copyQuad(q1);
    Quad *q2 = genQuad((char *)"LABEL", labReturn, NULL, NULL);
    Quad *q3 = genQuad((char *)"LABEL", labExit, NULL, NULL);
    Quad *q4 = genQuad((char *)"LABEL", labCont, NULL, NULL);
    aux.listQuad = addQuad(aux.listQuad, q1);
    aux.listQuad = addQuad(aux.listQuad, q2);
    aux.listQuad = addQuad(aux.listQuad, aux2.listQuad);
    aux.listQuad = addQuad(aux.listQuad, q4);
    aux.listQuad = addQuad(aux.listQuad, aux3.listQuad);
    aux.listQuad = addQuad(aux.listQuad, q1_5);
    aux.listQuad = addQuad(aux.listQuad, q3);
    return aux;
}

t_valuereturns ifStmt(char *jump_cont, char *jump_exit)
{
    t_valuereturns aux, aux2, aux3;
    consome_token(IF);
    consome_token(ABRIPAR);
    char *labTrue = genLabel();
    char *labFalse = genLabel();
    aux = expr();
    Quad *q1 = genQuad((char *)"IF", aux.NameResult, labTrue, labFalse);
    aux.listQuad = addQuad(aux.listQuad, q1);
    consome_token(FECHAPAR);
    Quad *q2 = genQuad((char *)"LABEL", labTrue, NULL, NULL);
    aux.listQuad = addQuad(aux.listQuad, q2);
    aux2 = stmt(jump_cont, jump_exit);
    aux.listQuad = addQuad(aux.listQuad, aux2.listQuad);
    aux3 = elsePart(jump_cont, jump_exit);
    Quad *q4 = genQuad((char *)"LABEL", labFalse, NULL, NULL);
    if (aux3.listQuad)
    {
        char *labExit = genLabel();
        Quad *q3 = genQuad((char *)"JUMP", labExit, NULL, NULL);
        Quad *q5 = genQuad((char *)"LABEL", labExit, NULL, NULL);
        aux.listQuad = addQuad(aux.listQuad, q3);
        aux.listQuad = addQuad(aux.listQuad, q4);
        aux.listQuad = addQuad(aux.listQuad, aux3.listQuad);
        aux.listQuad = addQuad(aux.listQuad, q5);
        aux.NameResult = aux3.NameResult;
    }
    else
    {
        aux.NameResult = aux2.NameResult;
        aux.listQuad = addQuad(aux.listQuad, q4);
    }
    return aux;
}

t_valuereturns elsePart(char *jump_cont, char *jump_exit)
{
    t_valuereturns aux;
    aux.listQuad = NULL;
    if (token_atual.ttoken == ELSE)
    {
        consome_token(ELSE);
        aux = stmt(jump_cont, jump_exit);
    }
    return aux;
}

t_valuereturns expr()
{
    t_valuereturns aux;
    aux = atrib();
    return aux;
}

t_valuereturns atrib()
{
    t_valuereturns aux, aux2;
    aux = or ();
    aux2 = restoAtrib(aux.NameResult);
    if (!((aux.bool_leftValue) || (aux2.bool_leftValue)))
    {
        fprintf(stderr, "\nErro de Atribuição de Variavel, linha %d coluna %d\n\n",
                token_atual.linha, token_atual.coluna);
        fclose(file_src);
        exit(ERROR_COMP);
    }
    aux.listQuad = addQuad(aux.listQuad, aux2.listQuad);
    aux.NameResult = aux2.NameResult;
    return aux;
}

t_valuereturns restoAtrib(char *parametro)
{
    t_valuereturns aux;
    aux.listQuad = NULL;
    if (token_atual.ttoken == ATRIB)
    {
        consome_token(ATRIB);
        aux = atrib();
        Quad *q1 = genQuad((char *)"=", parametro, aux.NameResult, NULL);
        aux.listQuad = addQuad(aux.listQuad, q1);
    }
    else
    {
        aux.bool_leftValue = 1;
        aux.NameResult = parametro;
    }
    return aux;
}

t_valuereturns or ()
{
    t_valuereturns aux, aux2;
    aux = and();
    aux2 = restoOr(aux.NameResult);
    aux.listQuad = addQuad(aux.listQuad, aux2.listQuad);
    aux.NameResult = aux2.NameResult;
    aux.bool_leftValue &= aux2.bool_leftValue;
    return aux;
}

t_valuereturns restoOr(char *parametro)
{
    t_valuereturns aux, aux2;
    aux.listQuad = NULL;
    if (token_atual.ttoken == OR)
    {
        consome_token(OR);
        aux = and();
        aux2 = restoOr(aux.NameResult);
        char *temp = genTemp();
        Quad *q1 = genQuad((char *)"||", temp, parametro, aux.NameResult);
        aux.listQuad = addQuad(aux.listQuad, q1);
        aux.listQuad = addQuad(aux.listQuad, aux2.listQuad);
        aux.NameResult = temp;
    }
    else
    {
        aux.bool_leftValue = 1;
        aux.NameResult = parametro;
    }
    return aux;
}

t_valuereturns and ()
{
    t_valuereturns aux, aux2;
    aux = not();
    aux2 = restoAnd(aux.NameResult);
    aux.listQuad = addQuad(aux.listQuad, aux2.listQuad);
    aux.NameResult = aux2.NameResult;
    aux.bool_leftValue &= aux2.bool_leftValue;
    return aux;
}

t_valuereturns restoAnd(char *parametro)
{
    t_valuereturns aux, aux2;
    aux.listQuad = NULL;
    if (token_atual.ttoken == AND)
    {
        consome_token(AND);
        aux = not();
        aux2 = restoAnd(aux.NameResult);
        char *temp = genTemp();
        Quad *q1 = genQuad((char *)"&&", temp, parametro, aux.NameResult);
        aux.listQuad = addQuad(aux.listQuad, q1);
        aux.listQuad = addQuad(aux.listQuad, aux2.listQuad);
        aux.NameResult = temp;
    }
    else
    {
        aux.bool_leftValue = 1;
        aux.NameResult = parametro;
    }
    return aux;
}

t_valuereturns not()
{
    t_valuereturns aux;
    if (token_atual.ttoken == NOT)
    {
        consome_token(NOT);
        aux = not();
        char *temp = genTemp();
        Quad *q1 = genQuad((char *)"!", temp, aux.NameResult, NULL);
        aux.listQuad = addQuad(aux.listQuad, q1);
        aux.NameResult = temp;
    }
    else
        aux = rel(); // nao muda

    return aux;
}

t_valuereturns rel()
{
    t_valuereturns aux, aux2;
    aux = add();
    aux2 = restorel(aux.NameResult);
    aux.listQuad = addQuad(aux.listQuad, aux2.listQuad);
    aux.NameResult = aux2.NameResult;
    aux.bool_leftValue &= aux2.bool_leftValue;
    return aux;
}

t_valuereturns restorel(char *parametro)
{
    t_valuereturns aux;
    aux.listQuad = NULL;
    if (token_atual.ttoken == IGUAL)
    {
        char *temp = genTemp();
        consome_token(IGUAL);
        aux = add();
        Quad *q1 = genQuad((char *)"==", temp, parametro, aux.NameResult);
        aux.listQuad = addQuad(aux.listQuad, q1);
        aux.NameResult = temp;
        aux.bool_leftValue = 0;
    }
    else if (token_atual.ttoken == NIGUAL)
    {
        char *temp = genTemp();
        consome_token(NIGUAL);
        aux = add();
        Quad *q1 = genQuad((char *)"!=", temp, parametro, aux.NameResult);
        aux.listQuad = addQuad(aux.listQuad, q1);
        aux.NameResult = temp;
        aux.bool_leftValue = 0;
    }
    else if (token_atual.ttoken == MENOR)
    {
        char *temp = genTemp();
        consome_token(MENOR);
        aux = add();
        Quad *q1 = genQuad((char *)"<", temp, parametro, aux.NameResult);
        aux.listQuad = addQuad(aux.listQuad, q1);
        aux.NameResult = temp;
        aux.bool_leftValue = 0;
    }
    else if (token_atual.ttoken == MENORIGUAL)
    {
        char *temp = genTemp();
        consome_token(MENORIGUAL);
        aux = add();
        Quad *q1 = genQuad((char *)"<=", temp, parametro, aux.NameResult);
        aux.listQuad = addQuad(aux.listQuad, q1);
        aux.NameResult = temp;
        aux.bool_leftValue = 0;
    }
    else if (token_atual.ttoken == MAIOR)
    {
        char *temp = genTemp();
        consome_token(MAIOR);
        aux = add();
        Quad *q1 = genQuad((char *)">", temp, parametro, aux.NameResult);
        aux.listQuad = addQuad(aux.listQuad, q1);
        aux.NameResult = temp;
        aux.bool_leftValue = 0;
    }
    else if (token_atual.ttoken == MAIORIGUAL)
    {
        char *temp = genTemp();
        consome_token(MAIORIGUAL);
        aux = add();
        Quad *q1 = genQuad((char *)">=", temp, parametro, aux.NameResult);
        aux.listQuad = addQuad(aux.listQuad, q1);
        aux.NameResult = temp;
        aux.bool_leftValue = 0;
    }
    else
    {
        aux.bool_leftValue = 1;
        aux.NameResult = parametro;
    }

    return aux;
}

t_valuereturns add()
{
    t_valuereturns aux, aux2;
    aux = mult();
    aux2 = restoAdd(aux.NameResult);
    aux.listQuad = addQuad(aux.listQuad, aux2.listQuad);
    aux.NameResult = aux2.NameResult;
    aux.bool_leftValue &= aux2.bool_leftValue;
    return aux;
}

t_valuereturns restoAdd(char *parametro)
{
    t_valuereturns aux, aux2;
    aux.listQuad = NULL;
    if (token_atual.ttoken == SOMA)
    {
        consome_token(SOMA);
        aux = mult();
        char *temp = genTemp();
        aux2 = restoAdd(temp); // WALACE
        Quad *q1 = genQuad((char *)"+", temp, parametro, aux.NameResult);
        aux.listQuad = addQuad(aux.listQuad, q1);
        aux.listQuad = addQuad(aux.listQuad, aux2.listQuad);
        aux.NameResult = aux2.NameResult;
        aux.bool_leftValue = 0;
    }
    else if (token_atual.ttoken == SUB)
    {
        consome_token(SUB);
        aux = mult();
        char *temp = genTemp();
        aux2 = restoAdd(temp); // WALACE
        Quad *q1 = genQuad((char *)"-", temp, parametro, aux.NameResult);
        aux.listQuad = addQuad(aux.listQuad, q1);
        aux.listQuad = addQuad(aux.listQuad, aux2.listQuad);
        aux.NameResult = aux2.NameResult;
        aux.bool_leftValue = 0;
    }
    else
    {
        aux.bool_leftValue = 1;
        aux.NameResult = parametro;
    }

    return aux;
}

t_valuereturns mult()
{
    t_valuereturns aux, aux2;
    aux = uno();
    aux2 = restoMult(aux.NameResult);
    aux.listQuad = addQuad(aux.listQuad, aux2.listQuad);
    aux.NameResult = aux2.NameResult;
    aux.bool_leftValue &= aux2.bool_leftValue;
    return aux;
}

t_valuereturns restoMult(char *parametro)
{
    t_valuereturns aux;
    aux.listQuad = NULL;
    if (token_atual.ttoken == MULT)
    {
        consome_token(MULT);
        aux = uno();
        char *temp = genTemp();
        Quad *q1 = genQuad((char *)"*", temp, parametro, aux.NameResult);
        aux.listQuad = addQuad(aux.listQuad, q1);
        aux.NameResult = temp;
        aux.bool_leftValue = 0;
    }
    else if (token_atual.ttoken == DIVI)
    {
        consome_token(DIVI);
        aux = uno();
        char *temp = genTemp();
        Quad *q1 = genQuad((char *)"/", temp, parametro, aux.NameResult);
        aux.listQuad = addQuad(aux.listQuad, q1);
        aux.NameResult = temp;
        aux.bool_leftValue = 0;
    }
    else if (token_atual.ttoken == MOD)
    {
        consome_token(MOD);
        aux = uno();
        char *temp = genTemp();
        Quad *q1 = genQuad((char *)"%", temp, parametro, aux.NameResult);
        aux.listQuad = addQuad(aux.listQuad, q1);
        aux.NameResult = temp;
        aux.bool_leftValue = 0;
    }
    else
    {
        aux.bool_leftValue = 1;
        aux.NameResult = parametro; // WALACE
    }

    return aux;
}

t_valuereturns uno()
{
    t_valuereturns aux;
    if (token_atual.ttoken == SOMA)
    {
        consome_token(SOMA);
        aux = uno();
        aux.bool_leftValue = 0;
    }
    else if (token_atual.ttoken == SUB)
    {
        consome_token(SUB);
        aux = uno();
        Quad *q1 = genQuad((char *)"-", aux.NameResult, "0", aux.NameResult);
        aux.listQuad = addQuad(aux.listQuad, q1);
        aux.bool_leftValue = 0;
    }
    else
        aux = fator();

    return aux;
}

t_valuereturns fator()
{
    t_valuereturns aux;
    aux.listQuad = NULL;
    if (token_atual.ttoken == ABRIPAR)
    {
        consome_token(ABRIPAR);
        aux = atrib();
        consome_token(FECHAPAR);
        aux.bool_leftValue = 0;
    }
    else if (token_atual.ttoken == NUMFLOAT)
    {
        char *temp = genTemp();
        char *lexema = consome_token(NUMFLOAT);
        Quad *q1 = genQuad((char *)"=", temp, lexema, NULL);
        aux.listQuad = addQuad(aux.listQuad, q1);
        aux.NameResult = temp;
        aux.bool_leftValue = 0;
    }
    else if (token_atual.ttoken == IDENT)
    {
        char *lexema = consome_token(IDENT);
        u_int32_t flag = 1;
        for (u_int32_t i = 0; i < lenVariables; i++)
            if (strcmp(lexema, listVariables[i].id_var) == 0)
            {
                flag = 0;
                break;
            }

        if (flag)
        {
            fprintf(stderr, "\nError de Compilação: Linha %d, Coluna %d\n\tAtribuição A Uma Variavel Não Declarada,  Variavel = \'%s\'\n\n", token_atual.linha, token_atual.coluna, lexema);
            fclose(file_src);
            exit(ERROR_COMP);
        }
        aux.NameResult = busca_variaveis(lexema);
        aux.bool_leftValue = 1;
    }
    else
    {
        char *lexema = consome_token(NUMINT);
        char *temp = genTemp();
        Quad *q1 = genQuad((char *)"=", temp, lexema, NULL);
        aux.listQuad = addQuad(aux.listQuad, q1);
        aux.NameResult = temp;
        aux.bool_leftValue = 0;
    }

    return aux;
}

char *decod_Token(TToken token)
{
    char *aux = (char *)malloc(sizeof(char) * STR_LEN);
    if (!aux)
        error_alloc("aux", "decod_Token");
    switch (token)
    {
    case 1:
        strcpy(aux, "Abri Parentese");
        break;
    case 2:
        strcpy(aux, "Fecha Parentese");
        break;
    case 3:
        strcpy(aux, "Virgula");
        break;
    case 4:
        strcpy(aux, "Ponto e Virgula");
        break;
    case 5:
        strcpy(aux, "Final de Arquivo");
        break;
    case 6:
        strcpy(aux, "ERROR (Não interpretado)");
        break;
    case 7:
        strcpy(aux, "Identificador");
        break;
    case 8:
        strcpy(aux, "String");
        break;
    case 9:
        strcpy(aux, "Sinal de soma(+)");
        break;
    case 10:
        strcpy(aux, "Sinal de subtração (-)");
        break;
    case 11:
        strcpy(aux, "Sinal de Divisão (/)");
        break;
    case 12:
        strcpy(aux, "Atribuição (=)");
        break;
    case 13:
        strcpy(aux, "Sinal de Multiplicação (*)");
        break;
    case 14:
        strcpy(aux, "Igual (==)");
        break;
    case 15:
        strcpy(aux, "Modulo (%)");
        break;
    case 16:
        strcpy(aux, "PRINT");
        break;
    case 17:
        strcpy(aux, "SCAN");
        break;
    case 18:
        strcpy(aux, "AND (&&)");
        break;
    case 19:
        strcpy(aux, "Abri chaves");
        break;
    case 20:
        strcpy(aux, "Fecha chaves");
        break;
    case 21:
        strcpy(aux, "Declaração de u_int32_t");
        break;
    case 22:
        strcpy(aux, "Declaração de FLOAT");
        break;
    case 23:
        strcpy(aux, "BREAK");
        break;
    case 24:
        strcpy(aux, "CONTINUE");
        break;
    case 25:
        strcpy(aux, "FOR");
        break;
    case 26:
        strcpy(aux, "Uma Expressão");
        break;
    case 27:
        strcpy(aux, "Numero Float");
        break;
    case 28:
        strcpy(aux, "WHILE");
        break;
    case 29:
        strcpy(aux, "IF");
        break;
    case 30:
        strcpy(aux, "ELSE");
        break;
    case 31:
        strcpy(aux, "OR ( || )");
        break;
    case 32:
        strcpy(aux, "Not (!)");
        break;
    case 33:
        strcpy(aux, "Menor (<)");
        break;
    case 34:
        strcpy(aux, "Maior (>)");
        break;
    case 35:
        strcpy(aux, "Menor Igual (<=)");
        break;
    case 36:
        strcpy(aux, "Maior Igual (>=)");
        break;
    case 37:
        strcpy(aux, "Diferente (!=)");
        break;
    default:
        printf("[ERRO]: Token desconhecido (decod_Token)\n");
        exit(ERROR_TOKEN);
    }

    return aux;
}

void add_id(Token token, u_int8_t tipo)
{
    if (lenVariables == 0)
    {
        listVariables = (t_variable *)malloc(sizeof(t_variable));
        if (!listVariables)
            error_alloc("listVariables", "add_id");

        lenVariables = 1;
        listVariables[0].id_var = (char *)malloc(sizeof(char) * STR_LEN);
        if (!listVariables[0].id_var)
            error_alloc("listVariables[0].id_var", "add_id");

        listVariables[0].type = tipo;
        listVariables[0].nivel = nivel_variaveis;
        strcpy(listVariables[0].id_var, token.lexema);
    }
    else
    {
        for (u_int32_t i = 0; i < lenVariables; i++)
            if (strcmp(token.lexema, listVariables[i].id_var) == 0)
                if (nivel_variaveis == listVariables[i].nivel)
                {
                    fprintf(stderr, "\nError de Compilação: Linha %d, Coluna %d\n\tMúltipla Declaração de Variavel,  Variavel = \'%s\'\n\n", token.linha, token.coluna, token.lexema);
                    fclose(file_src);
                    exit(ERROR_COMP);
                }

        listVariables = (t_variable *)realloc(listVariables, sizeof(t_variable) * (++(lenVariables)));
        if (!listVariables)
            error_alloc("listVariables", "add_id");

        listVariables[(lenVariables)-1].id_var = (char *)malloc(sizeof(char) * STR_LEN);
        if (!listVariables[(lenVariables)-1].id_var)
            error_alloc("listVariables[(lenVariables)-1].id_var", "add_id");

        listVariables[(lenVariables)-1].type = tipo;
        listVariables[(lenVariables)-1].nivel = nivel_variaveis;
        strcpy(listVariables[(lenVariables)-1].id_var, token.lexema);
    }
}

void deleta_variaveis()
{
    u_int32_t cont = 0;
    while (cont < lenVariables)
    {
        if ((listVariables[cont].nivel == nivel_variaveis) && (nivel_variaveis != 1))
        {
            for (; cont < lenVariables - 1; cont++)
                listVariables[cont] = listVariables[cont + 1];

            listVariables = (t_variable *)realloc(listVariables, sizeof(t_variable) * (--(lenVariables)));
            if (!listVariables)
                error_alloc("listVariables", "deleta_variaveis");
        }
        cont++;
    }
}

char *busca_variaveis(char *lexema)
{
    char *var = (char *)malloc(sizeof(char) * (STR_LEN));
    if (!var)
        error_alloc("var", "busca_variaveis");
    for (int32_t i = lenVariables - 1; i >= 0; i--)
        if (strcmp(listVariables[i].id_var, lexema) == 0)
        {
            sprintf(var, "_%d%s", listVariables[i].nivel, listVariables[i].id_var);
            return var;
        }
    return NULL;
}
