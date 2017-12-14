//*********************************************************************************
// Trabalho 2 - Organizacao e Recuperacao de Arquivos - 2017/2
// Docente: Prof. Dr. Ricardo Rodrigues Ciferri
// Discentes: Andre Camargo Rocha RA:587052 e Sergio Hideki RA:551848
//*********************************************************************************

#include <iostream>
#include <fstream>
#include <string.h>
#include <time.h>
#include <cstdlib>
#include <stdio.h>

struct registro
{                    // registro de tamanho fixo de 64 bytes
    char modelo[16]; // 16 bytes (15 + 1 para o '\0')
    char marca[19];  // 19 bytes (18 + 1 para o '\0')
    char pais[19];   // 19 bytes (18 + 1 para o '\0')
    char ano[5];     // 5 bytes (tipo 'int' estava vindo com 8 bytes por causa do compilador) (4 + 1 para o '\0')
    char pot[5];     // 5 bytes (4 + 1 para o '\0')
};

struct bloco
{ // bloco de tamanho fixo de 512 bytes (8 registros de 64 bytes cada)
    registro reg[8];
};

struct blocoAl
{ // bloco de tamanho fixo de 1024 bytes (16 registros de 64 bytes cada)
    registro reg[16];
};

typedef struct
{
    int bloco;
    int pos;
    char key[16];
} Index;

//*********************************************************************************

void criaArquivo();                                            // Cria novo arquivo binario para trabalhar com os dados
void insereBloco(FILE *arquivo);                               // Insere um novo bloco de 512 bytes no final do arquivo
void insereRegistro();                                         // Insere um novo registro de 64 bytes no primeiro espaço vazio encontrado em um bloco
void buscaRegistroNoIndex();                                   // Lista determinado registro dada uma chave (Modelo)
void listaRegistros();                                         // Lista todos os registros do arquivo de dados
void listaArquivo();                                           // Lista o arquivo
void removeRegistroPeloIndex();                                // Remove determinado registro dada uma chave (Modelo)
void insereBlocoAux(registro reg);                             // Faz insercao de registro mas recebendo como parametro
void insereLote();                                             // Insere um lote de carros aleatoriamente
void criaArquivoIndex();                                       // Cria novo arquivo binario para trabalhar com os indices
bool insereIndex(char modeloAux[16], int pox, int qntdblocos); // Insere ordenado novo index, passado por parametro após inserir registro
Index buscarNoIndexFile(char modelobuscado[16]);               // Retorna um índice caso encontrado, a partir da chave passada por parametro
void removeByIndex();                                          // Remove um dado começando pelo arquivo de indíces
bool treatAfterRemove(Index idx);                              // Realiza offset e tratamentos necessários após remoção de dados
void menu();                                                   // Imprime o menu de opcoes na tela
//*********************************************************************************

void criaArquivo()
{ //Cria arquivo e insere um bloco(8 registros) completamente vazio
    struct bloco blocoVazio;
    FILE *arquivo;
    int i;
    arquivo = fopen("carros.dat", "wb");

    if (!arquivo)
    { // verifica se nao ha erro na abertura do arquivo
        printf("Arquivo de dadosnao pode ser aberto para gravacao!\n\n");
    }
    else
    {
        printf("Arquivo de dados criado com sucesso!\n\n");
        criaArquivoIndex();
    }
    fclose(arquivo);
    system("PAUSE");
    return;
}

void criaArquivoIndex()
{ //Cria arquivo completamente vazio
    FILE *arquivo;
    int i;
    arquivo = fopen("index.dat", "wb");

    if (!arquivo)
    { // verifica se nao ha erro na abertura do arquivo
        printf("Arquivo de index nao pode ser aberto para gravacao!\n\n");
    }
    else
    {
        printf("Arquivo de index criado com sucesso!\n\n");
    }
    fclose(arquivo);
    return;
}

void insereBloco(FILE *arquivo)
{ // Abre o arquivo e insere um bloco vazio no final - usada caso todos os blocos estejam cheios
    struct bloco blocoVazio;
    int i;

    if (!arquivo)
    { // verifica se houve erro ao abrir o arquivo para escrita
        printf("Arquivo nao pode ser aberto para gravacao!\n\n");
        system("PAUSE");
        return;
    }
    for (i = 0; i < 8; i++)
    {
        strcpy(blocoVazio.reg[i].modelo, "#"); // preenche cada registro com '#' para mostrar que está vazio
    }
    fwrite(&blocoVazio, sizeof(blocoVazio), 1, arquivo);
}

void insereRegistro()
{
    FILE *arquivo;
    arquivo = fopen("carros.dat", "r+b"); // cria arquivo para leitura e escrita em modo binario

    char modeloAux[16];

    bloco blocoAux;
    int inserir = 0;
    int posicaoinsere = -1;
    int qntdblocos = 0;

    char buffer;

    if (!arquivo)
    { // verifica se houve erro ao abrir o arquivo
        printf("Arquivo nao pode ser aberto!\n\n");
        system("PAUSE");
        return;
    }

    while (inserir != 1)
    { // estamos buscando uma posiçao que possa ser inserido o novo registro
        int fim = fread(&blocoAux, sizeof(registro), 8, arquivo);
        fseek(arquivo, sizeof(blocoAux) * (qntdblocos++), SEEK_SET); // andando de bloco em bloco com o ponteiro de leitura
        if (fim == 0)
        {                                                //caso nada tenha sido retornado na leitura (EOF ou arquivo vazio) entao inserimos um novo bloco
            insereBloco(arquivo);                        // insere novo bloco vazio no final do arquivo pois nao existem opcoes livres para insercao
            fseek(arquivo, -sizeof(blocoAux), SEEK_END); // retorna o ponteiro para a posicao inicial do bloco inserido
        }
        fread(&blocoAux, sizeof(registro), 8, arquivo); //recupera o bloco todo (em 8 registros) para a variavel blocoAux
        for (int i = 0; i < 8; i++)
        {
            if (blocoAux.reg[i].modelo[0] == '#')
            {
                posicaoinsere = i; // posicao do registro a ser preenchido dentro do bloco
                inserir = 1;
                break;
            }
        }
    }

    //Coletando os campos de cada registro que vai ser inserido
    printf("Digite o modelo do carro: ");
    for (int i = 0; i < 15; i++)
    {

        buffer = getc(stdin); // recebe entrada do teclado e joga caractere por caractere para o buffer
        if (buffer == '\n')
        { // caso o usuario aperte ENTER, encerramos a string com o caractere NUL(\0)
            blocoAux.reg[posicaoinsere].modelo[i] = '\0';
            break; //saimos do laco de repeticao "for"
        }
        if (i == 14)
        { // ignoramos todo o conteudo do buffer que esteja fora do tamanho do campo no registro
            while (getc(stdin) != '\n')
            {
                continue;
            }
        }
        blocoAux.reg[posicaoinsere].modelo[i] = buffer; //pega o caractere apontado pelo ponteiro de leitura e atribui para modelo[i]
                                                        //modeloAux[i] = buffer;
    }
    blocoAux.reg[posicaoinsere].modelo[14] = '\0';
    strcpy(modeloAux, blocoAux.reg[posicaoinsere].modelo);

    printf("Digite a marca do carro: ");
    for (int i = 0; i < 18; i++)
    {

        buffer = getc(stdin); // recebe entrada do teclado e joga caractere por caractere para o buffer
        if (buffer == '\n')
        { // caso o usuario aperte ENTER, encerramos a string com o caractere NUL(\0)
            blocoAux.reg[posicaoinsere].marca[i] = '\0';
            break; //saimos do laco de repeticao "for"
        }
        if (i == 17)
        { // ignoramos todo o conteudo do buffer que esteja fora do tamanho do campo no registro
            while (getc(stdin) != '\n')
            {
                continue;
            }
        }
        blocoAux.reg[posicaoinsere].marca[i] = buffer; //pega o caractere apontado pelo ponteiro de leitura e atribui para modelo[i]
    }
    blocoAux.reg[posicaoinsere].marca[17] = '\0';

    printf("Digite o pais de fabricacao do carro: ");
    for (int i = 0; i < 18; i++)
    {

        buffer = getc(stdin); // recebe entrada do teclado e joga caractere por caractere para o buffer
        if (buffer == '\n')
        { // caso o usuario aperte ENTER, encerramos a string com o caractere NUL(\0)
            blocoAux.reg[posicaoinsere].pais[i] = '\0';
            break; //saimos do laco de repeticao "for"
        }
        if (i == 17)
        { // ignoramos todo o conteudo do buffer que esteja fora do tamanho do campo no registro
            while (getc(stdin) != '\n')
            {
                continue;
            }
        }
        blocoAux.reg[posicaoinsere].pais[i] = buffer; //pega o caractere apontado pelo ponteiro de leitura e atribui para modelo[i]
    }
    blocoAux.reg[posicaoinsere].pais[17] = '\0';

    printf("Digite o ano de fabricacao do carro: ");
    for (int i = 0; i < 5; i++)
    {

        buffer = getc(stdin); // recebe entrada do teclado e joga caractere por caractere para o buffer
        if (buffer == '\n')
        { // caso o usuario aperte ENTER, encerramos a string com o caractere NUL(\0)
            blocoAux.reg[posicaoinsere].ano[i] = '\0';
            break; //saimos do laco de repeticao "for"
        }
        if (i == 4)
        { // ignoramos todo o conteudo do buffer que esteja fora do tamanho do campo no registro
            while (getc(stdin) != '\n')
            {
                continue;
            }
        }
        blocoAux.reg[posicaoinsere].ano[i] = buffer; //pega o caractere apontado pelo ponteiro de leitura e atribui para modelo[i]
    }
    blocoAux.reg[posicaoinsere].ano[4] = '\0';

    printf("Digite a potencia do carro: ");
    for (int i = 0; i < 5; i++)
    {

        buffer = getc(stdin); // recebe entrada do teclado e joga caractere por caractere para o buffer
        if (buffer == '\n')
        { // caso o usuario aperte ENTER, encerramos a string com o caractere NUL(\0)
            blocoAux.reg[posicaoinsere].pot[i] = '\0';
            break; //saimos do laco de repeticao "for"
        }
        if (i == 4)
        { // ignoramos todo o conteudo do buffer que esteja fora do tamanho do campo no registro
            while (getc(stdin) != '\n')
            {
                continue;
            }
        }
        blocoAux.reg[posicaoinsere].pot[i] = buffer; //pega o caractere apontado pelo ponteiro de leitura e atribui para modelo[i]
    }
    blocoAux.reg[posicaoinsere].pot[4] = '\0';

    //posiciona o ponteiro do arquivo na posicao onde o bloco vai ser inserido
    fseek(arquivo, sizeof(bloco) * (qntdblocos - 1), SEEK_SET);

    //escreve todo o bloco no arquivo
    fwrite(&blocoAux, sizeof(registro), 8, arquivo);
    insereIndex(modeloAux, posicaoinsere, qntdblocos - 1); //Insere informacoes do registro no arquivo de indices

    //fecha o arquivo
    fclose(arquivo);
    return;
}

bool insereIndex(char modeloAux[16], int pos, int qntdblocos)
{
    FILE *arquivo;
    arquivo = fopen("index.dat", "a+b"); // cria arquivo para leitura e escrita em modo binario

    fseek(arquivo, 0, SEEK_END); //posicionando ponteiro de leitura no fim do arquivo
    int idxLength = (ftell(arquivo) / sizeof(Index));
    fseek(arquivo, 0, SEEK_SET); //reposicionando ponteiro de leitura no comeco do arquivo

    if (!arquivo)
    { // verifica se houve erro ao abrir o arquivo
        printf("Arquivo de indices nao pode ser aberto!\n\n");
        system("PAUSE");
    }

    Index *idxReader = (Index *)malloc(idxLength * sizeof(Index));
    int lido = 0;
    lido = fread(idxReader, sizeof(Index), idxLength, arquivo);
    fclose(arquivo);
    if (lido == 0)
    {
        arquivo = fopen("index.dat", "wb");
        free(idxReader);
        idxReader = (Index *)malloc(24);
        strcpy(idxReader[0].key, modeloAux);
        idxReader[0].bloco = qntdblocos;
        idxReader[0].pos = pos;
        int inseriu = fwrite(idxReader, sizeof(Index), 1, arquivo);
        if (inseriu > 0)
        {
            printf("Registro e index inseridos com sucesso \n");
            system("PAUSE");
            free(idxReader);
            fclose(arquivo);
            return true;
        }
        printf("Index nao inserido!! \n");
        system("PAUSE");
        free(idxReader);
        fclose(arquivo);
        return false;
    }
    int inserir;
    bool achou = false;

    if(idxLength == 1){
        idxLength = 2;
    }

    for (int i = 0; i < idxLength; i++)
    {
        if (strcmp(modeloAux, idxReader[i].key) < 0)
        {
            inserir = i;
            achou = true;
        }
    }
    if (achou == false)
    {
        inserir = idxLength;
    }

    Index idxWriter[idxLength];
    int flag = 0;
    for (int i = 0; i < idxLength + 1; i++)
    {
        if(flag)
        {
            strcpy(idxWriter[i].key, idxReader[i - 1].key);
            idxWriter[i].pos = idxReader[i - 1].pos;
            idxWriter[i].bloco = idxWriter[i - 1].bloco;
        }
        else
        {
            strcpy(idxWriter[i].key, idxReader[i].key);
            idxWriter[i].pos = idxReader[i].pos;
            idxWriter[i].bloco = idxWriter[i].bloco;
        }
        if(i == inserir)
        {
            flag = 1;
            strcpy(idxWriter[i].key, modeloAux);
            idxWriter[i].bloco = qntdblocos;
            idxWriter[i].pos = pos;
        }
    }
    arquivo = fopen("index.dat", "wb");
    int inseriu = fwrite(idxWriter, sizeof(Index), idxLength + 1, arquivo);
    if (inseriu > 0)
    {
        printf("Registro e index inseridos com sucesso \n");
        system("PAUSE");
        free(idxReader);
        fclose(arquivo);
        return true;
    }
    printf("Index não inserido!! \n");
    system("PAUSE");
    free(idxReader);
    fclose(arquivo);
    return false;
}

Index buscarNoIndexFile(char modelobuscado[16])
{
    FILE *arquivo;
    arquivo = fopen("index.dat", "rb");

    fseek(arquivo, 0, SEEK_END); //posicionando ponteiro de leitura no fim do arquivo
    int idxLength = (ftell(arquivo) / sizeof(Index));
    fseek(arquivo, 0, SEEK_SET); //reposicionando ponteiro de leitura no comeco do arquivo

    Index *idxReader = (Index *)malloc(idxLength * sizeof(Index));
    int lido = 0;
    lido = fread(idxReader, sizeof(Index), idxLength, arquivo);
    char buffer;

    if (!arquivo)
    { // verifica se houve erro ao abrir o arquivo
        printf("Arquivo de indices nao pode ser aberto!\n\n");
        system("PAUSE");
        idxReader[0].pos = -1;
        idxReader[0].bloco = -1;
        return idxReader[0];
    }

    if (!lido)
    {
        idxReader[0].pos = -1;
        idxReader[0].bloco = -1;
        return idxReader[0];
    }

    for (int i = 0; i < idxLength; i++)
    {
        if (strcmp(idxReader[i].key, modelobuscado) == 0)
        {
            printf("Index encontrado: \n");
            printf("idxReader[i].key: %s\n", idxReader[i].key);
            printf("idxReader[i].bloco: %d\n", idxReader[i].bloco);
            printf("idxReader[i].pos: %d \n", idxReader[i].pos);
            system("PAUSE");
            return idxReader[i];
        }
    }
    idxReader[0].pos = -1;
    idxReader[0].bloco = -1;
    return idxReader[0];
}

void buscaRegistroNoIndex()
{
    FILE *arquivo;
    arquivo = fopen("carros.dat", "rb");

    bloco blocoAux;
    int lido = 0;
    int posicaoreg = 0;
    int qntdblocos = 0;
    int encontrado = 0;
    char modelobuscado[16];
    char buffer;

    printf("Digite o modelo do carro buscado: ");
    for (int i = 0; i < 15; i++)
    {

        buffer = getc(stdin); // recebe entrada do teclado e joga caractere por caractere para o buffer
        if (buffer == '\n')
        { // caso o usuario aperte ENTER, encerramos a string com o caractere NUL(\0)
            modelobuscado[i] = '\0';
            break; //saimos do laco de repeticao "for"
        }
        if (i == 14)
        { // ignoramos todo o conteudo do buffer que esteja fora do tamanho do campo no registro
            while (getc(stdin) != '\n')
            {
                continue;
            }
        }
        modelobuscado[i] = buffer; //pega o caractere apontado pelo ponteiro de leitura e atribui para modelo[i]
    }
    modelobuscado[14] = '\0';

    if (!arquivo)
    {
        printf("Arquivo nao pode ser aberto!\n\n");
        system("PAUSE");
        fclose(arquivo);
        return;
    }

    Index idxReader = buscarNoIndexFile(modelobuscado);

    if(idxReader.bloco != -1){
        fseek(arquivo, sizeof(bloco) * idxReader.bloco, SEEK_SET); //posicionando ponteiro de leitura no comeco do bloco a ser lido
        lido = fread(&blocoAux, sizeof(registro), 8, arquivo);     //lendo 8 registros(1 bloco) do arquivo;

        if (lido == 0)
        {
            printf("\n\nNao existe registros para o modelo buscado!\n\n");
            system("PAUSE");
            fclose(arquivo);
            return;
        }

        printf("\n Registro Encontrado: \n");
        printf("\n\tModelo: %s\n\t", blocoAux.reg[idxReader.pos].modelo);
        printf("\n\tMarca: %s\n\t", blocoAux.reg[idxReader.pos].marca);
        printf("\n\tPais de Fabricacao: %s\n\t", blocoAux.reg[idxReader.pos].pais);
        printf("\n\tAno: %s\n\t", blocoAux.reg[idxReader.pos].ano);
        printf("\n\tPotencia: %scv\n\n", blocoAux.reg[idxReader.pos].pot);

        printf("\n\nRegistro encontrado com sucesso!\n\n");
        fclose(arquivo);
        system("PAUSE");
        return;
    }

    printf("\n\nNao existe registros para o modelo buscado!\n\n");
    fclose(arquivo);
    system("PAUSE");
    return;
}

void listaRegistros()
{
    FILE *arquivo;
    arquivo = fopen("carros.dat", "rb");

    bloco blocoAux;
    int posicaoreg = 0;
    int qntdblocos = 0;
    int lido = 1;

    if (!arquivo)
    {
        printf("Arquivo nao pode ser aberto!\n\n");
        system("PAUSE");
        return;
    }

    do
    {
        fseek(arquivo, sizeof(bloco) * (qntdblocos++), SEEK_SET); //posicionando ponteiro de leitura no comeco do bloco a ser lido
        lido = fread(&blocoAux, sizeof(registro), 8, arquivo);    //lendo 8 registros(1 bloco) do arquivo;

        if (lido == 0)
        {
            printf("\n\nNao existem mais registros para serem listados!\n");
            system("PAUSE");
            return;
        }

        for (int i = 0; i < 8; i++)
        {
            if (blocoAux.reg[i].modelo[0] != '#')
            {
                printf("\n----------------------------------------------------------\n");
                printf("\n\tModelo: %s\n\t", blocoAux.reg[i].modelo);
                printf("\n\tMarca: %s\n\t", blocoAux.reg[i].marca);
                printf("\n\tPais de Fabricacao: %s\n\t", blocoAux.reg[i].pais);
                printf("\n\tAno: %s\n\t", blocoAux.reg[i].ano);
                printf("\n\tPotencia: %scv\n\n", blocoAux.reg[i].pot);
            }
        }
    } while (lido != 0);

    fclose(arquivo);
    system("PAUSE");
    return;
}

void listaArquivo()
{
    FILE *arquivo;
    arquivo = fopen("carros.dat", "rb");

    FILE *arquivo2;
    arquivo2 = fopen("carros2.dat", "rb");

    bloco blocoAux;
    bloco bloco2;
    bloco blocoNovo;
    int posicaoreg = 0;
    int qntdblocos = 0;
    int lido = 1;
    int c = 0;
    int inserir = 0;
    int posicaoinsere = -1;

    if (!arquivo)
    {
        printf("Arquivo nao pode ser aberto!\n\n");
        system("PAUSE");
        return;
    }

    if (!arquivo2)
    {
        printf("Arquivo nao pode ser aberto!\n\n");
        system("PAUSE");
        return;
    }

    do
    {
        fseek(arquivo, sizeof(bloco) * (qntdblocos++), SEEK_SET); //posicionando ponteiro de leitura no comeco do bloco a ser lido
        lido = fread(&blocoAux, sizeof(registro), 8, arquivo);    //lendo 8 registros(1 bloco) do arquivo;

        for (int i = 0; i < 8; i++)
        {
            if (blocoAux.reg[i].modelo[0] != '#')
            {
                strcpy(bloco2.reg[c].modelo, blocoAux.reg[i].modelo);
                strcpy(bloco2.reg[c].marca, blocoAux.reg[i].marca);
                strcpy(bloco2.reg[c].pais, blocoAux.reg[i].pais);
                strcpy(bloco2.reg[c].ano, blocoAux.reg[i].ano);
                strcpy(bloco2.reg[c].pot, blocoAux.reg[i].pot);
                c++;
                if (c = 8)
                {
                    while (inserir != 1)
                    { // estamos buscando uma posiçao que possa ser inserido o novo registro
                        int fim = fread(&blocoNovo, sizeof(registro), 8, arquivo2);
                        fseek(arquivo2, sizeof(blocoNovo) * (qntdblocos++), SEEK_SET); // andando de bloco em bloco com o ponteiro de leitura
                        if (fim == 0)
                        {                                                  //caso nada tenha sido retornado na leitura (EOF ou arquivo vazio) entao inserimos um novo bloco
                            insereBloco(arquivo2);                         // insere novo bloco vazio no final do arquivo pois nao existem opcoes livres para insercao
                            fseek(arquivo2, -sizeof(blocoNovo), SEEK_END); // retorna o ponteiro para a posicao inicial do bloco inserido
                        }
                        fread(&blocoNovo, sizeof(registro), 8, arquivo2); //recupera o bloco todo (em 8 registros) para a variavel blocoAux
                        for (int i = 0; i < 8; i++)
                        {
                            if (blocoNovo.reg[i].modelo[0] == '#')
                            {
                                posicaoinsere = i; // posicao do registro a ser preenchido dentro do bloco
                                inserir = 1;
                                break;
                            }
                        }
                    }
                    //começa a preencher o bloco do arquivo novo
                    for (int cont = 0; c < 8; c++)
                    {
                        strcpy(blocoNovo.reg[cont].modelo, bloco2.reg[cont].modelo);
                        strcpy(blocoNovo.reg[cont].marca, bloco2.reg[cont].marca);
                        strcpy(blocoNovo.reg[cont].pais, bloco2.reg[cont].pais);
                        strcpy(blocoNovo.reg[cont].ano, bloco2.reg[cont].ano);
                        strcpy(blocoNovo.reg[cont].pot, bloco2.reg[cont].pot);
                    }

                    //posiciona o ponteiro do arquivo na posicao onde o bloco vai ser inserido
                    fseek(arquivo2, sizeof(blocoNovo) * (qntdblocos - 1), SEEK_SET);

                    //escreve todo o bloco no arquivo
                    fwrite(&blocoNovo, sizeof(registro), 8, arquivo2);

                    //fecha o arquivo
                    fclose(arquivo2);

                    //volta pra 0, pra não cair no if(c = 8)
                    c = 0;
                    inserir = 0;
                }
                //no caso de ter acabado de ler o arquivo e não ter completado o bloco
                if (lido == 0)
                {
                    while (inserir != 1)
                    { // estamos buscando uma posiçao que possa ser inserido o novo registro
                        int fim = fread(&blocoNovo, sizeof(registro), 8, arquivo2);
                        fseek(arquivo2, sizeof(blocoNovo) * (qntdblocos++), SEEK_SET); // andando de bloco em bloco com o ponteiro de leitura
                        if (fim == 0)
                        {                                                  //caso nada tenha sido retornado na leitura (EOF ou arquivo vazio) entao inserimos um novo bloco
                            insereBloco(arquivo2);                         // insere novo bloco vazio no final do arquivo pois nao existem opcoes livres para insercao
                            fseek(arquivo2, -sizeof(blocoNovo), SEEK_END); // retorna o ponteiro para a posicao inicial do bloco inserido
                        }
                        fread(&blocoNovo, sizeof(registro), 8, arquivo2); //recupera o bloco todo (em 8 registros) para a variavel blocoAux
                        for (int i = 0; i < c; i++)
                        {
                            if (blocoNovo.reg[i].modelo[0] == '#')
                            {
                                posicaoinsere = i; // posicao do registro a ser preenchido dentro do bloco
                                inserir = 1;
                                break;
                            }
                        }
                    }
                    //começa a preencher o bloco do arquivo novo
                    for (int cont = 0; cont < 8; c++)
                    {
                        strcpy(blocoNovo.reg[cont].modelo, bloco2.reg[cont].modelo);
                        strcpy(blocoNovo.reg[cont].marca, bloco2.reg[cont].marca);
                        strcpy(blocoNovo.reg[cont].pais, bloco2.reg[cont].pais);
                        strcpy(blocoNovo.reg[cont].ano, bloco2.reg[cont].ano);
                        strcpy(blocoNovo.reg[cont].pot, bloco2.reg[cont].pot);
                    }

                    //posiciona o ponteiro do arquivo na posicao onde o bloco vai ser inserido
                    fseek(arquivo2, sizeof(blocoNovo) * (qntdblocos - 1), SEEK_SET);

                    //escreve todo o bloco no arquivo
                    fwrite(&blocoNovo, sizeof(registro), 8, arquivo2);

                    //fecha o arquivo
                    fclose(arquivo2);

                    //volta pra 0, pra não cair no if(c = 8)
                    c = 0;
                }
                inserir = 0;
            }

            if (lido == 0)
            {
                printf("\nNao existem mais registros para serem listados!\n");
                system("PAUSE");
                return;
            }
        }
    } while (lido != 0);

    fclose(arquivo);
    system("PAUSE");
    return;
}

void removeRegistroPeloIndex()
{
    FILE *arquivo;
    arquivo = fopen("carros.dat", "r+b");

    if (!arquivo)
    {
        printf("O arquivo nao pode ser aberto!\n\n");
        system("PAUSE");
        return;
    }

    bloco blocoAux;
    int lido = 0;
    int posicaoreg = 0;
    int qntdblocos = 0;
    int encontrado = 0;
    char modelobuscado[16];
    char buffer;
    char confirma;

    printf("Digite o modelo do carro a ser removido: ");
    for (int i = 0; i < 15; i++)
    {

        buffer = getc(stdin); // recebe entrada do teclado e joga caractere por caractere para o buffer
        if (buffer == '\n')
        { // caso o usuario aperte ENTER, encerramos a string com o caractere NUL(\0)
            modelobuscado[i] = '\0';
            break; //saimos do laco de repeticao "for"
        }
        if (i == 14)
        { // ignoramos todo o conteudo do buffer que esteja fora do tamanho do campo no registro
            while (getc(stdin) != '\n')
            {
                continue;
            }
        }
        modelobuscado[i] = buffer; //pega o caractere apontado pelo ponteiro de leitura e atribui para modelo[i]
    }
    modelobuscado[14] = '\0';

    if (!arquivo)
    {
        printf("Arquivo nao pode ser aberto!\n\n");
        system("PAUSE");
        fclose(arquivo);
        return;
    }

    Index idxReader = buscarNoIndexFile(modelobuscado);
    if (idxReader.bloco != -1)
    {
        fseek(arquivo, sizeof(bloco) * idxReader.bloco, SEEK_SET); //posicionando ponteiro de leitura no comeco do bloco a ser lido
        lido = fread(&blocoAux, sizeof(registro), 8, arquivo);     //lendo 8 registros(1 bloco) do arquivo;

        if (lido == 0)
        {
            printf("\n\nNao existe registros para ser removido!\n\n");
            system("PAUSE");
            fclose(arquivo);
            return;
        }

        printf("\n Registro Encontrado: \n");
        printf("\n\tModelo: %s\n\t", blocoAux.reg[idxReader.pos].modelo);
        printf("\n\tMarca: %s\n\t", blocoAux.reg[idxReader.pos].marca);
        printf("\n\tPais de Fabricacao: %s\n\t", blocoAux.reg[idxReader.pos].pais);
        printf("\n\tAno: %s\n\t", blocoAux.reg[idxReader.pos].ano);
        printf("\n\tPotencia: %scv\n\n", blocoAux.reg[idxReader.pos].pot);

        printf("\nConfirme a remocao do registro acima(S/N): ");
        scanf("%c", &confirma);
        switch (confirma)
        {
        case 'S':
            blocoAux.reg[idxReader.pos].modelo[0] = '#';
            fseek(arquivo, sizeof(bloco) * (idxReader.bloco - 1), SEEK_SET);
            fwrite(&blocoAux, sizeof(registro), 8, arquivo);
            treatAfterRemove(idxReader);
            break;
        case 'N':
            printf("\n\nRegistro NAO removido!\n\n");
            system("PAUSE");
            break;
        default:
            break;
        }
    }
    

    fclose(arquivo);
    return;
}
bool treatAfterRemove(Index idx)
{

    FILE *arquivo;
    arquivo = fopen("index.dat", "a+b"); // cria arquivo para leitura e escrita em modo binario

    fseek(arquivo, 0, SEEK_END); //posicionando ponteiro de leitura no fim do arquivo
    int idxLength = (ftell(arquivo) / sizeof(Index));
    fseek(arquivo, 0, SEEK_SET); //reposicionando ponteiro de leitura no comeco do arquivo

    if (!arquivo)
    { // verifica se houve erro ao abrir o arquivo
        printf("Arquivo de indices nao pode ser aberto!\n\n");
        system("PAUSE");
    }

    Index *idxReader = (Index *)malloc(idxLength * sizeof(Index));
    int lido = 0;
    lido = fread(idxReader, sizeof(Index), idxLength, arquivo);
    fclose(arquivo);
    if (lido == 0)// caso arquivo vazio, não inserimos nada
    {
        arquivo = fopen("index.dat", "wb");
        free(idxReader);
        int close = fclose(arquivo);
        if(close == 0){
            printf("Registro e index removidos com sucesso \n");
            system("PAUSE");
            free(idxReader);
            return true;
        }
        else{
            printf("Erro no fechamento do arquivo de índices (que não deveria conter mais nenhum dado)\n");
            system("PAUSE");
            free(idxReader);
            return false;
        }
    }
    int inserir;
    bool achou = false;

    for (int i = 0; i < idxLength; i++)
    {
        if (strcmp(idx.key, idxReader[i].key) == 0)
        {
            inserir = i;
            achou = true;
        }
    }
    if (achou == false)
    {
        printf("Erro na remoção do indice\n");
        system("PAUSE");
        free(idxReader);
        return false;
    }

    Index idxWriter[idxLength - 1];
    int flag = 0;
    for (int i = 0; i < idxLength - 1; i++)
    {
        if (flag)
        {
            strcpy(idxWriter[i].key, idxReader[i + 1].key);
            idxWriter[i].pos = idxReader[i + 1].pos;
            idxWriter[i].bloco = idxWriter[i + 1].bloco;
        }
        else
        {
            strcpy(idxWriter[i].key, idxReader[i].key);
            idxWriter[i].pos = idxReader[i].pos;
            idxWriter[i].bloco = idxWriter[i].bloco;
        }
        if (i == inserir)
        {
            flag = 1;
        }
    }
    arquivo = fopen("index.dat", "wb");
    int inseriu = fwrite(idxWriter, sizeof(Index), idxLength - 1, arquivo);
    if (inseriu > 0)
    {
        printf("Registro e index removido com sucesso \n");
        system("PAUSE");
        free(idxReader);
        fclose(arquivo);
        return true;
    }
    printf("Index não removido!! \n");
    system("PAUSE");
    free(idxReader);
    fclose(arquivo);
    return false;
}
void insereBlocoAux(registro reg)
{
    FILE *arquivo;
    arquivo = fopen("carros.dat", "r+b"); // cria arquivo para leitura e escrita em modo binario

    bloco blocoAux;
    int inserir = 0;
    int posicaoinsere = -1;
    int qntdblocos = 0;

    char buffer;

    if (!arquivo)
    { // verifica se houve erro ao abrir o arquivo
        printf("Arquivo nao pode ser aberto!\n\n");
        system("PAUSE");
        return;
    }

    while (inserir != 1)
    { // estamos buscando uma posiçao que possa ser inserido o novo registro
        int fim = fread(&blocoAux, sizeof(registro), 8, arquivo);
        fseek(arquivo, sizeof(blocoAux) * (qntdblocos++), SEEK_SET); // andando de bloco em bloco com o ponteiro de leitura
        if (fim == 0)
        {                                                //caso nada tenha sido retornado na leitura (EOF ou arquivo vazio) entao inserimos um novo bloco
            insereBloco(arquivo);                        // insere novo bloco vazio no final do arquivo pois nao existem opcoes livres para insercao
            fseek(arquivo, -sizeof(blocoAux), SEEK_END); // retorna o ponteiro para a posicao inicial do bloco inserido
        }
        fread(&blocoAux, sizeof(registro), 8, arquivo); //recupera o bloco todo (em 8 registros) para a variavel blocoAux
        for (int i = 0; i < 8; i++)
        {
            if (blocoAux.reg[i].modelo[0] == '#')
            {
                posicaoinsere = i; // posicao do registro a ser preenchido dentro do bloco
                inserir = 1;
                break;
            }
        }
    }

    //Coletando os campos de cada registro que vai ser inserido
    strcpy(blocoAux.reg[posicaoinsere].modelo, reg.modelo);
    strcpy(blocoAux.reg[posicaoinsere].marca, reg.marca);
    strcpy(blocoAux.reg[posicaoinsere].pais, reg.pais);
    strcpy(blocoAux.reg[posicaoinsere].ano, reg.ano);
    strcpy(blocoAux.reg[posicaoinsere].pot, reg.pot);

    //posiciona o ponteiro do arquivo na posicao onde o bloco vai ser inserido
    fseek(arquivo, sizeof(bloco) * (qntdblocos - 1), SEEK_SET);

    //escreve todo o bloco no arquivo
    fwrite(&blocoAux, sizeof(registro), 8, arquivo);

    //fecha o arquivo
    fclose(arquivo);
}
void insereLote()
{
    int qntd;
    printf("\nDigite quantos registros voce quer inserir aleatoriamente: ");
    scanf("%d", &qntd);
    int contador = qntd;

    blocoAl blocoAleatorio1; // Bloco usado para armazenar registros e preencher arquivo aleatoriamente
    strcpy(blocoAleatorio1.reg[0].modelo, "Evoque");
    strcpy(blocoAleatorio1.reg[0].marca, "Land Rover");
    strcpy(blocoAleatorio1.reg[0].pais, "Alemanha");
    strcpy(blocoAleatorio1.reg[0].ano, "2016");
    strcpy(blocoAleatorio1.reg[0].pot, "1500");

    strcpy(blocoAleatorio1.reg[1].modelo, "HB20");
    strcpy(blocoAleatorio1.reg[1].marca, "Hyundai");
    strcpy(blocoAleatorio1.reg[1].pais, "Brasil");
    strcpy(blocoAleatorio1.reg[1].ano, "2014");
    strcpy(blocoAleatorio1.reg[1].pot, "1000");

    strcpy(blocoAleatorio1.reg[2].modelo, "i30");
    strcpy(blocoAleatorio1.reg[2].marca, "Hyundai");
    strcpy(blocoAleatorio1.reg[2].pais, "Brasil");
    strcpy(blocoAleatorio1.reg[2].ano, "2015");
    strcpy(blocoAleatorio1.reg[2].pot, "1000");

    strcpy(blocoAleatorio1.reg[3].modelo, "Fusca");
    strcpy(blocoAleatorio1.reg[3].marca, "Volkswagem");
    strcpy(blocoAleatorio1.reg[3].pais, "Alemanha");
    strcpy(blocoAleatorio1.reg[3].ano, "1960");
    strcpy(blocoAleatorio1.reg[3].pot, "200");

    strcpy(blocoAleatorio1.reg[4].modelo, "Uno Mile");
    strcpy(blocoAleatorio1.reg[4].marca, "Fiat");
    strcpy(blocoAleatorio1.reg[4].pais, "Italia");
    strcpy(blocoAleatorio1.reg[4].ano, "2005");
    strcpy(blocoAleatorio1.reg[4].pot, "500");

    strcpy(blocoAleatorio1.reg[5].modelo, "Focus");
    strcpy(blocoAleatorio1.reg[5].marca, "Ford");
    strcpy(blocoAleatorio1.reg[5].pais, "Estados Unidos");
    strcpy(blocoAleatorio1.reg[5].ano, "2009");
    strcpy(blocoAleatorio1.reg[5].pot, "650");

    strcpy(blocoAleatorio1.reg[6].modelo, "Camaro");
    strcpy(blocoAleatorio1.reg[6].marca, "Chevrolet");
    strcpy(blocoAleatorio1.reg[6].pais, "Mexico");
    strcpy(blocoAleatorio1.reg[6].ano, "2013");
    strcpy(blocoAleatorio1.reg[6].pot, "1050");

    strcpy(blocoAleatorio1.reg[7].modelo, "Celta");
    strcpy(blocoAleatorio1.reg[7].marca, "Chevrolet");
    strcpy(blocoAleatorio1.reg[7].pais, "Brasil");
    strcpy(blocoAleatorio1.reg[7].ano, "2013");
    strcpy(blocoAleatorio1.reg[7].pot, "400");

    strcpy(blocoAleatorio1.reg[8].modelo, "Monza");
    strcpy(blocoAleatorio1.reg[8].marca, "Chevrolet");
    strcpy(blocoAleatorio1.reg[8].pais, "Brasil");
    strcpy(blocoAleatorio1.reg[8].ano, "1990");
    strcpy(blocoAleatorio1.reg[8].pot, "1000");

    strcpy(blocoAleatorio1.reg[9].modelo, "Onix");
    strcpy(blocoAleatorio1.reg[9].marca, "Chevrolet");
    strcpy(blocoAleatorio1.reg[9].pais, "Brasil");
    strcpy(blocoAleatorio1.reg[9].ano, "2016");
    strcpy(blocoAleatorio1.reg[9].pot, "1200");

    strcpy(blocoAleatorio1.reg[10].modelo, "Del Rey");
    strcpy(blocoAleatorio1.reg[10].marca, "Ford");
    strcpy(blocoAleatorio1.reg[10].pais, "Brasil");
    strcpy(blocoAleatorio1.reg[10].ano, "1980");
    strcpy(blocoAleatorio1.reg[10].pot, "1000");

    strcpy(blocoAleatorio1.reg[11].modelo, "Gol");
    strcpy(blocoAleatorio1.reg[11].marca, "Volkswagem");
    strcpy(blocoAleatorio1.reg[11].pais, "Alemanha");
    strcpy(blocoAleatorio1.reg[11].ano, "2015");
    strcpy(blocoAleatorio1.reg[11].pot, "400");

    strcpy(blocoAleatorio1.reg[12].modelo, "March");
    strcpy(blocoAleatorio1.reg[12].marca, "Nissan");
    strcpy(blocoAleatorio1.reg[12].pais, "Japao");
    strcpy(blocoAleatorio1.reg[12].ano, "2015");
    strcpy(blocoAleatorio1.reg[12].pot, "400");

    strcpy(blocoAleatorio1.reg[13].modelo, "Frontier");
    strcpy(blocoAleatorio1.reg[13].marca, "Nissan");
    strcpy(blocoAleatorio1.reg[13].pais, "Japao");
    strcpy(blocoAleatorio1.reg[13].ano, "2017");
    strcpy(blocoAleatorio1.reg[13].pot, "1700");

    strcpy(blocoAleatorio1.reg[14].modelo, "Compass");
    strcpy(blocoAleatorio1.reg[14].marca, "Jeep");
    strcpy(blocoAleatorio1.reg[14].pais, "Estados Unidos");
    strcpy(blocoAleatorio1.reg[14].ano, "2017");
    strcpy(blocoAleatorio1.reg[14].pot, "1500");

    strcpy(blocoAleatorio1.reg[15].modelo, "Opala");
    strcpy(blocoAleatorio1.reg[15].marca, "Chevrolet");
    strcpy(blocoAleatorio1.reg[15].pais, "Brasil");
    strcpy(blocoAleatorio1.reg[15].ano, "1970");
    strcpy(blocoAleatorio1.reg[15].pot, "600");

    srand((unsigned)time(NULL));
    while (qntd >= 0)
    {
        int reg = rand() % 17;
        insereBlocoAux(blocoAleatorio1.reg[reg]);
        qntd--;
    }
    printf("\n%d registros inseridos com sucesso!\n\n", contador);
    system("PAUSE");
    return;
}

void menu()
{
    int opcao;

    while (1)
    {
        system("CLS");
        printf("------- MENU -------\n\n");
        printf("(1) Criar arquivo vazio\n");
        printf("(2) Inserir um novo registro\n");
        printf("(3) Buscar um registro\n");
        printf("(4) Listar todos os registro\n");
        printf("(5) Remover um registro\n");
        printf("(6) Inserir X registros aleatorios\n");
        printf("(0) Sair.\n");
        printf("\nDigite a opcao desejada: ");
        scanf("%d", &opcao);
        setbuf(stdin, NULL);
        system("CLS");

        switch (opcao)
        {
        case (0):
            return;
        case (1):
            criaArquivo();
            break; // sai do case e retorna para o menu apos o retorno de criaArquivo()
        case (2):
            insereRegistro();
            break; // sai do case e retorna para o menu apos o retorno de insereRegistro()
        case (3):
            buscaRegistroNoIndex();
            break; // sai do case e retorna para o menu apos o retorno de buscaRegistroNoIndex()
        case (4):
            listaRegistros();
            break; // sai do case e retorna para o menu apos o retorno de listaRegistro()
        case (5):
            removeRegistroPeloIndex();
            break; // sai do case e retorna para o menu apos o retorno de removeRegistroPeloIndex()
        case (6):
            insereLote();
            break; // sai do case e retorna para o menu apos o retorno de insereLote()
        default:
            continue;
        }
    }
}

int main()
{
    menu();
    return 0;
}
