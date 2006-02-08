#include "libc.h"

struct chunk {
    struct chunk *next;
    int num;
    int data[0];
};

void free_all(struct chunk *p) {
    if (p->next)
	free_all(p->next);
    free(p);
}

int main(int argc, char **argv) {
    int i;
    struct chunk *head = 0, *p;
    for (i = 0; i < 100; i++) {
	struct chunk *new_chunk = malloc(sizeof(struct chunk)
					 + i * sizeof(int));
	int j;
	/*printf("Allocated chunk %d at %p\n", i, new_chunk);*/
	new_chunk->next = head;
	new_chunk->num = i;
	head = new_chunk;
	for (j = 0; j < i; j++) {
	    new_chunk->data[j] = i;
	}
    }

    for (p = head; p; p = p->next) {
	int n = p->num;
	int j;
	for (j = 0; j < n; j++) {
	    if (p->data[j] != n) {
		printf("Bad data %d in chunk %d location %d\n",
		       p->data[j], n, j);
		return 1;
	    }
	}
    }

    free_all(head);
    return 0;
}
    
