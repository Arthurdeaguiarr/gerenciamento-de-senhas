/* Sistema de Gerenciamento de Senhas de Atendimento Clinico.
 * Categorias (regras didaticas): U = idade > 80; P = gestante, lactante,
 * necessidade especial, obeso, mulher >= 60 ou homem >= 65; N = demais.
 * Chamada: U > P > N; dentro de cada categoria, ordem de emissao (FIFO). */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_FILA   500   /* senhas em espera por categoria */
#define MAX_NUMERO 999   /* ultimo numero por categoria    */
#define PROXIMAS   5     /* senhas listadas no painel      */

typedef enum { NORMAL, PREFERENCIAL, URGENTE } Categoria;  /* maior valor = maior precedencia */

static const char  PREFIXO[] = "NPU";
static const char *NOME[]    = {"Normal", "Preferencial", "Urgente"};

typedef struct {
    char genero;                      /* 'M' ou 'F' */
    int  idade;
    bool gestante, lactante, necessidadeEspecial, obeso;
} Paciente;

typedef struct {
    Categoria categoria;
    int       numero;                 /* sequencial dentro da categoria */
} Senha;

typedef struct {                      /* fila circular (FIFO) */
    Senha itens[MAX_FILA];
    int   inicio, tamanho;
} Fila;

typedef struct {
    Fila  filas[3];                   /* uma por categoria */
    int   ultimoNumero[3];
    int   atendidos[3];
    Senha atual;                      /* senha em atendimento */
    bool  emAtendimento;
} Sistema;

/* ---------- Entrada de dados e validacao ---------- */

/* Le uma linha; encerra o programa se a entrada acabar (EOF). */
void lerLinha(const char *msg, char *dest, int tam)
{
    printf("%s", msg);
    if (fgets(dest, tam, stdin) == NULL) {
        printf("\nEntrada encerrada.\n");
        exit(0);
    }
    if (strchr(dest, '\n') == NULL) {            /* descarta o excedente */
        int c;
        while ((c = getchar()) != '\n' && c != EOF) { }
    }
}

/* Le um inteiro em [min, max], repetindo ate ser valido. */
int lerInteiro(const char *msg, int min, int max)
{
    char linha[32], *fim;
    for (;;) {
        lerLinha(msg, linha, sizeof linha);
        long v = strtol(linha, &fim, 10);
        while (isspace((unsigned char)*fim)) fim++;
        if (fim == linha || *fim != '\0')
            printf("  Entrada invalida: digite um numero inteiro.\n");
        else if (v < min || v > max)
            printf("  Valor fora do intervalo (%d a %d).\n", min, max);
        else
            return (int)v;
    }
}

/* Le um unico caractere que pertenca a 'validos' (ex.: "MF"), em maiuscula. */
char lerOpcao(const char *msg, const char *validos)
{
    char linha[32];
    for (;;) {
        lerLinha(msg, linha, sizeof linha);
        char c = (char)toupper((unsigned char)linha[0]);
        if (c != '\0' && c != '\n' && (linha[1] == '\n' || linha[1] == '\0')
            && strchr(validos, c) != NULL)
            return c;
        printf("  Resposta invalida: use uma das opcoes (%s).\n", validos);
    }
}

bool lerSimNao(const char *msg)
{
    return lerOpcao(msg, "SN") == 'S';
}

/* ---------- Modulo 1: registro de senhas ---------- */

/* Gestante e lactante so sao perguntadas quando o genero e feminino. */
Paciente cadastrarPaciente(void)
{
    Paciente p = {0};
    p.genero = lerOpcao("Genero do paciente (M/F): ", "MF");
    p.idade  = lerInteiro("Idade (anos): ", 0, 130);
    if (p.genero == 'F') {
        p.gestante = lerSimNao("Esta gestante? (S/N): ");
        p.lactante = lerSimNao("E lactante? (S/N): ");
    }
    p.necessidadeEspecial = lerSimNao("Possui necessidade especial? (S/N): ");
    p.obeso               = lerSimNao("Possui condicao de obesidade? (S/N): ");
    return p;
}

/* ---------- Modulo 2: classificacao ---------- */

Categoria classificarPaciente(const Paciente *p)
{
    if (p->idade > 80)
        return URGENTE;
    if (p->gestante || p->lactante || p->necessidadeEspecial || p->obeso
        || (p->genero == 'F' && p->idade >= 60)
        || (p->genero == 'M' && p->idade >= 65))
        return PREFERENCIAL;
    return NORMAL;
}

/* Escreve o codigo da senha (ex.: P002). */
void formatarSenha(const Senha *s, char *dest, size_t tam)
{
    snprintf(dest, tam, "%c%03d", PREFIXO[s->categoria], s->numero);
}

/* ---------- Gerenciamento das filas ---------- */

bool enfileirar(Fila *f, Senha s)
{
    if (f->tamanho == MAX_FILA) return false;
    f->itens[(f->inicio + f->tamanho++) % MAX_FILA] = s;
    return true;
}

bool desenfileirar(Fila *f, Senha *s)
{
    if (f->tamanho == 0) return false;
    *s = f->itens[f->inicio];
    f->inicio = (f->inicio + 1) % MAX_FILA;
    f->tamanho--;
    return true;
}

/* Consulta a posicao i da fila (0 = frente) sem remover; NULL se nao existir. */
const Senha *espiarFila(const Fila *f, int i)
{
    return (i >= 0 && i < f->tamanho) ? &f->itens[(f->inicio + i) % MAX_FILA] : NULL;
}

int totalEmEspera(const Sistema *sis)
{
    return sis->filas[0].tamanho + sis->filas[1].tamanho + sis->filas[2].tamanho;
}

/* Categoria da proxima senha a chamar (U, depois P, depois N); -1 se nao ha. */
int proximaCategoria(const Sistema *sis)
{
    for (int c = URGENTE; c >= NORMAL; c--)
        if (sis->filas[c].tamanho > 0) return c;
    return -1;
}

/* Gera a senha sequencial da categoria do paciente e a enfileira. */
bool emitirSenha(Sistema *sis, const Paciente *p, Senha *emitida)
{
    Categoria c = classificarPaciente(p);
    Senha s = { c, sis->ultimoNumero[c] + 1 };

    if (s.numero > MAX_NUMERO || !enfileirar(&sis->filas[c], s))
        return false;
    sis->ultimoNumero[c] = s.numero;
    *emitida = s;
    return true;
}

/* ---------- Modulo 3: registro de atendimento ---------- */

void consultarProximaSenha(const Sistema *sis)
{
    int c = proximaCategoria(sis);
    char cod[8];

    if (c < 0) { printf("Nao ha senhas aguardando chamada.\n"); return; }
    formatarSenha(espiarFila(&sis->filas[c], 0), cod, sizeof cod);
    printf("Proxima senha a ser chamada: %s (%s)\n", cod, NOME[c]);
}

void chamarProximoPaciente(Sistema *sis)
{
    int c = proximaCategoria(sis);
    char cod[8];

    if (sis->emAtendimento) {
        formatarSenha(&sis->atual, cod, sizeof cod);
        printf("Ja existe atendimento em andamento (senha %s). Finalize-o antes.\n", cod);
    } else if (c < 0) {
        printf("Nao ha senhas aguardando chamada.\n");
    } else {
        desenfileirar(&sis->filas[c], &sis->atual);
        sis->emAtendimento = true;
        formatarSenha(&sis->atual, cod, sizeof cod);
        printf(">>> Chamando senha %s (%s) <<<\n", cod, NOME[c]);
    }
}

/* A senha ja saiu da fila ao ser chamada, portanto nao retorna a ela. */
void finalizarAtendimento(Sistema *sis)
{
    char cod[8];

    if (!sis->emAtendimento) {
        printf("Nao ha atendimento em andamento para finalizar.\n");
        return;
    }
    formatarSenha(&sis->atual, cod, sizeof cod);
    sis->atendidos[sis->atual.categoria]++;
    sis->emAtendimento = false;
    printf("Atendimento da senha %s finalizado.\n", cod);
}

/* ---------- Modulo 4: painel de chamada ---------- */

void exibirPainel(const Sistema *sis)
{
    char cod[8];
    int listadas = 0;

    printf("\n==============================================\n");
    printf("            PAINEL DE CHAMADA\n");
    printf("==============================================\n");
    if (sis->emAtendimento) {
        formatarSenha(&sis->atual, cod, sizeof cod);
        printf(" Senha em atendimento: %s\n Categoria           : %s\n",
               cod, NOME[sis->atual.categoria]);
    } else {
        printf(" Senha em atendimento: nenhuma\n");
    }

    printf("----------------------------------------------\n");
    printf(" Proximas senhas (ordem de chamada):\n");
    for (int c = URGENTE; c >= NORMAL; c--)
        for (int i = 0; i < sis->filas[c].tamanho && listadas < PROXIMAS; i++) {
            formatarSenha(espiarFila(&sis->filas[c], i), cod, sizeof cod);
            printf("   %d. %s (%s)\n", ++listadas, cod, NOME[c]);
        }
    if (listadas == 0)
        printf("   (nenhuma senha em espera)\n");
    else if (totalEmEspera(sis) > listadas)
        printf("   ... e mais %d em espera\n", totalEmEspera(sis) - listadas);

    printf("----------------------------------------------\n");
    printf(" Pacientes em espera por categoria:\n");
    for (int c = URGENTE; c >= NORMAL; c--)
        printf("   %-13s: %d\n", NOME[c], sis->filas[c].tamanho);
    printf("   %-13s: %d\n", "Total", totalEmEspera(sis));
    printf("----------------------------------------------\n");
    printf(" Atendimentos concluidos: U=%d  P=%d  N=%d\n",
           sis->atendidos[URGENTE], sis->atendidos[PREFERENCIAL], sis->atendidos[NORMAL]);
    printf("==============================================\n");
}

/* ---------- Interface ---------- */

void exibirMenu(void)
{
    printf("\n======= SENHAS DE ATENDIMENTO - CLINICA =======\n"
           " 1 - Emitir nova senha\n"
           " 2 - Consultar proxima senha\n"
           " 3 - Chamar proximo paciente\n"
           " 4 - Finalizar atendimento\n"
           " 5 - Exibir painel de chamada\n"
           " 0 - Sair\n"
           "===============================================\n");
}

void registrarNovaSenha(Sistema *sis)
{
    Paciente p = cadastrarPaciente();
    Senha s;
    char cod[8];

    if (!emitirSenha(sis, &p, &s)) {
        printf("Nao foi possivel emitir a senha: limite da categoria %s atingido.\n",
               NOME[classificarPaciente(&p)]);
        return;
    }
    formatarSenha(&s, cod, sizeof cod);
    printf("\nSenha emitida: %s - categoria %s\n", cod, NOME[s.categoria]);
}

void executarMenu(Sistema *sis)
{
    int op;
    do {
        exibirMenu();
        op = lerInteiro("Escolha uma opcao: ", 0, 5);
        switch (op) {
            case 1: registrarNovaSenha(sis);    break;
            case 2: consultarProximaSenha(sis); break;
            case 3: chamarProximoPaciente(sis); break;
            case 4: finalizarAtendimento(sis);  break;
            case 5: exibirPainel(sis);          break;
            default: printf("Encerrando o sistema. Ate logo!\n");
        }
    } while (op != 0);
}

int main(void)
{
    static Sistema sistema;   /* zerada por ser static */
    executarMenu(&sistema);
    return 0;
}
