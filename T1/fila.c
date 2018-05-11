#include "fila.h"
#include <stdlib.h>

/**
 * fila.c
 * Implementação das funções da fila.h
 * Descrições em fila.h
 **/

 elemento_t * aloca_elemento (aviao_t * dado) {
    elemento_t* elem = (elemento_t*) malloc(sizeof(elemento_t));
    elem->dado = dado;
    return elem;
 }

 void desaloca_elemento (elemento_t * elemento) {
 	free(elemento);
 }

fila_ordenada_t * criar_fila () {
    fila_ordenada_t* fila = (fila_ordenada_t*) malloc(sizeof(fila_ordenada_t));
    fila->n_elementos = 0;
    pthread_mutex_init(&fila->mutex, NULL);
    return fila;
}

void desaloca_fila (fila_ordenada_t * fila) {
	elemento_t* elem = fila->primeiro;

    for (size_t i = 0; i < fila->n_elementos; i++) {
        elemento_t* prox = elem->proximo;
        desaloca_elemento(elem);
        elem = prox;
    }

    pthread_mutex_destroy(&fila->mutex);
	free(fila);
}

void inserir (fila_ordenada_t * fila, aviao_t * dado) {
	pthread_mutex_lock(&fila->mutex);
	elemento_t* elem = aloca_elemento(dado);

    if (fila->n_elementos == 0) {
        fila->primeiro = elem;
        fila->ultimo = elem;
    } else {
        elem->anterior = fila->ultimo;
        elem->anterior->proximo = elem;
        fila->ultimo = elem;
    }
	fila->n_elementos++;
	pthread_mutex_unlock(&fila->mutex);
}

aviao_t * remover (fila_ordenada_t * fila) {
	pthread_mutex_lock(&fila->mutex);
	if (fila->n_elementos > 0) {
        elemento_t* elem = fila->primeiro;
        aviao_t* aviao = elem->dado;
        fila->primeiro = elem->proximo;
        fila->n_elementos--;
        desaloca_elemento(elem);
        pthread_mutex_unlock(&fila->mutex);

        return aviao;
    } else {
        pthread_mutex_unlock(&fila->mutex);
        return NULL;
	}
}
