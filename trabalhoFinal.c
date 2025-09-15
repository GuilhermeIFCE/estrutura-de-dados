#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Livro {
    char isbn[14];       // Formato: 978-85-359-9999-9
    char titulo[100];
    char autor[50];
    int ano_publicacao;
    int disponivel;      // 1 para disponível, 0 para emprestado
} Livro;

typedef struct NoAVL {
    Livro* livro;
    struct NoAVL* esq;
    struct NoAVL* dir;
    int altura;
} NoAVL;

int maior(int a, int b) { // Retorna o maior valor entre dois inteiros (a e b). Usada para atualizar a altura dos nós da árvore.
    return (a > b) ? a : b;
}

int altura(NoAVL* raiz) { // Retorna a altura de um nó da árvore. Se o nó for NULL, retorna 0.
    if (raiz == NULL)
        return 0;
    return raiz->altura;
}

NoAVL* mesq_livro(NoAVL* raiz) { // Procura o menor nó, o nó mais à esquerda
    NoAVL* temp = raiz;
    while (temp && temp->esq != NULL)
        temp = temp->esq;
    return temp;
}

int fbalanceamento(NoAVL *raiz){ // Calcula o fator de balanceamento de um nó
    if(!raiz)
        return 0;
    return (altura(raiz->esq)- altura(raiz->dir));
}

void rotar_esq(NoAVL **raiz){ //Faz uma rotação simples à esquerda no nó apontado por "a". Usada quando a árvore está desbalanceada para a direita.
    if (*raiz == NULL || (*raiz)->dir == NULL) return;

    NoAVL* b = (*raiz)->dir;
    NoAVL* c = b->esq;

    b->esq = *raiz;
    (*raiz)->dir = c;

    (*raiz)->altura = 1 + maior(altura((*raiz)->esq), altura((*raiz)->dir));
    b->altura = 1 + maior(altura(b->esq), altura(b->dir));

    *raiz = b;
}

void rotar_dir(NoAVL **raiz){ // Faz uma rotação simples à direita no nó apontado por "a". Usada quando a árvore está desbalanceada para a esquerda.
    if (*raiz == NULL || (*raiz)->esq == NULL) return;

    NoAVL* b = (*raiz)->esq;
    NoAVL* c = b->dir;

    b->dir = *raiz;
    (*raiz)->esq = c;

    (*raiz)->altura = 1 + maior(altura((*raiz)->esq), altura((*raiz)->dir));
    b->altura = 1 + maior(altura(b->esq), altura(b->dir));

    *raiz = b;
}

Livro* gerarLivro(char* isbn,char* titulo,char* autor,int ano_publicacao,int disponivel){ // Cria dinamicamente um novo livro e retorna um ponteiro para a estrutura criada.
    Livro* novo=(Livro*)malloc(sizeof(Livro));
    if(!novo){
        printf("Erro ao criar Livro\n");
        exit(1);
    }

    strcpy(novo->isbn, isbn);
    strcpy(novo->titulo, titulo);
    strcpy(novo->autor, autor);
    novo->ano_publicacao=ano_publicacao;
    novo->disponivel=disponivel;

    return novo;
}

NoAVL* inserirlivro(NoAVL** raiz, Livro *livro){ // Insere um livro na árvore AVL, mantendo a ordenação por ISBN. Após inserir, verifica o fator de balanceamento e realiza rotações, se necessário.
    if (*raiz == NULL) {
        *raiz  = (NoAVL*)malloc(sizeof(NoAVL));
        (*raiz)->livro = livro;
        (*raiz)->esq = NULL;
        (*raiz)->dir = NULL;
        (*raiz)->altura = 1;
        return *raiz;
    }

    if(strcmp(livro->isbn, (*raiz)->livro->isbn) < 0){
        (*raiz)->esq = inserirlivro(&((*raiz)->esq), livro);
    }
    else if(strcmp(livro->isbn, (*raiz)->livro->isbn) > 0){
        (*raiz)->dir = inserirlivro(&((*raiz)->dir), livro);
    }
    else{
        return *raiz; // ISBN igual, não insere
    }

    (*raiz)->altura = 1 + maior(altura((*raiz)->esq), altura((*raiz)->dir));

    int fb = fbalanceamento(*raiz);

    if (fb > 1 && strcmp(livro->isbn, (*raiz)->esq->livro->isbn) < 0) {
        rotar_dir(raiz);
        return *raiz;
    }
    if (fb < -1 && strcmp(livro->isbn, (*raiz)->dir->livro->isbn) > 0) {
        rotar_esq(raiz);
        return *raiz;
    }
    if (fb > 1 && strcmp(livro->isbn, (*raiz)->esq->livro->isbn) > 0) {
        rotar_esq(&(*raiz)->esq);
        rotar_dir(raiz);
        return *raiz;
    }
    if (fb < -1 && strcmp(livro->isbn, (*raiz)->dir->livro->isbn) < 0) {
        rotar_dir((&(*raiz)->dir));
        rotar_esq(raiz);
        return *raiz;
    }

    return *raiz;
}

void remover(NoAVL** raiz, char* isbn) { // Remove um livro da árvore AVL pelo ISBN. Depois da remoção, a árvore é rebalanceada. 
    // Caso base: verifica se a árvore está vazia
    if (*raiz == NULL) return;

    // Percorre a árvore para encontrar o nó a remover
    if (strcmp(isbn, (*raiz)->livro->isbn) < 0){  // Busca se está na esquerda
        remover(&(*raiz)->esq, isbn); 
    } else if (strcmp(isbn, (*raiz)->livro->isbn) > 0){  // Busca se está na direita
        remover(&(*raiz)->dir, isbn);
    } else {
        NoAVL* temp;
        // Nó com 0 ou 1 filho → substitui o nó pelo filho.
        if (!(*raiz)->esq || !(*raiz)->dir) {
            temp = (*raiz)->esq ? (*raiz)->esq : (*raiz)->dir;
            
            if (!temp) { // 0 filhos, libera memória e zera o ponteiro
                free((*raiz)->livro); 
                free(*raiz);
                *raiz = NULL;
            } else { // 1 filho, copia o nó para um temporário e libera memória
                NoAVL *antigo = *raiz;
                *raiz = temp;
                free(antigo->livro);
                free(antigo);
            }
        } else {
            // Nó com 2 filhos
            temp = mesq_livro((*raiz)->dir); // Substitui pelo menor nó da subárvore direita
            *(*raiz)->livro = *temp->livro; // Copia os dados do sucessor para o nó atual
            remover(&(*raiz)->dir, temp->livro->isbn); // Remove o nó duplicado da subárvore direita
        }
    }

    // Verifica se o nó virou NULL após a remoção
    if(!*raiz) return;

    // Atualiza a altura do nó atual
    (*raiz)->altura = 1 + maior(altura((*raiz)->esq), altura((*raiz)->dir));

    // Calcula o fator de balancemaneto
    int bal = fbalanceamento(*raiz);

    // Caso LL
    if (bal > 1 && fbalanceamento((*raiz)->esq) >= 0) 
        rotar_dir(raiz);
    
    // Caso LR
    if (bal > 1 && fbalanceamento((*raiz)->esq) < 0) { 
        rotar_esq(&(*raiz)->esq); 
        rotar_dir(raiz); 
    }

    // Caso RR
    if (bal < -1 && fbalanceamento((*raiz)->dir) <= 0) 
        rotar_esq(raiz);

    // Caso RL
    if (bal < -1 && fbalanceamento((*raiz)->dir) > 0) { 
        rotar_dir(&(*raiz)->dir); 
        rotar_esq(raiz); 
    }
}

Livro* buscarLivro(NoAVL* raiz, char* isbn) {
    if (raiz == NULL)
        return NULL;

    int cmp = strcmp(isbn, raiz->livro->isbn);

    if (cmp == 0){
        return raiz->livro;
    }
    else if (cmp < 0){
        return buscarLivro(raiz->esq, isbn);
    }
    else{
        return buscarLivro(raiz->dir, isbn);
    }
}

void emprestarLivro(NoAVL* raiz, char* isbn) {
    Livro* livro = buscarLivro(raiz, isbn);

    if (livro == NULL) {
        printf("Livro com ISBN %s não encontrado!\n", isbn);
        return;
    }

    if (livro->disponivel == 1) {
        livro->disponivel = 0;
        printf("Livro \"%s\" emprestado com sucesso!\n", livro->titulo);
    } else {
        printf("O livro \"%s\" já está emprestado!\n", livro->titulo);
    }
}

void devolverLivro(NoAVL* raiz, char* isbn) {
    Livro* livro = buscarLivro(raiz, isbn);

    if (livro == NULL) {
        printf("Livro com ISBN %s não encontrado!\n", isbn);
        return;
    }

    if (livro->disponivel == 0) {
        livro->disponivel = 1;
        printf("Livro \"%s\" devolvido com sucesso!\n", livro->titulo);
    } else {
        printf("O livro \"%s\" já estava disponível!\n", livro->titulo);
    }
}

void exibir(NoAVL* raiz){
    if(!raiz) return;
    exibir(raiz->esq);
    printf("%s -> ", raiz->livro->isbn);
    exibir(raiz->dir);
}

void livrosDisponiveis(NoAVL* raiz){ //Percorre em ordem e se estiver disponível faz o print
    if(!raiz) return;
    livrosDisponiveis(raiz->esq);
    if(raiz->livro->disponivel == 1){
        printf(" %s disponivel \n",raiz->livro->isbn);
    }
    livrosDisponiveis(raiz->dir);
}

// Função para contar o total de nós (livros)
int qtdLivros(NoAVL *raiz) {
    if (!raiz) return 0;
    return 1 + qtdLivros(raiz->esq) + qtdLivros(raiz->dir);
}

int main() {
    NoAVL *raiz = NULL;
    int opc;
    int qtd, alt;
    char isbn[14];
    while(1){
        printf("\n\nDigite a opção desejada:\n");
        printf("1 - Inserir Livro\n2 - Remover\n3 - Buscar Livro por ISBN\n4 - Listar Todos os Livros(Crescente)\n5 - Emprestar\n6 - Devolver\n7 - Estatísticas\n8 - Livros Disponíveis\n9 - Sair\nEscolha: ");
        scanf("%d",&opc);
        getchar();

        if(opc==1){
            char titulo[100],autor[50];
            int ano;
            printf("Digite a isbn do livro: ");
            fgets(isbn,14,stdin);
            isbn[strcspn(isbn, "\n")] = '\0';

            printf("Digite o titulo do livro: ");
            fgets(titulo,100,stdin);
            titulo[strcspn(titulo, "\n")] = '\0';

            printf("Digite o autor do livro: ");
            fgets(autor,50,stdin);
            autor[strcspn(autor, "\n")] = '\0';

            printf("Digite o ano do livro: ");
            scanf("%d", &ano);

            Livro* livro = gerarLivro(isbn, titulo, autor, ano, 1);
            raiz = inserirlivro(&raiz, livro);
            printf("Livro inserido na arvore!\n");
        }else if(opc == 2){
            printf("Digite o código do livro que deseja remover: ");
            fgets(isbn, 14, stdin);
            isbn[strcspn(isbn, "\n")] = '\0';
            remover(&raiz, isbn);
        }else if(opc == 3){
            printf("Digite o ISBN do livro que deseja buscar: ");
            fgets(isbn, 14, stdin);
            isbn[strcspn(isbn, "\n")] = '\0';
            Livro* l = buscarLivro(raiz, isbn);
            if(l) printf("Livro encontrado: %s - %s\n", l->isbn, l->titulo);
            else printf("Livro não encontrado!\n");
        }else if(opc == 4){
            printf("Livros cadastrados:\n");
            exibir(raiz);
            printf("\n");
        }else if(opc == 5){
            printf("Digite o ISBN do livro a emprestar: ");
            fgets(isbn, 14, stdin);
            isbn[strcspn(isbn, "\n")] = '\0';
            emprestarLivro(raiz, isbn);
        }else if(opc == 6){
            printf("Digite o ISBN do livro a devolver: ");
            fgets(isbn, 14, stdin);
            isbn[strcspn(isbn, "\n")] = '\0';
            devolverLivro(raiz, isbn);
        }else if(opc == 7){
            qtd = qtdLivros(raiz);
            printf("Quantidade de livros disponíveis: %d\n", qtd);
            alt = altura(raiz);
            printf("Altura da arvore: %d\n", alt);
        }else if(opc == 8){
            livrosDisponiveis(raiz);
        }else if(opc == 9){
            break;
        }
    }
    return 0;
}