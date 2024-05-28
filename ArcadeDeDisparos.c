#include "keyboard.h"
#include "screen.h"
#include "timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define VEL_TIROS_INIMIGOS 1
#define INTERVALO_TIRO 100
#define CORACAO "\u2764"
#define VIDA_INICIAL 3
#define MAX_TIROS_INIMIGOS 20
#define LNAVE 5
#define ANAVE 1
#define ESQUERDA 'a'
#define DIREITA 'd'
#define LINIMIGO 3
#define AINIMIGO 1
#define MAX_INIMIGOS 5
#define LTIRO 1
#define ATIRO 1
#define MAX_TIROS 6

enum TipoInimigo { INIMIGO_UNICO };

typedef struct Node {
  char nome[4];
  int pontos;
  struct Node *next;
} Node;

typedef struct {
  int Iniciado;
  int x_tiroInimigo, y_tiroInimigo;
} TiroDosInimigos;

typedef struct {
  int x_NavesInimigas, y_NavesInimigas;
  int vida;
  int Iniciado;
  enum TipoInimigo Tipo;
} NavesInimigas;

typedef struct {
  int Iniciado;
  int x_Tironave, y_Tironave;
} Tiro;

typedef struct {
  int x_Nave, y_Nave;
} Objeto;

typedef struct {
  int Nivel;
  int TotalInimigos;
} InfoDoJG;

Node *listaPontuacao = NULL;
TiroDosInimigos TirosInimigos[MAX_TIROS_INIMIGOS];
InfoDoJG InfoJG;
Objeto Nave;
NavesInimigas Inimigos[MAX_INIMIGOS];
Tiro Tiros[MAX_TIROS];
int VidaJgdr = VIDA_INICIAL;
int Direcao = 1;
int Score = 0;

int InimigosDerrotados() {
  for (int i = 0; i < MAX_INIMIGOS; ++i) {
    if (Inimigos[i].Iniciado) {
      return 0;
    }
  }
  return 1;
}

void adicionarPontuacao(char nome[4], int pontos) {
  Node *novoNode = (Node *)malloc(sizeof(Node));
  strcpy(novoNode->nome, nome);
  novoNode->pontos = pontos;

  Node **ptr = &listaPontuacao;
  while (*ptr != NULL && pontos < (*ptr)->pontos) {
    ptr = &(*ptr)->next;
  }

  novoNode->next = *ptr;
  *ptr = novoNode;
}

void mostrarRanking() {
  printf("\n\033[1;34mRanking:\033[m\n");
  Node *atual = listaPontuacao;
  for (int posicao = 1; atual != NULL && posicao <= 3; ++posicao) {
    printf("%d. %s \033[1;34m- %d pontos\033[m\n", posicao, atual->nome, atual->pontos);
    atual = atual->next;
  }
}

void salvarPontuacaoEmArquivo() {
  FILE *arquivo = fopen("pontuacao.dat", "wb");
  if (arquivo == NULL) {
    exit(EXIT_FAILURE);
  }
  Node *atual = listaPontuacao;
  while (atual != NULL) {
    fwrite(atual, sizeof(Node), 1, arquivo);
    atual = atual->next;
  }
  fclose(arquivo);
}

void carregarPontuacaoDoArquivo() {
  FILE *arquivo = fopen("pontuacao.dat", "rb");
  if (arquivo != NULL) {
    Node buffer;
    while (fread(&buffer, sizeof(Node), 1, arquivo) == 1) {
      adicionarPontuacao(buffer.nome, buffer.pontos);
    }
    fclose(arquivo);
  }
}
void liberarListaPontuacao() {
  Node *atual = listaPontuacao;
  while (atual != NULL) {
    Node *proximo = atual->next;
    free(atual);
    atual = proximo;
  }
}

void DesignTirosInimigos() {
  for (int i = 0; i < MAX_TIROS_INIMIGOS; ++i) {
    if (TirosInimigos[i].Iniciado) {
      screenGotoxy(TirosInimigos[i].x_tiroInimigo,
                   TirosInimigos[i].y_tiroInimigo);
      printf("\033[1;33m!\033m");
    }
  }
}
void PercursoTirosInimigos() {
  for (int i = 0; i < MAX_TIROS_INIMIGOS; ++i) {
    if (TirosInimigos[i].Iniciado) {
      TirosInimigos[i].y_tiroInimigo += VEL_TIROS_INIMIGOS;
      if (TirosInimigos[i].y_tiroInimigo >= MAXY) {
        TirosInimigos[i].Iniciado = 0;
      }
    }
  }
}
void TiroInimigo() {
  for (int i = 0; i < MAX_INIMIGOS; ++i) {
    if (Inimigos[i].Iniciado) {
      for (int j = 0; j < MAX_TIROS_INIMIGOS; ++j) {
        if (!TirosInimigos[j].Iniciado) {
          TirosInimigos[j].Iniciado = 1;
          TirosInimigos[j].x_tiroInimigo =
              Inimigos[i].x_NavesInimigas + LINIMIGO / 2;
          TirosInimigos[j].y_tiroInimigo =
              Inimigos[i].y_NavesInimigas + AINIMIGO + 1;
          break;
        }
      }
    }
  }
}
void drawBorders() {
  screenSetColor(CYAN, BLACK);
  screenBoxEnable();
  screenBoxEnable();
  screenGotoxy(MINX, MINY);
  printf("%c", BOX_UPLEFT);
  for (int i = MINX + 1; i < MAXX; ++i) {
    screenGotoxy(i, MINY);
    printf("%c", BOX_HLINE);
  }
  screenGotoxy(MAXX, MINY);
  printf("%c", BOX_UPRIGHT);
  for (int i = MINY + 1; i < MAXY; ++i) {
    screenGotoxy(MINX, i);
    printf("%c", BOX_VLINE);
    screenGotoxy(MAXX, i);
    printf("%c", BOX_VLINE);
  }
  screenGotoxy(MINX, MAXY);
  printf("%c", BOX_DWNLEFT);
  for (int i = MINX + 1; i < MAXX; ++i) {
    screenGotoxy(i, MAXY);
    printf("%c", BOX_HLINE);
  }
  screenGotoxy(MAXX, MAXY);
  printf("%c", BOX_DWNRIGHT);
  screenBoxDisable();
}
void DesignVidaJgdr() {
  screenGotoxy(MINX + 1, MINY + 1);
  printf("  \033[1;34mVida: \033[m");
  for (int i = 0; i < VidaJgdr; ++i) {
    printf("\033[0;31m%s\033[m", CORACAO);
    printf(" ");
  }
}
void IniciarJogo(int Nivel, int reiniciar) {
  InfoJG.Nivel = Nivel;
  InfoJG.TotalInimigos = MAX_INIMIGOS * (Nivel + 1);
  if (!reiniciar) {
    Score = 0;
    VidaJgdr = VIDA_INICIAL;
  }

  Nave.x_Nave = (MAXX - LNAVE) / 2;
  Nave.y_Nave = MAXY - ANAVE - 1;

  for (int i = 0; i < MAX_INIMIGOS; ++i) {
    Inimigos[i].Iniciado = 0;
  }
  for (int i = 0; i < MAX_INIMIGOS; ++i) {
    Inimigos[i].x_NavesInimigas = i * (LINIMIGO + 4) + 1;
    Inimigos[i].y_NavesInimigas = 2;
    Inimigos[i].Iniciado = 1;
  }
  for (int i = 0; i < MAX_TIROS; ++i) {
    Tiros[i].Iniciado = 0;
    Tiros[i].x_Tironave = -1;
    Tiros[i].y_Tironave = -1;
  }

  for (int i = 0; i < MAX_TIROS_INIMIGOS; ++i) {
    TirosInimigos[i].Iniciado = 0;
    TirosInimigos[i].x_tiroInimigo = -1;
    TirosInimigos[i].y_tiroInimigo = -1;
  }
}

void DesignNave() {
  screenGotoxy(Nave.x_Nave, Nave.y_Nave);
  printf("\033[1;34m .⡴⣿⢦.\033[m");
  DesignVidaJgdr();
}

void DesignInimigos() {
  for (int i = 0; i < MAX_INIMIGOS; ++i) {
    if (Inimigos[i].Iniciado && Inimigos[i].Tipo == INIMIGO_UNICO) {
      screenGotoxy(Inimigos[i].x_NavesInimigas, Inimigos[i].y_NavesInimigas);
      printf("\033[1;33m⢈⢝⠭⡫⡁\033[m");
    }
  }
}

void DesignTiros() {
  for (int i = 0; i < MAX_TIROS; ++i) {
    if (Tiros[i].Iniciado) {
      screenGotoxy(Tiros[i].x_Tironave, Tiros[i].y_Tironave);
      printf("\033[1;36m|\033[m");
    }
  }
}

void PercursoNave(int movimento) {
  Nave.x_Nave += movimento;
  if (Nave.x_Nave < MINX) {
    Nave.x_Nave = MINX;
  } else if (Nave.x_Nave + LNAVE > MAXX) {
    Nave.x_Nave = MAXX - LNAVE;
  }
}

void PercursoInimigos() {
  static int count = 0;
  count++;
  for (int i = 0; i < MAX_INIMIGOS; ++i) {
    if (Inimigos[i].Iniciado) {
      Inimigos[i].x_NavesInimigas += Direcao;
      if (count % 10 == 0) {
        Inimigos[i].y_NavesInimigas += Direcao;
      }
      if (Inimigos[i].x_NavesInimigas + LINIMIGO >= MAXX ||
          Inimigos[i].x_NavesInimigas <= MINX) {
        Direcao *= -1;
        for (int j = 0; j < MAX_INIMIGOS; ++j) {
          Inimigos[j].y_NavesInimigas += 1;
        }
      }
    }
  }
}

void PercursoTiros() {
  for (int i = 0; i < MAX_TIROS; ++i) {
    if (Tiros[i].Iniciado) {
      Tiros[i].y_Tironave--;
      if (Tiros[i].y_Tironave <= MINY) {
        Tiros[i].Iniciado = 0;
      }
    }
  }
}

void Colisoes() {

  for (int i = 0; i < MAX_INIMIGOS; ++i) {
    if (Inimigos[i].Iniciado) {
      for (int j = 0; j < MAX_TIROS; ++j) {
        if (Tiros[j].Iniciado &&
            Tiros[j].y_Tironave == Inimigos[i].y_NavesInimigas &&
            Tiros[j].x_Tironave >= Inimigos[i].x_NavesInimigas &&
            Tiros[j].x_Tironave <= Inimigos[i].x_NavesInimigas + LINIMIGO) {
          Inimigos[i].vida--;
          Tiros[j].Iniciado = 0;
          if (Inimigos[i].vida <= 0) {
            Inimigos[i].Iniciado = 0;
            Score += 100;
          }
        }
      }
    }
  }

  for (int i = 0; i < MAX_TIROS_INIMIGOS; ++i) {
    if (TirosInimigos[i].Iniciado && TirosInimigos[i].y_tiroInimigo == Nave.y_Nave &&
        TirosInimigos[i].x_tiroInimigo >= Nave.x_Nave &&
        TirosInimigos[i].x_tiroInimigo <= Nave.x_Nave + LNAVE) {
      VidaJgdr--;
      Score -= 50;
      TirosInimigos[i].Iniciado = 0;
      if (VidaJgdr <= 0) {
        screenClear();
        screenGotoxy(MAXX / 2 - 4, MAXY / 2);
        printf("\033[1;31mGAME OVER\033[m");
        screenUpdate();
        sleep(3);
        exit(0);
      }
    }
  }

  for (int i = 0; i < MAX_INIMIGOS; ++i) {
    if (Inimigos[i].Iniciado &&
        Inimigos[i].y_NavesInimigas + AINIMIGO >= MAXY) {
      screenClear();
      screenGotoxy(MAXX / 2 - 4, MAXY / 2);
      printf("\033[1;31mGAME OVER\033[m");
      screenUpdate();
      sleep(3);
      exit(0);
    }
  }
}

void Atirar() {
  for (int i = 0; i < MAX_TIROS; ++i) {
    if (!Tiros[i].Iniciado) {
      Tiros[i].Iniciado = 1;
      Tiros[i].x_Tironave = Nave.x_Nave + LNAVE / 2;
      Tiros[i].y_Tironave = Nave.y_Nave - 1;
      break;
    }
  }
}

void vitoria() {
  screenClear();
  screenGotoxy(MAXX / 2 - 4, MAXY / 2);
  char escolha;
  printf("\033[1;32mYOU WIN!\n\033[1;34mAperte C para continuar, ou S para "
         "sair:\033[m ");
  scanf(" %c", &escolha);
  if (escolha == 'C' || escolha == 'c') {
    IniciarJogo(InfoJG.Nivel, 1);
  } else {
    screenClear();
    screenUpdate();
    char nome[4];
    printf("\033[1;32mVOCÊ VENCEU!\n\033[1;34mDigite seu nome (3 "
           "caracteres):\033[m ");
    scanf("%3s", nome);
    adicionarPontuacao(nome, Score);
    salvarPontuacaoEmArquivo();
    screenClear();
    screenUpdate();
    mostrarRanking();
    exit(0);
  }
}

int main() {
  screenInit(0);
  keyboardInit();
  timerInit(50);
  int Nivel = 1;

  IniciarJogo(Nivel, 0);
  VidaJgdr = VIDA_INICIAL;
  int TiroInimigoTimer = 0;
  int jogadorDigitouNome = 0;

  carregarPontuacaoDoArquivo();
  while (!jogadorDigitouNome) {
    drawBorders();

    if (timerTimeOver()) {
      PercursoInimigos();
      PercursoTiros();
      PercursoTirosInimigos();
      Colisoes();
      screenClear();
      DesignNave();
      DesignTiros();
      DesignInimigos();
      DesignTirosInimigos();
      screenGotoxy(MAXX - 15, MINY + 1);
      printf("\033[1;34mScore:\033[m \033[1;34m%d\033[m", Score);
      screenUpdate();
    }

    if (InimigosDerrotados()) {
      vitoria();
    }

    TiroInimigoTimer++;
    if (TiroInimigoTimer >= 900) {
      TiroInimigo();
      TiroInimigoTimer = 0;
    }

    if (keyhit()) {
      char letra = readch();
      if (letra == ESQUERDA)
        PercursoNave(-1);
      else if (letra == DIREITA)
        PercursoNave(1);
      else if (letra == ' ') {
        Atirar();
      }
    }

    if (VidaJgdr <= 0) {
      screenClear();
      screenGotoxy(MAXX / 2 - 4, MAXY / 2);
      printf("\033[1;31mGAME OVER\033[m");
      sleep(3);
      exit(0);
    }
  }

  liberarListaPontuacao();
  keyboardDestroy();
  screenDestroy();
  timerDestroy();
  return 0;
}
