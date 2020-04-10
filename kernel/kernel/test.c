#include <stdio.h>
#include <stdlib.h>

struct BST
{
	int value;
	struct BST *left;
	struct BST *right;
};

typedef struct BST BST;

BST *make(int v, BST* g, BST *d)
{
	BST *s = (BST*) malloc(sizeof(BST));
	s->value = v;
	s->left = g;
	s->right = d;
	return s;
}

void add(BST **a, int x) 
{
	BST *t = *a;
	if (!t) 
	{
		*a = make(x, NULL, NULL);
		return;
	}
	if (x < t->value) 
		add(&t->left, x);
	else if (x > t->value)
		add(&t->right, x);
}

int mem(BST *a, int x) 
{
	if (x == a->value) 
		return 1;
	if (x < a->value) 
		return mem(a->left, x);
	if (a->right)
		return mem(a->right, x);
	return 0;
}

void print(BST* a) 
{
	if (!a)
		return;
	printf("(");
	if (a->left) 
		print(a->left);
	printf("%d",a->value);
	if (a->right) 
		print(a->right);
	printf(")");
}

void test_main() 
{
	BST *dico = 0;
	for (int i = 1 ; i < 10 ; i++)
	{
		int x = (55 * i) % 34;
		add(&dico, x);
		print(dico);
		printf("\n");
	}
	add(&dico, 42);
	add(&dico, -1);
	print(dico); 
	printf("\n");
}

