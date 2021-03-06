/**========================================================
| UTFPR  2018   Operating System
| https://github.com/gabrielsouzaesilva/
| https://github.com/cordeirolibel/
=========================================================**/

#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include "datatypes.h"		// estruturas de dados necessárias

#define STACKSIZE 32768		/* tamanho de pilha das threads */

//==============================================================
// Variaveis globais ===========================================

// Estrutura que define uma tarefa main
task_t *tk_main;
task_t *tk_atual;
ucontext_t ct_main;

int id_tasks;

//==============================================================
// Funções gerais ==============================================

// Inicializa o sistema operacional; deve ser chamada no inicio do main()
void pingpong_init () {

	// desativa o buffer da saida padrao (stdout), usado pela função printf 
	setvbuf (stdout, 0, _IONBF, 0);

	// ===> Criando tarefa main
	// C++  >> task_t* tk_main = new task_t;
	tk_main = malloc(sizeof(*tk_main)); 
	// C++  >> ucontext_t* ct_main = new ct_main;
	ucontext_t* ct_main = malloc(sizeof(*ct_main));
	getcontext(ct_main); // armazena contexto atual em ct_main
	tk_main->context = ct_main;

	//id de tarefas inicia em 0
	id_tasks = 0;

	//tk_aux = NULL;

	//Referencia para task atual
	tk_atual = tk_main;

	//print debug
	#ifdef DEBUG
	printf ("pingpong init conlcuido.\n");
	#endif

	return;
}

//==============================================================
// Gerência de tarefas =========================================

// Cria uma nova tarefa. Retorna um ID> 0 ou erro.
int task_create (task_t *task,			// descritor da nova tarefa
                 void (*start_func)(void *),	// funcao corpo da tarefa
                 void *arg) {			// argumentos para a tarefa

	// Crianto contexto
	ucontext_t* context = malloc(sizeof(*context));

	getcontext (context);

	//Criando Pilha
	char *stack = malloc (STACKSIZE) ;
	if (stack) {
		context->uc_stack.ss_sp = stack ;
		context->uc_stack.ss_size = STACKSIZE;
		context->uc_stack.ss_flags = 0;
		context->uc_link = 0;
	}
	else {
		perror ("Erro na criação da pilha: ");
		exit (1);
	}

	//ajusta id
	id_tasks++;
	task->tid = id_tasks;
	task->context = context;

	// ajusta alguns valores internos do contexto salvo em context
	makecontext (context, (void*)(*start_func), 1, arg);

	#ifdef DEBUG
	printf ("task_create: criou tarefa %d\n", task->tid) ;
	#endif

	return id_tasks;
}



// alterna a execução para a tarefa indicada
int task_switch(task_t *task) {
	task_t *tk_aux;
	int result = 0;
	tk_aux = tk_atual;
	tk_atual = task;
	result = swapcontext(tk_aux->context, tk_atual->context); // swapcontex retorna 0 se ok ou -1 se der erro.

	#ifdef DEBUG
	printf ("task_switch: trocando contexto %d -> %d\n", tk_aux->tid, tk_atual->tid);
	#endif

	return result;
}


// Termina a tarefa corrente, indicando um valor de status encerramento
void task_exit (int exitCode) {

	// desalocar
	free(tk_atual->context);

	#ifdef DEBUG
	printf ("task_exit: tarefa %d sendo encerrada\n", tk_atual->tid);
	#endif

	return;
}


// retorna o identificador da tarefa corrente (main eh 0)
int task_id ()
{
	return tk_atual->tid;
}

// suspende uma tarefa, retirando-a de sua fila atual, adicionando-a à fila
// queue e mudando seu estado para "suspensa"; usa a tarefa atual se task==NULL
void task_suspend (task_t *task, task_t **queue) ;

// acorda uma tarefa, retirando-a de sua fila atual, adicionando-a à fila de
// tarefas prontas ("ready queue") e mudando seu estado para "pronta"
void task_resume (task_t *task) ;

//==============================================================
// Operações de escalonamento ==================================

// libera o processador para a próxima tarefa, retornando à fila de tarefas
// prontas ("ready queue")
void task_yield () ;

// define a prioridade estática de uma tarefa (ou a tarefa atual)
void task_setprio (task_t *task, int prio) ;

// retorna a prioridade estática de uma tarefa (ou a tarefa atual)
int task_getprio (task_t *task) ;

//==============================================================
// Operações de sincronização ==================================

// a tarefa corrente aguarda o encerramento de outra task
int task_join (task_t *task) ;

//==============================================================
// Operações de gestão do tempo ================================

// suspende a tarefa corrente por t segundos
void task_sleep (int t) ;

// retorna o relógio atual (em milisegundos)
unsigned int systime () ;

//==============================================================
// Operações de IPC ============================================

// semáforos

// cria um semáforo com valor inicial "value"
int sem_create (semaphore_t *s, int value) ;

// requisita o semáforo
int sem_down (semaphore_t *s) ;

// libera o semáforo
int sem_up (semaphore_t *s) ;

// destroi o semáforo, liberando as tarefas bloqueadas
int sem_destroy (semaphore_t *s) ;

// mutexes

// Inicializa um mutex (sempre inicialmente livre)
int mutex_create (mutex_t *m) ;

// Solicita um mutex
int mutex_lock (mutex_t *m) ;

// Libera um mutex
int mutex_unlock (mutex_t *m) ;

// Destrói um mutex
int mutex_destroy (mutex_t *m) ;

// barreiras

// Inicializa uma barreira
int barrier_create (barrier_t *b, int N) ;

// Chega a uma barreira
int barrier_join (barrier_t *b) ;

// Destrói uma barreira
int barrier_destroy (barrier_t *b) ;

// filas de mensagens

// cria uma fila para até max mensagens de size bytes cada
int mqueue_create (mqueue_t *queue, int max, int size) ;

// envia uma mensagem para a fila
int mqueue_send (mqueue_t *queue, void *msg) ;

// recebe uma mensagem da fila
int mqueue_recv (mqueue_t *queue, void *msg) ;

// destroi a fila, liberando as tarefas bloqueadas
int mqueue_destroy (mqueue_t *queue) ;

// informa o número de mensagens atualmente na fila
int mqueue_msgs (mqueue_t *queue) ;
