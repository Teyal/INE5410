#include "aviao.h"
#include <stdlib.h>

/**
 * aviao.c
 * Implementação das funções do aviao.h
 * Descrições em aviao.h
 **/

aviao_t * aloca_aviao (size_t combustivel, size_t id) {
	aviao_t* aviao = (aviao_t*) malloc(sizeof(aviao_t));
	aviao->combustivel = combustivel;
	aviao->id = id;
	return aviao;
}

void desaloca_aviao(aviao_t* aviao) {
	pthread_exit(&(aviao->thread));
	free(aviao);
}
