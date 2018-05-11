#include "aeroporto.h"
#include <stdlib.h>
#include <stdio.h>

/**
 * aeroporto.c
 * Implementação das funções do aeroporto.h
 * Descrições em aeroporto.h
 **/

aeroporto_t* iniciar_aeroporto (size_t* args, size_t n_args) {
	aeroporto_t* aeroporto = malloc(sizeof(aeroporto_t));
	aeroporto->n_pistas = args[0]-1;
    aeroporto->n_portoes = args[1];
    aeroporto->n_esteiras = args[2];
    aeroporto->n_max_avioes_esteira = args[3];
    aeroporto->t_pouso_decolagem = args[4];
    aeroporto->t_remover_bagagens = args[5];
    aeroporto->t_inserir_bagagens = args[6];
	aeroporto->t_bagagens_esteira = args[7];

	aeroporto->fila_pouso = criar_fila();
	aeroporto->fila_preferencial = criar_fila();
	pthread_mutex_init(&aeroporto->requer_pista, NULL);
	sem_init(&aeroporto->pistas, 0, aeroporto->n_pistas);
	sem_init(&aeroporto->portoes, 0, aeroporto->n_portoes);
	sem_init(&aeroporto->esteiras, 0, (aeroporto->n_esteiras * aeroporto->n_max_avioes_esteira));

	return aeroporto;
}

void aproximacao_aeroporto (aeroporto_t* aeroporto, aviao_t* aviao) {
	if (aviao->combustivel >=10) {
		inserir(aeroporto->fila_pouso, aviao);
	} else {
		inserir(aeroporto->fila_preferencial, aviao);
	}
	pthread_mutex_unlock(&aeroporto->requer_pista);
	printf("Avião %d, com %d de combustivel, requer uma pista. \n",
		(int) aviao->id, (int) aviao->combustivel);
}

void pousar_aviao (aeroporto_t* aeroporto, aviao_t* aviao) {
	sem_wait(&aviao->liberacao_pouso_decolagem);
	if (aviao->combustivel >= 10){
		remover(aeroporto->fila_pouso);
	} else {
		remover(aeroporto->fila_preferencial);
	}
	printf("bb\n");
	usleep(aeroporto->t_pouso_decolagem *1000);
	printf("Avião %d pousou com sucesso. \n", (int) aviao->id);
	sem_post(&aeroporto->pistas);
}

void acoplar_portao (aeroporto_t* aeroporto, aviao_t* aviao) {
	sem_wait(&aeroporto->portoes);
	printf("Avião %d aclopou em um portão\n", (int) aviao->id);
}

void transportar_bagagens (aeroporto_t* aeroporto, aviao_t* aviao) {
	sem_wait(&aeroporto->esteiras);
	usleep(aeroporto->t_remover_bagagens*1000);
	printf("Avião %d removeu as bagagens. \n", (int) aviao->id);
	adicionar_bagagens_esteira(aeroporto, aviao);
	sem_post(&aeroporto->esteiras);
	usleep(aeroporto->t_inserir_bagagens*1000);
	printf("Avião %d está com os novos passageiros e suas bagagens.\n", (int)aviao->id);
}

void adicionar_bagagens_esteira (aeroporto_t* aeroporto, aviao_t* aviao) {
	usleep(aeroporto->t_bagagens_esteira*1000);
}

void decolar_aviao (aeroporto_t* aeroporto, aviao_t* aviao) {
	printf("cc\n");
	if(sem_trywait(&aeroporto->pistas)==0){
		sem_post(&aeroporto->portoes);
	}else{
		printf("EMERGENCIA\n");
		inserir(aeroporto->fila_preferencial, aviao);
	}
	sem_wait(&aviao->liberacao_pouso_decolagem);
	usleep(aeroporto->t_pouso_decolagem*1000);
	printf("Avião %d decolou. Boa viagem\n", (int) aviao->id);
	sem_post(&aeroporto->pistas);
	pthread_exit(NULL);
}

int finalizar_aeroporto (aeroporto_t* aeroporto) {
	desaloca_fila(aeroporto->fila_pouso);
	desaloca_fila(aeroporto->fila_preferencial);
	pthread_mutex_destroy(&aeroporto->requer_pista);
	sem_destroy(&(aeroporto->pistas));
	sem_destroy(&(aeroporto->portoes));
	sem_destroy(&(aeroporto->esteiras));
	free(aeroporto);

	return 1;
}
