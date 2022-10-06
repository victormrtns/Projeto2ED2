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
void Inserir(FILE *insere, FILE *out, FILE *prim,indice *vet);
void BuscaPrimaria(FILE *Busca_P);
void CarregarIndice();
void RecriarIndice();
void BuscaSecundaria(FILE *Busca_S);
void CarregarArquivos(FILE *insere, FILE *Busca_P, FILE *Busca_S);

int main(){

    FILE *insere;		// ponteiro arq insere.bin
    FILE *Busca_P;
    FILE *Busca_S;
    FILE *out;			// ponteiro arq main.bin (guarda todos os dados)
    FILE *prim;			// ponteiro arq primario.bin (indice com ISBNs + byteoffset)

    REGISTRO REG;
    int opcao;
    //Inicializo o contador como 1 pra dar certo o num de posicoes
    int contador = 1;


    if( (out = fopen("C:\\Users\\vmhug\\CLionProjects\\untitled\\main.bin","r+b")) == NULL){

        out = fopen("C:\\Users\\vmhug\\CLionProjects\\untitled\\main.bin","w+b");
    }

    if( (insere = fopen("C:\\Users\\vmhug\\CLionProjects\\untitled\\insere.bin","r+b")) == NULL){

        printf("ERRO NA ABERTURA DO ARQUIVO");
    }

    if( (prim = fopen("C:\\Users\\vmhug\\CLionProjects\\untitled\\primario.bin","r+b")) == NULL){

        prim = fopen("C:\\Users\\vmhug\\CLionProjects\\untitled\\primario.bin","w+b");
        // adiciona header inicializando com -1 (desatualizado)
        int num = -1;
        fwrite(&num, sizeof(int), 1, prim);
    }

    //Aqui esse contador vai pegar a quantidade de registros no arquivo insere
    while( fread(&REG, sizeof(REG), 1, insere) ){
        contador++;
    }
    rewind(insere);
    //Aqui ele printa pra ver se ta certo
    printf("%d\n",contador);
    indice *vet;
    //Faz o malloc de todas as posicoes (6 no caso) nesse vetor
    vet  = (indice *)malloc(contador*sizeof(indice));
    rewind(insere);

    do{
        printf("\n MENU\n 0-Sair\n 1-Inserir \n 2-BuscaPrimaria\n 3-BuscaSecundaria \n 4-CarregarArquivos\n Opcao: ");
        scanf("%d",&opcao);

        switch(opcao){
            case 0:{
                break;
            }
            case 1:{
                Inserir(insere, out, prim,vet);
                break;
            }
            case 2:{
                BuscaPrimaria(Busca_P);
                break;
            }
            case 3:{
                BuscaSecundaria(Busca_S);
                break;
            }
            case 4:{
                CarregarArquivos(insere,Busca_P,Busca_S);
                break;
            }
            default:{
                printf("Opcao Invalida\n");
                break;
            }

        }
    }while(opcao!=0);

    free(vet);
    fclose(out);
    fclose(insere);
    fclose(prim);
/*	fclose(Busca_P);
	fclose(Busca_S);*/

    return 0;
}
void Inserir(FILE *insere, FILE *out, FILE *prim,indice *vet){//

    int tam_registro=0, contador=0;
    char aux[14], registro[125];
    REGISTRO REG;

    int byteoffset;

    rewind(insere);
    rewind(out);
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

            // apos insercao no main, insere ISBN + byteoffset no vetor (indice)
            sprintf(vet[contador].isbn, "%s", aux);
            vet[contador].byteoffset = byteoffset;

// -------------- ordenar vetor quando for inserir, verificacao de quando fechar descarregar vetor para arq primario ----------------------- //

            //marca o registro como lido no insere.bin
            aux[0] = '/';
            fseek(insere, (contador)*sizeof(REG), SEEK_SET);
            fwrite(&aux, sizeof(REG.ISBN), 1, insere);


            // apos insercao nos arqs main e primario, insere/atualiza arq secundario
            // tem q escrrever em 2 arqs -->
            // Primeiro: nome_autor + byteoffset desse autor no segundo arq
            // Segundo: ISBN + ponteiro da lista q aponta para outros byteoffsets nesse arq (os q tiverem são o mesmo nome de autor com isbn diferentes)

            break;
        }
        contador++;
    }


    for(int i = 0 ;i<contador+1;i++){
        printf("%s\n",vet[i].isbn);
        printf("%d\n",vet[i].byteoffset);
    }
}
void BuscaPrimaria(FILE *Busca_P){//

    FILE *arq;

    if ((arq = fopen("dados.bin","w+b")) == NULL)
    {
        printf("o arquivo ja existe\n");
        return ;
    }

    fclose(arq);
}
void CarregarIndice(){//




}
void RecriarIndice(){//




}
void BuscaSecundaria(FILE *Busca_S){//



}
void CarregarArquivos(FILE *insere,FILE *Busca_P,FILE *Busca_S){//



    if ((insere = fopen("insere.bin","rb")) == NULL)
    {
        return ;
    }

    if ((Busca_P = fopen("busca_p.bin","rb")) == NULL)
    {
        return ;
    }

    if ((Busca_S = fopen("busca_s.bin","rb")) == NULL)
    {
        return ;
    }

    fclose(insere);
    fclose(Busca_P);
    fclose(Busca_S);

}