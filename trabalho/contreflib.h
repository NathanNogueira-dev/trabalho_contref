#ifndef CONTREF_H
#define CONTREF_H

#include <stdio.h>
#include <stdlib.h>

/* Estrutura */
typedef struct RefInfo {
    void *ptr;          // bloco alocado
    size_t refs;        // contador
    struct RefInfo *next;
} RefInfo;

/* Lista global de blocos rastreados */
static RefInfo *ref_list = NULL;

/* Localiza bloco */
static RefInfo *ref_find(void *p) {
    for (RefInfo *r = ref_list; r; r = r->next)
        if (r->ptr == p)
            return r;
    return NULL;
}

/* Aloca com contador = 1 */
static void *malloc2(size_t sz) {
    void *p = malloc(sz);
    if (!p) {
        fprintf(stderr, "[contref] erro: malloc falhou\n");
        exit(1);
    }

    RefInfo *r = malloc(sizeof(RefInfo));
    r->ptr  = p;
    r->refs = 1;
    r->next = ref_list;
    ref_list = r;

    return p;
}

/* Reduz contador e libera quando necessário */
static void ref_release(void *p) {
    RefInfo *r = ref_find(p);
    if (!r) return;

    r->refs--;

    if (r->refs == 0) {
        // remove da lista
        if (ref_list == r) {
            ref_list = r->next;
        } else {
            RefInfo *aux = ref_list;
            while (aux->next != r) aux = aux->next;
            aux->next = r->next;
        }

        free(r->ptr);
        free(r);
    }
}

/*
 atrib2(&a, b)
 Faz a troca com contagem:
    decrementa antigo
    incrementa novo
    a = b
*/
static void atrib2(void *orig, void *novo) {
    void *old = *(void**)orig;


    // incrementa novo se existir
    RefInfo *r_new = ref_find(novo);
    if (r_new)
        r_new->refs++;

    // decrementa antigo
    RefInfo *r_old = ref_find(old);
    if (r_old)
        ref_release(old);

    *(void**)orig = novo;
}

/* Mostra status atual */
static void dump() {
    printf("\n=== CONTREF: estado atual ===\n");
    if (!ref_list) {
        printf("(nenhum bloco rastreado)\n\n");
        return;
    }

    for (RefInfo *r = ref_list; r; r = r->next)
        printf("ptr=%p | refs=%zu\n", r->ptr, r->refs);

    printf("==============================\n\n");
}

#endif
