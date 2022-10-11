#include<stdio.h>
#include<stdlib.h>
#include<string.h>

//------------------------------------------------STRUCT------------------------------------------
typedef struct registro{
    char ISBN[14];
    char Titulo[50],autor[50];
    char ano[5];
}REGISTRO;

typedef struct vetor_indices_arq_primario{
    char isbn[14];
    int byteoffset;
} indice;

//------------------------------------------------FUNÇÔES------------------------------------------
void Inserir(FILE *insere, FILE *out, FILE *prim, indice* vet, int *tam_vet_carregar);
void BuscaPrimaria(FILE *Busca_P,FILE *out,FILE *prim,indice *vet,int tam_vetor);
void CarregarIndice(FILE *prim, indice *vet, int *tam_vet);
void RecriarIndice(FILE *prim, indice *vet, int tam_vet);
void BuscaSecundaria(FILE *Busca_S);
void CarregarArquivos(FILE *insere, FILE *Busca_P, FILE *Busca_S);

int main(){

    FILE *insere;		// ponteiro arq insere.bin
    FILE *out;			// ponteiro arq main.bin (guarda todos os dados)
    FILE *prim;			// ponteiro arq primario.bin (indice com ISBNs + byteoffset)
    FILE *Busca_P;
    FILE *Busca_S;

    int tamanho;
    int tam_vet_carregar;	// parametro funcao CarregarIndice
    REGISTRO REG;
    int opcao;

    if( (out = fopen("C:\\Users\\vmhug\\CLionProjects\\untitled\\main.bin","r+b")) == NULL){

        out = fopen("C:\\Users\\vmhug\\CLionProjects\\untitled\\main.bin","w+b");
    }

    if( (insere = fopen("C:\\Users\\vmhug\\CLionProjects\\untitled\\insere.bin","r+b")) == NULL){

        printf("ERRO NA ABERTURA DO ARQUIVO insere.bin");
    }

    if( (prim = fopen("C:\\Users\\vmhug\\CLionProjects\\untitled\\primario.bin","r+b")) == NULL){

        prim = fopen("C:\\Users\\vmhug\\CLionProjects\\untitled\\primario.bin","w+b");

        int num = 1;	// adiciona header inicializando com 1 (atualizado)
        fwrite(&num, sizeof(int), 1, prim);
    }

    if ((Busca_P = fopen("C:\\Users\\vmhug\\CLionProjects\\untitled\\busca_p.bin","r+b")) == NULL){
        printf("ERRO NA ABERTURA DO ARQUIVO busca_p.bin");
    }

    if ((Busca_S = fopen("C:\\Users\\vmhug\\CLionProjects\\untitled\\busca_s.bin","r+b")) == NULL){
        printf("ERRO NA ABERTURA DO ARQUIVO busca_s.bin");
    }


    //contador recupera a quant de registros no arq insere.bin
    int contador = 1;									//Inicializo contador=1 pra dar certo o num de posicoes
    while( fread(&REG, sizeof(REG), 1, insere) ){
        contador++;
    }

    // cria vetor com a quant de registros que podem ser inseridos a partir do arq insere.bin
    indice vet[contador];		// logica com vet estatico
    rewind(insere);

    char ISBN[14];
    int i = 0;
    vet[0].byteoffset = 0;		// salva primeiro byteoffset = 0

    // quando abre o programa, salva no vetor todos os registros ja inseridos ( a partir do main ), para continuar usando vetor atualizado na funcao insere
    while (fread(&tamanho, sizeof(int), 1, out)){
        if(i>0) {
            vet[i].byteoffset = ftell(out) - sizeof(int);
            //Salva em uma var auxiliar e depois copia para o vet[i].isbn para n bugar
            fread(&ISBN, sizeof(char), 13, out);
            strcpy(vet[i].isbn, ISBN);
            fseek(out, tamanho - sizeof(ISBN) + 1, SEEK_CUR);
            printf(" i>0 \n isbn: %s - offset: %d\n", vet[i].isbn, vet[i].byteoffset);
        }
        else{
            //Se for i = 0 ele salva so isso pq o byteoffset é 0 e foi setado la em ima
            fread(&ISBN, sizeof(char), 13, out);
            strcpy(vet[i].isbn,ISBN);
            fseek(out, tamanho - sizeof(ISBN) + 1, SEEK_CUR);
            printf(" else \n isbn: %s - offset: %d\n", vet[i].isbn, vet[i].byteoffset);
        }
        i++;
    }
    rewind(out);

// verificacao status do header --> atualizado = 1 ou desatualizado = 0 (quando fecha e abre de novo) no arq primario

    int num;
    rewind(prim);
    fread(&num, sizeof(int), 1, prim);		//Le header pra ver se ta atualizado ou nao
    printf("header (num) = %d e i (contador do vet) = %d\n", num, i);

// se header desatualizado --> recria arq primario.bin a partir do main
    if(num == 0){
        RecriarIndice(prim,vet, i);
    }

    do{
        printf("\n MENU\n 0-Sair\n 1-Inserir \n 2-BuscaPrimaria\n 3-BuscaSecundaria \n 4-CarregarArquivos\n Opcao: ");
        scanf("%d",&opcao);

        switch(opcao){
            case 0:{
                CarregarIndice(prim, vet, &tam_vet_carregar);
                break;
            }
            case 1:{
                Inserir(insere, out, prim, vet, &tam_vet_carregar);
                break;
            }
            case 2:{
                BuscaPrimaria(Busca_P, out, prim, vet, contador);
                break;
            }
            case 3:{
                BuscaSecundaria(Busca_S);
                break;
            }
            case 4:{
                CarregarArquivos(insere, Busca_P, Busca_S);
                break;
            }
            default:{
                printf("Opcao Invalida\n");
                break;
            }

        }
    }while(opcao!=0);

    fclose(out);
    fclose(insere);
    fclose(prim);
    fclose(Busca_P);
    fclose(Busca_S);

    return 0;
}

// funcao q compara dois valores do vetor dentro do qsort para ordenar vetor
int comparador(const void *a, const void *b){
    return ( *(int*)a - *(int*)b );
}

void Inserir(FILE *insere, FILE *out, FILE *prim, indice* vet, int *tam_vet_carregar){//

    int tam_registro=0, contador=0;
    char aux[14], registro[125];
    REGISTRO REG;

    int byteoffset;
    int i;

    rewind(insere);

    // quando for inserir um registro, muda header = 0 -> desatualizado
    rewind(prim);
    int num = 0;
    fwrite(&num, sizeof(int), 1, prim);
    printf("atualizou header = %d no primario (entrou inserir)\n", num);
    rewind(prim);

    // le no insere.bin registro a inserir e escreve no final do arq main string formatada com todos os dados
    while( fread(&REG, sizeof(REG), 1, insere) ){
        sprintf(aux, "%s", REG.ISBN);

        if(aux[0]!='/'){

            // formata string, calcula tam total e escreve no arq main
            sprintf(registro,"%s#%s#%s#%s", REG.ISBN, REG.Titulo, REG.autor, REG.ano);
            tam_registro = strlen(registro) + 1;

            // posiciona no arq para escrever e recupera byteoffset da onde o registro vai ser inserido
            fseek(out, 0, SEEK_END);
            byteoffset = ftell(out);
            fwrite(&tam_registro, sizeof(int), 1, out);
            fwrite(registro, sizeof(char), tam_registro, out);

            // fecha para conseguir salvar o que escreveu no arq (fica no buffer e só salva realmente quando fecha) e já abre de novo para continuar fluxo
            fclose(out);
            if( (out = fopen("C:\\Users\\vmhug\\CLionProjects\\untitled\\main.bin","r+b")) == NULL){
                printf("erro abertura arq main.bin dentro funcao insere");
            }

            // apos insercao no main, insere ISBN + byteoffset no vetor (indice)
            sprintf(vet[contador].isbn, "%s", aux);
            strcat(vet[contador].isbn,"\0");
            vet[contador].byteoffset = byteoffset;

            // ordena vetor
            qsort(vet, contador+1, sizeof(vet[0]), comparador);
            *tam_vet_carregar = contador+1;			// para passar tam do vetor hora que for descarregar no arq primario (funcao CarregarIndice)

            //marca o registro como lido no insere.bin
            aux[0] = '/';
            fseek(insere, (contador)*sizeof(REG), SEEK_SET);
            fwrite(&aux, sizeof(REG.ISBN), 1, insere);
            // fecha para conseguir salvar o que escreveu no arq (fica no buffer e só salva realmente quando fecha) e já abre de novo para continuar fluxo
            fclose(insere);
            if( (insere = fopen("C:\\Users\\vmhug\\CLionProjects\\untitled\\insere.bin","r+b")) == NULL){
                printf("erro abertura arq insere.bin dentro funcao insere");
            }

            fclose(prim);
            if( (prim = fopen("C:\\Users\\vmhug\\CLionProjects\\untitled\\primario.bin","r+b")) == NULL){
                printf("erro abertura arq primario.bin dentro funcao insere");
            }

            // apos insercao nos arqs main e primario, insere/atualiza arq secundario
            // tem q escrrever em 2 arqs -->
            // Primeiro: nome_autor + byteoffset desse autor no segundo arq
            // Segundo: ISBN + ponteiro da lista q aponta para outros byteoffsets nesse arq (os q tiverem são o mesmo nome de autor com isbn diferentes)

            break;
        }
        contador++;
    }
}

void BuscaPrimaria(FILE *Busca_P, FILE *out, FILE *prim, indice *vet, int tam_vetor){//

    char aux[14],temp[14];
    int tam_registro,i=0,flag=0;
    int contador=0,verifica=0;
    REGISTRO registro;


    rewind(Busca_P);
    rewind(prim);

    fread(&flag, sizeof(int), 1,prim);// le a flag para saber se a flag ta atualizado ou desatualizado

    if(flag==-1){
        CarregarIndice(prim, vet, &tam_vetor);// se desatualizzado , carrega o indice no arquivo
    }
    if(flag==-1 && vet[0].byteoffset==-1){
        //RecriarIndice(out,prim,vet);// recria o indice
        CarregarIndice(prim, vet, &tam_vetor); // carrega o indice reciado para a memoria
    }

//	printf("Chave que vamos buscar: %s\n",sizeof(vet[i].isbn));*/

    while(fread(&aux, sizeof(aux), 1,Busca_P)){// le os ISBNS do arquivo primario.bin
        if(aux[0]!='/'){
            printf("Chave que vamos buscar: %s\n",aux);

            while(vet[i].byteoffset!=-1 && i<tam_vetor){// se o vetor possui um registro na posição então  byteoffset é diferente de -1
                if(strcmp(vet[i].isbn,aux)==0){
                    verifica=1;//verifica=1 apenas se o ISBN tem um registro correspondente
                    break;
                }
                else{
                    i++;
                }
            }

            if(verifica!=1){
                printf("ISBN nao corresponde a nenhum registro no arquivo\n");
                aux[0]='/';
                fseek(Busca_P, (contador)*sizeof(aux), SEEK_SET);
                fwrite(&aux, sizeof(aux), 1,Busca_P);//marca o isbn do arquivo busca_p como ja lido(existem repeticoes, o que fazer com elas?)
                return;
            }

            fseek(out,vet[i].byteoffset,SEEK_SET);//posiciona o ponteiro do arquivo para byoffset do registro encontrado

            fread(&tam_registro, sizeof(int), 1, out);//le o tamanho do registro
            printf("byteoffset: %d\n",vet[i].byteoffset);

            fread(&registro, tam_registro, 1, out);//le o registro
            printf("registro: %s",registro);

            aux[0]='/';
            fseek(Busca_P, (contador)*sizeof(aux), SEEK_SET);
            fwrite(&aux, sizeof(aux), 1,Busca_P);//marca o isbn do arquivo busca_p como ja lido(existem repeticoes, o que fazer com elas?)

            break;
        }

        contador++;

    }

    return;
}

void CarregarIndice(FILE *prim, indice *vet, int *tam_vet){

    int i, header=1;

// -----------  FAZER VERIFICACAO CASO CHAME FUNCAO SEM ENTRAR NO INSERE ------------- //

    // reposiciona pro inicio do arq primario.bin e pula espaco do header para comecar a inserir
    rewind(prim);

    // atualiza header para 1 = atualizado
    fwrite(&header, sizeof(int), 1, prim);

    // escreve o que esta no vetor no arq primario apos heder
    for(i=0; i<*tam_vet; i++){

        fwrite(vet[i].isbn, sizeof(vet[i].isbn), 1, prim);
        fwrite(&vet[i].byteoffset, sizeof(int), 1, prim);
    }
}

void RecriarIndice(FILE *prim,indice *vet, int tam_vet){	// abre arq main.bin para leitura, recupera os dados, insere no vetor, escreve no arq primario
    int header = 1; // atualizado
    int i;
    rewind(prim);

    fwrite(&header, sizeof(int), 1, prim);		// adiciona header = 1 (atualizado)

    // ja recebe vetor atualizado com os dados do main.bin (ISBN + byteoffset) --> feito no main()
    qsort(vet, tam_vet, sizeof(vet[0]), comparador); 	// ordena vetor

    for(i=0; i<tam_vet; i++){
        printf("isbn: %d - byteoffset: %d \n", vet[i].isbn, vet[i].byteoffset);
        fwrite(vet[i].isbn, sizeof(vet[i].isbn), 1, prim);
        fwrite(&vet[i].byteoffset, sizeof(int), 1, prim);
    }
    fclose(prim);
    if( (prim = fopen("C:\\Users\\vmhug\\CLionProjects\\untitled\\primario.bin","r+b")) == NULL){
            printf("ta sertu aqui na recriar indice");
    }
}

void BuscaSecundaria(FILE *Busca_S){}

void CarregarArquivos(FILE *insere,FILE *Busca_P,FILE *Busca_S){}
