#ifndef AVIAO_H
#define AVIAO_H

#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>

/**
 * O id deve ser usado pra indentificar individualmente cada avião e
 * sua a situação do avião no programa.
 **/
typedef struct {
  pthread_t thread; // Uso dos recursos pelo avião é monitorado pela thread
  size_t combustivel; // Indica prioridade do avião para pousar
  size_t id; // Identificador do avião: 0, 1, 2, ...
  sem_t liberacao_pouso_decolagem;
} aviao_t;

// Estas funcoes devem cuidar da alocacao dinâmica de memória
aviao_t * aloca_aviao (size_t combustivel, size_t id);
void desaloca_aviao (aviao_t* aviao);

#endif
