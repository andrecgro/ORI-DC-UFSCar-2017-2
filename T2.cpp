//*********************************************************************************
// Trabalho 1 - Organizacao e Recuperacao de Arquivos - 2017/2
// Docente: Prof. Dr. Ricardo Rodrigues Ciferri
// Discentes: Andre Camargo Rocha RA:587052 e Sergio Hideki RA:551848
//*********************************************************************************

#include <iostream>
#include <fstream>
#include <string.h>
#include <time.h>
#include <cstdlib>
#include <stdio.h>

struct registro { // registro de tamanho fixo de 64 bytes
    char modelo[16]; // 16 bytes (15 + 1 para o '\0')
    char marca[19]; // 19 bytes (18 + 1 para o '\0')
    char pais[19]; // 19 bytes (18 + 1 para o '\0')
    char ano[5]; // 5 bytes (tipo 'int' estava vindo com 8 bytes por causa do compilador) (4 + 1 para o '\0')
    char pot[5]; // 5 bytes (4 + 1 para o '\0')
};

struct bloco{ // bloco de tamanho fixo de 512 bytes (8 registros de 64 bytes cada)
    registro reg[8];
};

struct blocoAl{ // bloco de tamanho fixo de 1024 bytes (16 registros de 64 bytes cada)
    registro reg[16];
};

typedef struct{
    int bloco;
    int pos;
    char key[16];
}Index;


//*********************************************************************************

void criaArquivo(); // Cria novo arquivo binario para trabalhar com os dados
void insereBloco(FILE* arquivo); // Insere um novo bloco de 512 bytes no final do arquivo
void insereRegistro(); // Insere um novo registro de 64 bytes no primeiro espaço vazio encontrado em um bloco
void buscaRegistro(); // Lista determinado registro dada uma chave (Modelo)
void listaRegistros(); // Lista todos os registros do arquivo de dados
void listaArquivo(); // Lista o arquivo
void removeRegistro(); // Remove determinado registro dada uma chave (Modelo)
void insereBlocoAux(registro reg); // Faz insercao de registro mas recebendo como parametro
void insereLote(); // Insere um lote de carros aleatoriamente
void criaArquivoIndex(); // Cria novo arquivo binario para trabalhar com os indices
Index* lerIndexFile();
void menu(); // Imprime o menu de opcoes na tela
void insereIndex(char modeloAux[16], int pox, int qntdblocos);
//*********************************************************************************

void criaArquivo(){//Cria arquivo e insere um bloco(8 registros) completamente vazio
    struct bloco blocoVazio;
    FILE* arquivo;
    int i;
    arquivo = fopen("carros.dat", "wb");

    if (!arquivo) { // verifica se nao ha erro na abertura do arquivo
        printf("Arquivo de dadosnao pode ser aberto para gravacao!\n\n");
    }
    else{
        printf("Arquivo de dados criado com sucesso!\n\n");
        criaArquivoIndex();
    }
    fclose(arquivo);
    system("PAUSE");
    return;
}

void criaArquivoIndex(){//Cria arquivo completamente vazio
    FILE* arquivo;
    int i;
    arquivo = fopen("index.dat", "wb");

    if (!arquivo) { // verifica se nao ha erro na abertura do arquivo
        printf("Arquivo de index nao pode ser aberto para gravacao!\n\n");
    }
    else{
        printf("Arquivo de index criado com sucesso!\n\n");
    }
    fclose(arquivo);
    return;
}

void insereBloco(FILE* arquivo){ // Abre o arquivo e insere um bloco vazio no final - usada caso todos os blocos estejam cheios
    struct bloco blocoVazio;
    int i;

    if (!arquivo) { // verifica se houve erro ao abrir o arquivo para escrita
        printf("Arquivo nao pode ser aberto para gravacao!\n\n");
        system("PAUSE");
        return;
    }
    for (i = 0; i<8; i++){
        strcpy(blocoVazio.reg[i].modelo, "#");// preenche cada registro com '#' para mostrar que está vazio
    }
    fwrite(&blocoVazio, sizeof(blocoVazio), 1, arquivo);

}

void insereRegistro(){
    FILE* arquivo;
    arquivo = fopen("carros.dat", "r+b");// cria arquivo para leitura e escrita em modo binario

    char modeloAux[16];

    bloco blocoAux;
    int inserir = 0;
    int posicaoinsere = -1;
    int qntdblocos = 0;

    char buffer;

    if (!arquivo) { // verifica se houve erro ao abrir o arquivo
        printf("Arquivo nao pode ser aberto!\n\n");
        system("PAUSE");
        return;
    }

    while(inserir != 1){// estamos buscando uma posiçao que possa ser inserido o novo registro
        int fim = fread(&blocoAux, sizeof(registro), 8, arquivo);
        fseek(arquivo,sizeof(blocoAux)*(qntdblocos++), SEEK_SET);// andando de bloco em bloco com o ponteiro de leitura
        if(fim == 0){//caso nada tenha sido retornado na leitura (EOF ou arquivo vazio) entao inserimos um novo bloco
            insereBloco(arquivo); // insere novo bloco vazio no final do arquivo pois nao existem opcoes livres para insercao
            fseek(arquivo,-sizeof(blocoAux), SEEK_END);// retorna o ponteiro para a posicao inicial do bloco inserido
        }
        fread(&blocoAux, sizeof(registro), 8, arquivo); //recupera o bloco todo (em 8 registros) para a variavel blocoAux
        for(int i=0; i<8; i++){
            if(blocoAux.reg[i].modelo[0] == '#'){
                posicaoinsere = i; // posicao do registro a ser preenchido dentro do bloco
                inserir = 1;
                break;
            }
        }
    }

    //Coletando os campos de cada registro que vai ser inserido
	printf("Digite o modelo do carro: ");
	for(int i = 0; i < 15; i++){

        buffer = getc(stdin); // recebe entrada do teclado e joga caractere por caractere para o buffer
        if (buffer == '\n'){// caso o usuario aperte ENTER, encerramos a string com o caractere NUL(\0)
            blocoAux.reg[posicaoinsere].modelo[i] = '\0';
            break;//saimos do laco de repeticao "for"
        }
        if (i == 14){// ignoramos todo o conteudo do buffer que esteja fora do tamanho do campo no registro
            while (getc(stdin) != '\n'){
                continue;
            }
        }
        blocoAux.reg[posicaoinsere].modelo[i] = buffer;//pega o caractere apontado pelo ponteiro de leitura e atribui para modelo[i]
        //modeloAux[i] = buffer;
	}
    blocoAux.reg[posicaoinsere].modelo[14] = '\0';
    strcpy(modeloAux,blocoAux.reg[posicaoinsere].modelo);

	printf("Digite a marca do carro: ");
	for(int i = 0; i < 18; i++){

        buffer = getc(stdin); // recebe entrada do teclado e joga caractere por caractere para o buffer
        if (buffer == '\n'){// caso o usuario aperte ENTER, encerramos a string com o caractere NUL(\0)
            blocoAux.reg[posicaoinsere].marca[i] = '\0';
            break;//saimos do laco de repeticao "for"
        }
        if (i == 17){// ignoramos todo o conteudo do buffer que esteja fora do tamanho do campo no registro
            while (getc(stdin) != '\n'){
                continue;
            }
        }
        blocoAux.reg[posicaoinsere].marca[i] = buffer;//pega o caractere apontado pelo ponteiro de leitura e atribui para modelo[i]
	}
    blocoAux.reg[posicaoinsere].marca[17] = '\0';

	printf("Digite o pais de fabricacao do carro: ");
	for(int i = 0; i < 18; i++){

        buffer = getc(stdin); // recebe entrada do teclado e joga caractere por caractere para o buffer
        if (buffer == '\n'){// caso o usuario aperte ENTER, encerramos a string com o caractere NUL(\0)
            blocoAux.reg[posicaoinsere].pais[i] = '\0';
            break;//saimos do laco de repeticao "for"
        }
        if (i == 17){// ignoramos todo o conteudo do buffer que esteja fora do tamanho do campo no registro
            while (getc(stdin) != '\n'){
                continue;
            }
        }
        blocoAux.reg[posicaoinsere].pais[i] = buffer;//pega o caractere apontado pelo ponteiro de leitura e atribui para modelo[i]
	}
    blocoAux.reg[posicaoinsere].pais[17] = '\0';

	printf("Digite o ano de fabricacao do carro: ");
	for(int i = 0; i < 5; i++){

        buffer = getc(stdin); // recebe entrada do teclado e joga caractere por caractere para o buffer
        if (buffer == '\n'){// caso o usuario aperte ENTER, encerramos a string com o caractere NUL(\0)
            blocoAux.reg[posicaoinsere].ano[i] = '\0';
            break;//saimos do laco de repeticao "for"
        }
        if (i == 4){// ignoramos todo o conteudo do buffer que esteja fora do tamanho do campo no registro
            while (getc(stdin) != '\n'){
                continue;
            }
        }
        blocoAux.reg[posicaoinsere].ano[i] = buffer;//pega o caractere apontado pelo ponteiro de leitura e atribui para modelo[i]
	}
    blocoAux.reg[posicaoinsere].ano[4] = '\0';

	printf("Digite a potencia do carro: ");
	for(int i = 0; i < 5; i++){

        buffer = getc(stdin);// recebe entrada do teclado e joga caractere por caractere para o buffer
        if (buffer == '\n'){// caso o usuario aperte ENTER, encerramos a string com o caractere NUL(\0)
            blocoAux.reg[posicaoinsere].pot[i] = '\0';
            break;//saimos do laco de repeticao "for"
        }
        if (i == 4){// ignoramos todo o conteudo do buffer que esteja fora do tamanho do campo no registro
            while (getc(stdin) != '\n'){
                continue;
            }
        }
        blocoAux.reg[posicaoinsere].pot[i] = buffer;//pega o caractere apontado pelo ponteiro de leitura e atribui para modelo[i]
	}
    blocoAux.reg[posicaoinsere].pot[4] = '\0';

    //posiciona o ponteiro do arquivo na posicao onde o bloco vai ser inserido
	fseek(arquivo, sizeof(bloco)*(qntdblocos-1), SEEK_SET);

	//escreve todo o bloco no arquivo
	fwrite(&blocoAux, sizeof(registro), 8, arquivo);
    insereIndex(modeloAux,posicaoinsere,qntdblocos-1);//Insere informacoes do registro no arquivo de indices

	//fecha o arquivo
	fclose(arquivo);

	printf("\n\nRegistro inserido com sucesso!\n\n");
	system("PAUSE");
    return;
}


void insereIndex(char modeloAux[16], int pos, int qntdblocos ){
    FILE* arquivo;
    arquivo = fopen("index.dat", "a+b");// cria arquivo para leitura e escrita em modo binario

    fseek(arquivo,0, SEEK_END);//posicionando ponteiro de leitura no fim do arquivo
    int idxLength = (ftell(arquivo)/sizeof(Index));
    fseek(arquivo,0, SEEK_SET);//reposicionando ponteiro de leitura no comeco do arquivo

    Index* fileContent = lerIndexFile();

    for(int i=0; i<idxLength;i++){
        if(fileContent[i].key > modeloAux){
            
        }
        else{
            continue;
        }
    }

    if (!arquivo) { // verifica se houve erro ao abrir o arquivo
        printf("Arquivo de indices nao pode ser aberto!\n\n");
        system("PAUSE");
    }

    Index idx;
    idx.bloco=qntdblocos;
    idx.pos=pos;
    strcpy(idx.key, modeloAux);

    fwrite(&idx, sizeof(idx), 1, arquivo);

    fclose(arquivo);
}

Index* lerIndexFile(){
    FILE* index;
    index = fopen("index.dat", "r+b");
    long fileSize = ftell(index);
    int lido = 0;
    int encontrado = 0;


    Index *idxAux;
    fseek(index,0, SEEK_END);//posicionando ponteiro de leitura no fim do arquivo
    int idxLength = (ftell(index)/sizeof(Index));
    idxAux = (Index*) malloc( idxLength * sizeof(Index) );

    if (!index) { // verifica se houve erro ao abrir o arquivo
        printf("Arquivo de indices nao pode ser aberto!\n\n");
        system("PAUSE");
    }

    fseek(index,0, SEEK_SET);//posicionando ponteiro de leitura no comeco do arquivo

    lido = fread(idxAux, sizeof(Index), idxLength, index); //lendo 8 registros(1 bloco) do arquivo;
    if(lido == 0){
        printf("\n\nNao existem indices para serem recuperados!\n\n");
        system("PAUSE");
        return NULL;
    }

    for(int i=0; i < idxLength; i++){
        if (idxAux[i].key[0] != '#')
        {
            printf("\n\tModelo: %s\n\t", idxAux[i].key);
            printf("\n\tBloco: %d\n\t", idxAux[i].bloco);
            printf("\n\tPosicao: %d\n\t", idxAux[i].pos);
        }
    }
    fclose(index);
    system("PAUSE");
    return idxAux;
}



void removeIndex(char modeloAux[16]){
    FILE* index;
    FILE* dados;
    index = fopen("index.dat", "r+b");
    long fileSize = ftell(index);
    int bloco=0;
    int pos=0;

    Index *idxAux;
    fseek(index, 0, SEEK_END);//posicionando ponteiro de leitura no comeco do arquivo
    int idxLength = (ftell(index)/sizeof(Index));
    idxAux = (Index*) malloc( idxLength * sizeof(Index) );
    
    // Binary search in indexe's array
    Index pivot;

    // Properly removing the index found
    
    /*for(int i=0; i<idxLength; i++){
        if(strcmp(modeloAux.key,idxAux[i].key)){
            bloco = idxAux[i].bloco;
            pos = idxAux[i].pos;
            strcpy(idxAux[i].key[0], "#");
        }
    } */
    int flag = 0;
    // Rearranging indexes on index file
    Index idxToWrite[idxLength-1];
    for(int i=0; i<idxLength-1; i++){
        if (idxAux[i].key[0] == '#')
        {
            flag=1;
        }
        if(flag){
            idxToWrite[i]=idxAux[i+1];
        }
        else{
             idxToWrite[i]=idxAux[i];
        }
    }
    free(idxAux);
    // Closing file to open using fopen but in another mode
    fclose(index);
    index = fopen("index.dat", "w+b");
    fwrite(&idxToWrite, sizeof(Index), idxLength-1, index);
    fclose(index);
}
/* 
void removeIndex(char modeloAux[16], int pos, int qntdblocos){
    FILE* index;
    index = fopen("index.dat", "r+b");
    long fileSize = ftell(index);
    int lido = 0;
    int encontrado = 0;
    int removidos = 0;
    Index idxReturn;


    Index *idxAux;
    fseek(index,0, SEEK_END);//posicionando ponteiro de leitura no comeco do arquivo
    int idxLength = (ftell(index)/sizeof(Index));
    idxAux = (Index*) malloc( idxLength * sizeof(Index) );

    index *idxAux = lerIndexFile();

    for(int i=0;i<idxLength;i++){
        if(strcmp(idxAux[i].key, modeloAux) && idxAux.bloco == qntdblocos && idxAux.pos == pos){
            strcpy(idxAux[i].key[0], "#");
            removidos++;
            //funcao para fazer shift
            break;
        }
    }
    strcpy(idxAux[idxLength-1].key,"#");

    int j = 0;
    for(int i=0; i<idxLength-removidos;i++){
        if(strcmp(idxAux.key[i]), "#"){
            i++;
        }
        else{
            idxToWrite[j] = idxAux[i];
            j++;
        }
    }
    
    fclose(index);
    
    free(idxAux);
    newIndex = fopen("index.dat", "w+b");
    fwrite(&idxToWrite, sizeof(Index), idxLength-removidos, newIndex);
    fclose(newIndex);
    return 
    
} */

void buscaRegistro(){
    FILE* arquivo;
    arquivo = fopen("carros.dat", "rb");

    bloco blocoAux;
    int lido=0;
    int posicaoreg=0;
    int qntdblocos=0;
    int encontrado = 0;
    char modelobuscado[16];
    char buffer;

    printf("Digite o modelo do carro buscado: ");
	for(int i = 0; i < 15; i++){

        buffer = getc(stdin); // recebe entrada do teclado e joga caractere por caractere para o buffer
        if (buffer == '\n'){// caso o usuario aperte ENTER, encerramos a string com o caractere NUL(\0)
            modelobuscado[i] = '\0';
            break;//saimos do laco de repeticao "for"
        }
        if (i == 14){// ignoramos todo o conteudo do buffer que esteja fora do tamanho do campo no registro
            while (getc(stdin) != '\n'){
                continue;
            }
        }
        modelobuscado[i] = buffer;//pega o caractere apontado pelo ponteiro de leitura e atribui para modelo[i]
	}
    modelobuscado[14] = '\0';

    if(!arquivo){
        printf("Arquivo nao pode ser aberto!\n\n");
        system("PAUSE");
        return;
    }

    while(encontrado!=1){
        fseek(arquivo, sizeof(bloco)*(qntdblocos++), SEEK_SET);//posicionando ponteiro de leitura no comeco do bloco a ser lido
        lido = fread(&blocoAux, sizeof(registro), 8, arquivo); //lendo 8 registros(1 bloco) do arquivo;

        if(lido == 0){
            printf("\n\nNao existem registros para serem listados!\n\n");
            system("PAUSE");
            return;
        }

        for(int i=0; i<8;i++){
            if(strcmp(blocoAux.reg[i].modelo, modelobuscado) == 0){
                printf("\n\tModelo: %s\n\t", blocoAux.reg[i].modelo);
                printf("\n\tMarca: %s\n\t", blocoAux.reg[i].marca);
                printf("\n\tPais de Fabricacao: %s\n\t", blocoAux.reg[i].pais);
                printf("\n\tAno: %s\n\t", blocoAux.reg[i].ano);
                printf("\n\tPotencia: %scv\n\n", blocoAux.reg[i].pot);
                encontrado=1;
                break;
            }
        }
    }
    printf("\n\nRegistro encontrado com sucesso!\n\n");
    fclose(arquivo);
    system("PAUSE");
    return;
}



void listaRegistros(){
    FILE* arquivo;
    arquivo = fopen("carros.dat", "rb");

    bloco blocoAux;
    int posicaoreg=0;
    int qntdblocos=0;
    int lido=1;

    if(!arquivo){
        printf("Arquivo nao pode ser aberto!\n\n");
        system("PAUSE");
        return;
    }


    do{
        fseek(arquivo, sizeof(bloco)*(qntdblocos++), SEEK_SET);//posicionando ponteiro de leitura no comeco do bloco a ser lido
        lido = fread(&blocoAux, sizeof(registro), 8, arquivo); //lendo 8 registros(1 bloco) do arquivo;

        if(lido == 0){
            printf("\n\nNao existem mais registros para serem listados!\n");
            system("PAUSE");
            return;
        }

        for(int i=0; i<8;i++){
            if(blocoAux.reg[i].modelo[0] != '#'){
                printf("\n----------------------------------------------------------\n");
                printf("\n\tModelo: %s\n\t", blocoAux.reg[i].modelo);
                printf("\n\tMarca: %s\n\t", blocoAux.reg[i].marca);
                printf("\n\tPais de Fabricacao: %s\n\t", blocoAux.reg[i].pais);
                printf("\n\tAno: %s\n\t", blocoAux.reg[i].ano);
                printf("\n\tPotencia: %scv\n\n", blocoAux.reg[i].pot);
            }
        }
    }while(lido!=0);

    fclose(arquivo);
    system("PAUSE");
    return;
}

void listaArquivo(){
    FILE* arquivo;
    arquivo = fopen("carros.dat", "rb");

    FILE* arquivo2;
    arquivo2 = fopen("carros2.dat", "rb");

    bloco blocoAux;
    bloco bloco2;
    bloco blocoNovo;
    int posicaoreg=0;
    int qntdblocos=0;
    int lido=1;
    int c = 0;
    int inserir = 0;
    int posicaoinsere = -1;

    if(!arquivo){
        printf("Arquivo nao pode ser aberto!\n\n");
        system("PAUSE");
        return;
    }

    if(!arquivo2){
        printf("Arquivo nao pode ser aberto!\n\n");
        system("PAUSE");
        return;
    }

    do{
        fseek(arquivo, sizeof(bloco)*(qntdblocos++), SEEK_SET);//posicionando ponteiro de leitura no comeco do bloco a ser lido
        lido = fread(&blocoAux, sizeof(registro), 8, arquivo); //lendo 8 registros(1 bloco) do arquivo;

        for(int i=0; i<8;i++){
            if(blocoAux.reg[i].modelo[0] != '#'){
                strcpy(bloco2.reg[c].modelo, blocoAux.reg[i].modelo);
                strcpy(bloco2.reg[c].marca, blocoAux.reg[i].marca);
                strcpy(bloco2.reg[c].pais, blocoAux.reg[i].pais);
                strcpy(bloco2.reg[c].ano, blocoAux.reg[i].ano);
                strcpy(bloco2.reg[c].pot, blocoAux.reg[i].pot);
                c++;
                if(c = 8){
                        while(inserir != 1){// estamos buscando uma posiçao que possa ser inserido o novo registro
                            int fim = fread(&blocoNovo, sizeof(registro), 8, arquivo2);
                            fseek(arquivo2,sizeof(blocoNovo)*(qntdblocos++), SEEK_SET);// andando de bloco em bloco com o ponteiro de leitura
                            if(fim == 0){//caso nada tenha sido retornado na leitura (EOF ou arquivo vazio) entao inserimos um novo bloco
                                insereBloco(arquivo2); // insere novo bloco vazio no final do arquivo pois nao existem opcoes livres para insercao
                                fseek(arquivo2,-sizeof(blocoNovo), SEEK_END);// retorna o ponteiro para a posicao inicial do bloco inserido
                            }
                            fread(&blocoNovo, sizeof(registro), 8, arquivo2); //recupera o bloco todo (em 8 registros) para a variavel blocoAux
                            for(int i=0; i<8; i++){
                                if(blocoNovo.reg[i].modelo[0] == '#'){
                                    posicaoinsere = i; // posicao do registro a ser preenchido dentro do bloco
                                    inserir = 1;
                                    break;
                                }
                            }
                        }
                        //começa a preencher o bloco do arquivo novo
                        for(int cont=0; c<8; c++){
                        strcpy(blocoNovo.reg[cont].modelo, bloco2.reg[cont].modelo);
                        strcpy(blocoNovo.reg[cont].marca, bloco2.reg[cont].marca);
                        strcpy(blocoNovo.reg[cont].pais, bloco2.reg[cont].pais);
                        strcpy(blocoNovo.reg[cont].ano, bloco2.reg[cont].ano);
                        strcpy(blocoNovo.reg[cont].pot, bloco2.reg[cont].pot);
                        }

                        //posiciona o ponteiro do arquivo na posicao onde o bloco vai ser inserido
                        fseek(arquivo2, sizeof(blocoNovo)*(qntdblocos-1), SEEK_SET);

                        //escreve todo o bloco no arquivo
                        fwrite(&blocoNovo, sizeof(registro), 8, arquivo2);

                        //fecha o arquivo
                        fclose(arquivo2);

                        //volta pra 0, pra não cair no if(c = 8)
                        c = 0;
                        inserir = 0;
                }
                //no caso de ter acabado de ler o arquivo e não ter completado o bloco
                if (lido == 0){
                    while(inserir != 1){// estamos buscando uma posiçao que possa ser inserido o novo registro
                            int fim = fread(&blocoNovo, sizeof(registro), 8, arquivo2);
                            fseek(arquivo2,sizeof(blocoNovo)*(qntdblocos++), SEEK_SET);// andando de bloco em bloco com o ponteiro de leitura
                            if(fim == 0){//caso nada tenha sido retornado na leitura (EOF ou arquivo vazio) entao inserimos um novo bloco
                                insereBloco(arquivo2); // insere novo bloco vazio no final do arquivo pois nao existem opcoes livres para insercao
                                fseek(arquivo2,-sizeof(blocoNovo), SEEK_END);// retorna o ponteiro para a posicao inicial do bloco inserido
                            }
                            fread(&blocoNovo, sizeof(registro), 8, arquivo2); //recupera o bloco todo (em 8 registros) para a variavel blocoAux
                            for(int i=0; i<c; i++){
                                if(blocoNovo.reg[i].modelo[0] == '#'){
                                    posicaoinsere = i; // posicao do registro a ser preenchido dentro do bloco
                                    inserir = 1;
                                    break;
                                }
                            }
                        }
                        //começa a preencher o bloco do arquivo novo
                        for(int cont=0; cont<8; c++){
                        strcpy(blocoNovo.reg[cont].modelo, bloco2.reg[cont].modelo);
                        strcpy(blocoNovo.reg[cont].marca, bloco2.reg[cont].marca);
                        strcpy(blocoNovo.reg[cont].pais, bloco2.reg[cont].pais);
                        strcpy(blocoNovo.reg[cont].ano, bloco2.reg[cont].ano);
                        strcpy(blocoNovo.reg[cont].pot, bloco2.reg[cont].pot);
                        }

                        //posiciona o ponteiro do arquivo na posicao onde o bloco vai ser inserido
                        fseek(arquivo2, sizeof(blocoNovo)*(qntdblocos-1), SEEK_SET);

                        //escreve todo o bloco no arquivo
                        fwrite(&blocoNovo, sizeof(registro), 8, arquivo2);

                        //fecha o arquivo
                        fclose(arquivo2);

                        //volta pra 0, pra não cair no if(c = 8)
                        c = 0;
                }
                inserir = 0;
        }

        if(lido == 0){
            printf("\nNao existem mais registros para serem listados!\n");
            system("PAUSE");
            return;
            }
        }
    }while(lido!=0);

    fclose(arquivo);
    system("PAUSE");
    return;
}

void removeRegistro(){
    FILE* arquivo;
    arquivo = fopen("carros.dat", "r+b");

    if(!arquivo){
        printf("O arquivo nao pode ser aberto!\n\n");
        system("PAUSE");
        return;
    }

    bloco blocoAux;
    int lido=0;
    int qntdblocos=0;
    int encontrado = 0;
    char modelobuscado[16];
    char buffer;
    char confirma;

    printf("Digite o modelo do carro que deseja remover: ");
	for(int i = 0; i < 15; i++){

        buffer = getc(stdin); // recebe entrada do teclado e joga caractere por caractere para o buffer
        if (buffer == '\n'){// caso o usuario aperte ENTER, encerramos a string com o caractere NUL(\0)
            modelobuscado[i] = '\0';
            break;//saimos do laco de repeticao "for"
        }
        if (i == 14){// ignoramos todo o conteudo do buffer que esteja fora do tamanho do campo no registro
            while (getc(stdin) != '\n'){
                continue;
            }
        }
        modelobuscado[i] = buffer;//pega o caractere apontado pelo ponteiro de leitura e atribui para modelo[i]
	}
    modelobuscado[14] = '\0';

    while(encontrado!=1){
        fseek(arquivo, sizeof(bloco)*(qntdblocos++), SEEK_SET);//posicionando ponteiro de leitura no comeco do bloco a ser lido
        lido = fread(&blocoAux, sizeof(registro), 8, arquivo); //lendo 8 registros(1 bloco) do arquivo;

        if(lido == 0){
            printf("\n\nNao existe registros para ser removido!\n\n");
            system("PAUSE");
            return;
        }

        for(int i=0; i<8;i++){
            if(strcmp(blocoAux.reg[i].modelo, modelobuscado) == 0){
                printf("\n Registro Encontrado: \n");
                printf("\n\tModelo: %s\n\t", blocoAux.reg[i].modelo);
                printf("\n\tMarca: %s\n\t", blocoAux.reg[i].marca);
                printf("\n\tPais de Fabricacao: %s\n\t", blocoAux.reg[i].pais);
                printf("\n\tAno: %s\n\t", blocoAux.reg[i].ano);
                printf("\n\tPotencia: %scv\n\n", blocoAux.reg[i].pot);
                encontrado=1;
                printf("\nConfirme a remocao do registro acima(S/N): ");
                scanf("%c",&confirma);
                switch(confirma){
                    case 'S':
                        blocoAux.reg[i].modelo[0] = '#';
                        fseek(arquivo, sizeof(bloco)*(qntdblocos-1), SEEK_SET);
                        fwrite(&blocoAux, sizeof(registro), 8, arquivo);
                        printf("\n\nRegistro removido com sucesso!\n\n");
                        break;
                    case 'N':
                        printf("\n\nRegistro NAO removido!\n\n");
                        break;
                    default:
                        break;
                }
            }
        }
    }
    fclose(arquivo);
    system("PAUSE");
    return;
}
void insereBlocoAux(registro reg){
    FILE* arquivo;
    arquivo = fopen("carros.dat", "r+b");// cria arquivo para leitura e escrita em modo binario

    bloco blocoAux;
    int inserir = 0;
    int posicaoinsere = -1;
    int qntdblocos = 0;

    char buffer;

    if (!arquivo) { // verifica se houve erro ao abrir o arquivo
        printf("Arquivo nao pode ser aberto!\n\n");
        system("PAUSE");
        return;
    }

    while(inserir != 1){// estamos buscando uma posiçao que possa ser inserido o novo registro
        int fim = fread(&blocoAux, sizeof(registro), 8, arquivo);
        fseek(arquivo,sizeof(blocoAux)*(qntdblocos++), SEEK_SET);// andando de bloco em bloco com o ponteiro de leitura
        if(fim == 0){//caso nada tenha sido retornado na leitura (EOF ou arquivo vazio) entao inserimos um novo bloco
            insereBloco(arquivo); // insere novo bloco vazio no final do arquivo pois nao existem opcoes livres para insercao
            fseek(arquivo,-sizeof(blocoAux), SEEK_END);// retorna o ponteiro para a posicao inicial do bloco inserido
        }
        fread(&blocoAux, sizeof(registro), 8, arquivo); //recupera o bloco todo (em 8 registros) para a variavel blocoAux
        for(int i=0; i<8; i++){
            if(blocoAux.reg[i].modelo[0] == '#'){
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
	fseek(arquivo, sizeof(bloco)*(qntdblocos-1), SEEK_SET);

	//escreve todo o bloco no arquivo
	fwrite(&blocoAux, sizeof(registro), 8, arquivo);

	//fecha o arquivo
	fclose(arquivo);
}
void insereLote(){
    int qntd;
    printf("\nDigite quantos registros voce quer inserir aleatoriamente: ");
    scanf("%d", &qntd);
    int contador = qntd;

    blocoAl blocoAleatorio1; // Bloco usado para armazenar registros e preencher arquivo aleatoriamente
    strcpy(blocoAleatorio1.reg[0].modelo, "Evoque");
    strcpy(blocoAleatorio1.reg[0].marca,"Land Rover");
    strcpy(blocoAleatorio1.reg[0].pais,"Alemanha");
    strcpy(blocoAleatorio1.reg[0].ano,"2016");
    strcpy(blocoAleatorio1.reg[0].pot,"1500");

    strcpy(blocoAleatorio1.reg[1].modelo,"HB20");
    strcpy(blocoAleatorio1.reg[1].marca,"Hyundai");
    strcpy(blocoAleatorio1.reg[1].pais,"Brasil");
    strcpy(blocoAleatorio1.reg[1].ano,"2014");
    strcpy(blocoAleatorio1.reg[1].pot,"1000");

    strcpy(blocoAleatorio1.reg[2].modelo,"i30");
    strcpy(blocoAleatorio1.reg[2].marca,"Hyundai");
    strcpy(blocoAleatorio1.reg[2].pais,"Brasil");
    strcpy(blocoAleatorio1.reg[2].ano,"2015");
    strcpy(blocoAleatorio1.reg[2].pot,"1000");

    strcpy(blocoAleatorio1.reg[3].modelo,"Fusca");
    strcpy(blocoAleatorio1.reg[3].marca,"Volkswagem");
    strcpy(blocoAleatorio1.reg[3].pais,"Alemanha");
    strcpy(blocoAleatorio1.reg[3].ano,"1960");
    strcpy(blocoAleatorio1.reg[3].pot,"200");

    strcpy(blocoAleatorio1.reg[4].modelo,"Uno Mile");
    strcpy(blocoAleatorio1.reg[4].marca,"Fiat");
    strcpy(blocoAleatorio1.reg[4].pais,"Italia");
    strcpy(blocoAleatorio1.reg[4].ano,"2005");
    strcpy(blocoAleatorio1.reg[4].pot,"500");

    strcpy(blocoAleatorio1.reg[5].modelo,"Focus");
    strcpy(blocoAleatorio1.reg[5].marca,"Ford");
    strcpy(blocoAleatorio1.reg[5].pais,"Estados Unidos");
    strcpy(blocoAleatorio1.reg[5].ano,"2009");
    strcpy(blocoAleatorio1.reg[5].pot,"650");

    strcpy(blocoAleatorio1.reg[6].modelo,"Camaro");
    strcpy(blocoAleatorio1.reg[6].marca,"Chevrolet");
    strcpy(blocoAleatorio1.reg[6].pais,"Mexico");
    strcpy(blocoAleatorio1.reg[6].ano,"2013");
    strcpy(blocoAleatorio1.reg[6].pot,"1050");

    strcpy(blocoAleatorio1.reg[7].modelo,"Celta");
    strcpy(blocoAleatorio1.reg[7].marca,"Chevrolet");
    strcpy(blocoAleatorio1.reg[7].pais,"Brasil");
    strcpy(blocoAleatorio1.reg[7].ano,"2013");
    strcpy(blocoAleatorio1.reg[7].pot,"400");

    strcpy(blocoAleatorio1.reg[8].modelo,"Monza");
    strcpy(blocoAleatorio1.reg[8].marca,"Chevrolet");
    strcpy(blocoAleatorio1.reg[8].pais,"Brasil");
    strcpy(blocoAleatorio1.reg[8].ano,"1990");
    strcpy(blocoAleatorio1.reg[8].pot,"1000");

    strcpy(blocoAleatorio1.reg[9].modelo,"Onix");
    strcpy(blocoAleatorio1.reg[9].marca,"Chevrolet");
    strcpy(blocoAleatorio1.reg[9].pais,"Brasil");
    strcpy(blocoAleatorio1.reg[9].ano,"2016");
    strcpy(blocoAleatorio1.reg[9].pot,"1200");

    strcpy(blocoAleatorio1.reg[10].modelo,"Del Rey");
    strcpy(blocoAleatorio1.reg[10].marca,"Ford");
    strcpy(blocoAleatorio1.reg[10].pais,"Brasil");
    strcpy(blocoAleatorio1.reg[10].ano,"1980");
    strcpy(blocoAleatorio1.reg[10].pot,"1000");

    strcpy(blocoAleatorio1.reg[11].modelo,"Gol");
    strcpy(blocoAleatorio1.reg[11].marca,"Volkswagem");
    strcpy(blocoAleatorio1.reg[11].pais,"Alemanha");
    strcpy(blocoAleatorio1.reg[11].ano,"2015");
    strcpy(blocoAleatorio1.reg[11].pot,"400");

    strcpy(blocoAleatorio1.reg[12].modelo,"March");
    strcpy(blocoAleatorio1.reg[12].marca,"Nissan");
    strcpy(blocoAleatorio1.reg[12].pais,"Japao");
    strcpy(blocoAleatorio1.reg[12].ano,"2015");
    strcpy(blocoAleatorio1.reg[12].pot,"400");

    strcpy(blocoAleatorio1.reg[13].modelo,"Frontier");
    strcpy(blocoAleatorio1.reg[13].marca,"Nissan");
    strcpy(blocoAleatorio1.reg[13].pais,"Japao");
    strcpy(blocoAleatorio1.reg[13].ano,"2017");
    strcpy(blocoAleatorio1.reg[13].pot,"1700");

    strcpy(blocoAleatorio1.reg[14].modelo,"Compass");
    strcpy(blocoAleatorio1.reg[14].marca,"Jeep");
    strcpy(blocoAleatorio1.reg[14].pais,"Estados Unidos");
    strcpy(blocoAleatorio1.reg[14].ano,"2017");
    strcpy(blocoAleatorio1.reg[14].pot,"1500");

    strcpy(blocoAleatorio1.reg[15].modelo,"Opala");
    strcpy(blocoAleatorio1.reg[15].marca,"Chevrolet");
    strcpy(blocoAleatorio1.reg[15].pais,"Brasil");
    strcpy(blocoAleatorio1.reg[15].ano,"1970");
    strcpy(blocoAleatorio1.reg[15].pot,"600");

    srand( (unsigned)time(NULL) );
    while(qntd >= 0){
        int reg = rand() % 17;
        insereBlocoAux(blocoAleatorio1.reg[reg]);
        qntd--;
    }
    printf("\n%d registros inseridos com sucesso!\n\n", contador);
    system("PAUSE");
    return;

}

void menu(){
int opcao;

	while (1){
		system("CLS");
		printf("------- MENU -------\n\n");
		printf("(1) Criar arquivo vazio\n");
		printf("(2) Inserir um novo registro\n");
		printf("(3) Buscar um registro\n");
		printf("(4) Listar todos os registro\n");
		printf("(5) Remover um registro\n");
		printf("(6) Inserir X registros aleatorios\n");
		printf("(7) Ler arquivo de indices\n");
		printf("(0) Sair.\n");
		printf("\nDigite a opcao desejada: ");
		scanf("%d", &opcao);
		setbuf(stdin, NULL);
		system("CLS");

		switch(opcao){
			case (0): return;
			case (1): criaArquivo();
				break;// sai do case e retorna para o menu apos o retorno de criaArquivo()
			case (2): insereRegistro();
				break;// sai do case e retorna para o menu apos o retorno de insereRegistro()
            case (3): buscaRegistro();
				break;// sai do case e retorna para o menu apos o retorno de buscaRegistro()
            case (4): listaRegistros();
				break;// sai do case e retorna para o menu apos o retorno de listaRegistro()
            case (5): removeRegistro();
				break;// sai do case e retorna para o menu apos o retorno de removeRegistro()
            case (6): insereLote();
				break;// sai do case e retorna para o menu apos o retorno de insereLote()
            case (7): lerIndexFile();
				break;
			default:
				continue;
		}
	}
}

int main(){
    menu();
    return 0;
}
