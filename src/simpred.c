/**************************************************
*
* Gabriel da Silva Chiquetto e Leonardo Cavalcante Almeida Rocha
* Simulador de predição de Desvios Condicionais
* Professora: Nahri Balesdent Moreano
* Compilar utilizando?
*
*/
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

//Stat
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


extern int errno;

// Estrutura de dados que guarda a predição
typedef struct 
{
	int predict;
}BPB_l;

// Estrutura de dados que guarda o Branch prediction buffer, sendo size o seu tamanho e BPB_l * linha um ponteiro para cada linha
typedef struct 
{
	int size;
	BPB_l * linha;
}BPB;

enum boolean {
    T = 1,
    N = 0,
};

typedef enum boolean _bool;

// Função que cria um Branch Prediction Buffer
BPB* cria_BPB(int num_linhas_BPB);

// Função que lê o trace e chama a função pred_NT
float read_trace_nt(FILE *trace);

// Função que lê o trace e chama a função pred_1B
float read_trace_1b(FILE *trace, int num_linhas_BPB);

// Função que lê o trace e chama a função pred_2B
float read_trace_2b(FILE *trace, int num_linhas_BPB);

// Função que lê o trace e chama a função pred_CR
float read_trace_cr(FILE *trace, int num_linhas_BPB, int m, int n);

// Função que faz a predição de desvio Not Taken
int pred_nt(char verify);

// Função que faz a predição de desvio usando BPB de 1 bit
int pred_1b(unsigned int address, char verify, BPB * buffer);

// Função que faz a predição de desvio usando BPB de 2 bits
int pred_2b(unsigned int address, char verify, BPB * buffer);

// Função que faz a predição de desvio usando correlação.
int pred_cr(unsigned int address, char verify, BPB * buffer, int m, int n_bits, _bool * branch_h);

/**
 * Abre o arquivo com thread safe
*/ 
FILE* openFile(char* path, char* flag);

int main(int argc, char **argv)
{
	int num_linhas_BPB;
	int m;
	int n;
	float erro;

	FILE *trace = NULL;

	trace = openFile( argv[2], "rb");

	if(strcmp(argv[1], "nt") == 0)
	{	
		//printf("\nnt\n");
		erro = read_trace_nt(trace);

	}else if(strcmp(argv[1], "1b") == 0)
	{	
		num_linhas_BPB = strtoll(argv[3], NULL, 10);
		//printf("\n1b\n");
		erro = read_trace_1b(trace, num_linhas_BPB);

	}else if(strcmp(argv[1], "2b") == 0)
	{	
		num_linhas_BPB = strtoll(argv[3], NULL, 10);
		//printf("\n2b\n");
		erro = read_trace_2b(trace, num_linhas_BPB);

	}else if(strcmp(argv[1], "cr") == 0)
	{	
		num_linhas_BPB = strtoll(argv[3], NULL, 10);
		m = strtol(argv[4], NULL, 10);
		n = strtol(argv[5], NULL, 10);
		//printf("m = %d, n = %d\n", m, n);
		//printf("\ncr\n");
		erro = read_trace_cr(trace, num_linhas_BPB, m, n);

	}else
	{
		printf("\nTécnica de predição inválida.\n");
		return 0;
	}
	//printf("num_linhas_BPB = %d\n", num_linhas_BPB);
	printf("taxa de acertos = %%%.2f\n", 100-erro*100);

	return 0;
}

FILE* openFile(char* path, char* flag)
{

	FILE* fp = fopen(path, flag);
	if(fp == NULL)
	{
		int errnumb = errno;
		fprintf(stderr, "Value of errno: %d\n", errno);
      	perror("Error printed by perror");
      	fprintf(stderr, "Error opening file: %s\n", strerror( errnumb ));
	}
	return fp;
}

//TODO corrigir desaloca
void desaloca(BPB * buffer){
	if(buffer != NULL)
	{	
		for (int i = buffer->size - 1; i >= 0 ; i--)
		{	
        	if(&(buffer->linha[i]) != NULL)
        	{	
		    	free(&(buffer->linha[i]));
        	}
    	}
    	free(buffer);
	}
}

BPB* cria_BPB(int num_linhas_BPB)
{
	BPB* pred_buffer;

	pred_buffer = (BPB*) malloc(sizeof(BPB));
	pred_buffer->size = num_linhas_BPB;
	pred_buffer->linha = (BPB_l*) calloc(1, sizeof(BPB_l) * num_linhas_BPB);

	return pred_buffer;
}

int testa_BPB(BPB * buffer, int num_linhas_BPB){
	if(buffer->size != num_linhas_BPB)
	{
		printf("size != num_linhas_BPB");
		return 1;
	}
	for (int i = 0; i < buffer->size; ++i)
	{
		if(buffer->linha[i].predict != 0)
		{	
			printf("predict errado\n");
			return 1;
		}
	}
	return 0;
}

unsigned int calcula_Address(unsigned int Address, int size)
{	
	int n = (int) log2((double) size);
	if(size == 1) return 0;
	Address = Address >> 2;
    Address = Address << (32 - n);
    Address = Address >> (32 - n);
    return Address;
}


float read_trace_nt(FILE *trace)
{
	char Desvio;
	unsigned int Address;
	int i = 0;
	int N = 0;
	int erro = 0;

	while(!feof(trace))// && i < 30)
	{	
		fscanf(trace, "%u %c", &Address, &Desvio);
		erro += pred_nt(Desvio); 
		i++;
		if(Desvio == 'T') N++;

		// printf("Address = %u, D = %c\n", Address, Desvio);
	}

	return ((float)--erro)/((float)--i);
}

float read_trace_1b(FILE *trace, int num_linhas_BPB)
{
	char Desvio;
	unsigned int Address;
	int i = 0;
	int erro = 0;
	BPB* BPB = cria_BPB(num_linhas_BPB);
	//int teste = testa_BPB(BPB);
	//if(teste == 0) printf("BPB criado com sucesso!\n");
	while(!feof(trace))// && i < 30)
	{	
		fscanf(trace, "%u %c", &Address, &Desvio);
		erro += pred_1b(Address, Desvio, BPB); 
		i++;
		// printf("Address = %u, D = %c\n", Address, Desvio);
	}
	desaloca(BPB);
	return ((float)erro)/((float)--i);
}

float read_trace_2b(FILE *trace, int num_linhas_BPB)
{
	char Desvio;
	unsigned int Address;
	int erro = 0;
	int i = 0;

	BPB* BPB = cria_BPB(num_linhas_BPB);
	while(!feof(trace))// && i < 30)
	{	
		fscanf(trace, "%u %c", &Address, &Desvio);
		erro += pred_2b(Address, Desvio, BPB); 
		i++;
		// printf("Address = %u, D = %c\n", Address, Desvio);
	}
	desaloca(BPB);
	return ((float)erro)/((float)--i);
}

float read_trace_cr(FILE *trace, int num_linhas_BPB, int m, int n)
{

    char Desvio;
	unsigned int Address;
    int erro = 0;
    int i = 0;
	_bool * branch_historico = (_bool *) calloc(1, m*sizeof(_bool));
    BPB** buffers_cr = (BPB**) malloc((2<<(m-1)) * sizeof(BPB*));
	unsigned int cr_address = 0;
		
	for(int j = 0; j < (2<<(m-1)); j++){
        buffers_cr[j] = cria_BPB(1);
        //printf("%d -> buffers_cr[%d]->size = %d\n",__LINE__, j, buffers_cr[j]->size);
        //int teste = testa_BPB(buffers_cr[j],num_linhas_BPB);
    }
    while(!feof(trace))
    {
    	cr_address = 0;
    	for (int j = 0; j < m; ++j)
    	{
    		cr_address += branch_historico[j] << j; 
    	}
    	//printf("cr_address = %d", cr_address);
        fscanf(trace, "%u %c", &Address, &Desvio);        
        erro += pred_cr(Address, Desvio, buffers_cr[cr_address], m, n, branch_historico); 
        i++;   
    }
     for (int j = m; j >= 0; --j)
    {
    	desaloca(buffers_cr[j]);
    }
	return ((float)erro)/((float)--i);
}

int pred_nt(char verify)
{
	return verify == 'N'? 0 : 1;
}

int pred_1b(unsigned int address, char verify, BPB * buffer)
{
	int ret = 0;
	
	address = calcula_Address(address, buffer->size);

	if(buffer->linha[address].predict != (verify == 'T'))
		ret = 1;

	buffer->linha[address].predict = (verify == 'T');

	return ret;
}

int pred_2b(unsigned int address, char verify, BPB * buffer)
{
    address = calcula_Address(address, buffer->size);

    switch(buffer->linha[address].predict)
    {
        case 0 :
            if(verify == 'T')
            {
                buffer->linha[(int) address].predict = 1;
                return 1;
            }
            break;
        case 1 :
            if(verify == 'T')
            {
                buffer->linha[(int) address].predict = 2;
                return 1;
            }
            else
            {
                buffer->linha[(int) address].predict = 0;
            }
            break;
        case 2 :
            if(verify == 'N')
            {
                buffer->linha[(int) address].predict = 3;
                return 1;
            }
            break;
        case 3 :
            if(verify == 'N')
            {
                buffer->linha[(int) address].predict = 0;
                return 1;
            }
            else
            {
                buffer->linha[(int) address].predict = 2;
            }
            break;
        default :
            break;
    }
    return 0;
}


//TODO FIX pred_cr
int pred_cr(unsigned int address, char verify, BPB * buffer, int m, int n_bits, _bool * branch_h)
{
	int ret = 0;
	//printf("Buffer->size = %d\n", buffer->size);
	address = calcula_Address(address, buffer->size);
	int max = (2<<(n_bits-1)) - 1, min = 0, threshold = 2 << (n_bits-2);
	//printf("\nmax = %d, threshold = %d\n", max, threshold);

	if((verify == 'T') != ((buffer->linha[address].predict) >= threshold))
	{
		ret = 1;
	}
	//printf("PASSOU DO PRIMEIRO IF\n");
	if((verify == 'T') && ((buffer->linha[address].predict) != max))
	{
		(buffer->linha[address].predict)++;
	}

	if((verify == 'N') && ((buffer->linha[address].predict) != min))
	{
		(buffer->linha[address].predict)--;
	}

	for(int j = m-2; j >= 0; j--)
	{
		branch_h[j+1] = branch_h[j];
	}

	branch_h[0] = (verify == 'T');

	return ret;
}