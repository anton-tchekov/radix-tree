#include "symtab.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#define CAPACITY 1024

static void test_complete(void)
{
	int rv;
	char buf[256];
	SymTab *tab;

	printf("\ntest_complete\n");

	tab = symtab_create(CAPACITY);

	symtab_put(tab, "main", 1);
	symtab_put(tab, "test_put", 2);
	symtab_put(tab, "symtab_create", 3);
	symtab_put(tab, "symtab_destroy", 4);
	symtab_put(tab, "symtab_put", 5);
	symtab_put(tab, "symtab_get", 6);
	symtab_put(tab, "test_exists", 7);

	symtab_print(tab);

	strcpy(buf, "sy");
	rv = symtab_complete(tab, buf);
	printf("%s\n", buf);
	assert(!strcmp(buf, "symtab_"));
	assert(rv == 1);

	strcpy(buf, "symtab_insert");
	rv = symtab_complete(tab, buf);
	printf("%s\n", buf);
	assert(!strcmp(buf, "symtab_insert"));
	assert(rv == 0);

	strcpy(buf, "main");
	rv = symtab_complete(tab, buf);
	printf("%s\n", buf);
	assert(!strcmp(buf, "main"));
	assert(rv == 0);

	symtab_destroy(tab);
}

static void iter_callback(void *data, char *ident)
{
	int *n = data;
	printf("%d: %s\n", *n, ident);
	++(*n);
}

static void test_prefix_iter(void)
{
	int cnt = 0;
	char buf[256] = "sy";
	SymTab *tab;

	printf("\ntest_prefix_iter\n");

	tab = symtab_create(CAPACITY);

	symtab_put(tab, "main", 1);
	symtab_put(tab, "test_put", 2);
	symtab_put(tab, "symtab_create", 3);
	symtab_put(tab, "symtab_destroy", 4);
	symtab_put(tab, "symtab_put", 5);
	symtab_put(tab, "symtab_get", 6);
	symtab_put(tab, "test_exists", 7);

	symtab_prefix_iter(tab, buf, 3, &cnt, iter_callback);
	assert(cnt == 3);

	symtab_destroy(tab);
}

static void test_put_get(void)
{
	SymTab *tab;

	printf("\ntest_put_get\n");

	tab = symtab_create(CAPACITY);

	symtab_put(tab, "hello", 7);
	symtab_put(tab, "world", 2);
	symtab_put(tab, "test", 5);
	symtab_put(tab, "team", 9);
	symtab_put(tab, "toast", 4);
	symtab_put(tab, "te", 11);
	symtab_put(tab, "browser", 42);
	symtab_put(tab, "brow", 9);

	assert(symtab_get(tab, "toast") == 4);
	assert(symtab_get(tab, "brow") == 9);
	assert(symtab_get(tab, "browser") == 42);

	symtab_put(tab, "toast", 77);

	symtab_print(tab);

	assert(symtab_get(tab, "team") == 9);
	assert(symtab_get(tab, "world") == 2);
	assert(symtab_get(tab, "hello") == 7);
	assert(symtab_get(tab, "te") == 11);
	assert(symtab_get(tab, "test") == 5);
	assert(symtab_get(tab, "toast") == 77);

	assert(symtab_get(tab, "t") == 0);
	assert(symtab_get(tab, "nonexistant") == 0);
	assert(symtab_get(tab, "toaster") == 0);
	assert(symtab_get(tab, "team member") == 0);
	assert(symtab_get(tab, "hello world") == 0);
	assert(symtab_get(tab, "testing") == 0);
	assert(symtab_get(tab, "the world") == 0);

	assert(symtab_get(tab, "") == 0);

	symtab_destroy(tab);
}

static void test_remove_prefix(void)
{
	SymTab *tab;

	printf("\ntest_remove_prefix\n");

	tab = symtab_create(CAPACITY);

	symtab_put(tab, "testing", 99);
	symtab_put(tab, "test", 33);

	symtab_print(tab);

	assert(symtab_get(tab, "test") == 33);
	symtab_remove(tab, "test");
	assert(symtab_get(tab, "test") == 0);

	symtab_destroy(tab);
}

static void test_remove_suffix(void)
{
	SymTab *tab;

	printf("\ntest_remove_suffix\n");

	tab = symtab_create(CAPACITY);

	symtab_put(tab, "testing", 99);
	symtab_put(tab, "test", 33);

	symtab_print(tab);

	assert(symtab_get(tab, "testing") == 99);
	symtab_remove(tab, "testing");
	assert(symtab_get(tab, "testing") == 0);
	symtab_print(tab);

	symtab_destroy(tab);
}

static void test_remove_branch(void)
{
	SymTab *tab;

	printf("\ntest_remove_branch\n");

	tab = symtab_create(CAPACITY);

	symtab_put(tab, "team", 22);
	symtab_put(tab, "test", 55);

	symtab_print(tab);

	assert(symtab_get(tab, "test") == 55);
	symtab_remove(tab, "test");
	assert(symtab_get(tab, "test") == 0);
	symtab_print(tab);

	symtab_destroy(tab);
}

static void test_remove_prev_branch(void)
{
	SymTab *tab;

	printf("\ntest_remove_prev_branch\n");

	tab = symtab_create(CAPACITY);

	symtab_put(tab, "team", 22);
	symtab_put(tab, "test", 55);
	symtab_put(tab, "toast", 44);

	symtab_print(tab);

	assert(symtab_get(tab, "toast") == 44);
	symtab_remove(tab, "toast");
	assert(symtab_get(tab, "toast") == 0);
	symtab_print(tab);
	assert(symtab_get(tab, "team") == 22);
	assert(symtab_get(tab, "test") == 55);

	symtab_destroy(tab);
}

static void test_remove(void)
{
	SymTab *tab;

	printf("\ntest_remove\n");

	tab = symtab_create(CAPACITY);

	/* Test remove when empty */
	assert(symtab_remove(tab, "bla") == 0);
	assert(symtab_remove(tab, "") == 0);

	symtab_put(tab, "hello", 1);
	symtab_put(tab, "hello world", 1);
	symtab_put(tab, "world", 2);

	symtab_print(tab);

	/* Test remove */
	assert(symtab_get(tab, "hello world") == 1);
	symtab_remove(tab, "hello world");
	assert(symtab_get(tab, "hello world") == 0);

	/* Test remove non-existant */
	assert(symtab_remove(tab, "hello123") == 0);
	assert(symtab_remove(tab, "nonsense") == 0);
	assert(symtab_remove(tab, "goodbye") == 0);
	assert(symtab_remove(tab, "hel") == 0);
	assert(symtab_remove(tab, "h") == 0);
	assert(symtab_remove(tab, "something") == 0);
	assert(symtab_remove(tab, "") == 0);

	symtab_print(tab);

	assert(symtab_get(tab, "hello") == 1);
	assert(symtab_get(tab, "world") == 2);

	symtab_destroy(tab);
}

static void test_cmdline(void)
{
	SymTab *tab = symtab_create(CAPACITY);
	char buf[256];
	char *delim = " \n";
	char *p;
	int running = 1;

	while(running)
	{
		printf("> ");
		fgets(buf, sizeof(buf), stdin);

		p = strtok(buf, delim);
		if(p == NULL)
		{
			continue;
		}

		if(!strcmp(p, "quit"))
		{
			running = 0;
		}
		else if(!strcmp(p, "help"))
		{
			printf(
				"Command\n"
				"quit             | Quit\n"
				"print            | Print all entries\n"
				"get `ident`      | Get value for identifier\n"
				"put `ident` int  | Insert identifier with value\n"
				"remove `ident`   | Remove identifier\n");
		}
		else if(!strcmp(p, "print"))
		{
			symtab_print(tab);
		}
		else if(!strcmp(p, "get"))
		{
			p = strtok(NULL, delim);
			if(p == NULL) { printf("Invalid identifier\n"); continue; }
			int v = symtab_get(tab, p);
			if(v == 0)
			{
				printf("Not found\n");
			}
			else
			{
				printf("%s = %d\n", p, v);
			}
		}
		else if(!strcmp(p, "remove"))
		{
			p = strtok(NULL, delim);
			if(p == NULL) { printf("Invalid identifier\n"); continue; }
			symtab_remove(tab, p);
		}
		else if(!strcmp(p, "put"))
		{
			p = strtok(NULL, delim);
			if(p == NULL) { printf("Invalid identifier\n"); continue; }
			char *q = strtok(NULL, delim);
			if(q == NULL) { printf("Invalid value\n"); continue; }
			int val = atoi(q);
			if(val == 0)
			{
				printf("Invalid value\n");
			}
			else
			{
				symtab_put(tab, p, val);
				printf("%s = %d\n", p, val);
			}
		}
		else
		{
			printf("Unknown command\n");
		}
	}

	symtab_destroy(tab);
}

int main(void)
{
	printf("Starting SymTab Test\n");
	test_put_get();
	//test_complete();
	test_remove();
	test_remove_prefix();
	test_remove_suffix();
	test_prefix_iter();
	test_remove_branch();
	test_remove_prev_branch();
	test_cmdline();

	return 0;
}
