
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

struct entry {
   int data;
   struct entry *prev;
   struct entry *next;
};

typedef struct entry head;
typedef struct entry entry;

void init_head(head *h)
{
   h->next = h;
   h->prev = h;
}

void init_entry(entry *e, int data)
{
   e->data = data;
   e->next = NULL;
   e->prev = NULL;
}

entry* alloc_entry(int data)
{
   entry *e = malloc(sizeof(*e));
   if (e != NULL) {
      init_entry(e, data);
   }
   return e;
}

int is_empty(head *h)
{
   return (h->next == h && h->prev == h);
}

void add_at_first(head *h, entry *e)
{
   e->next = h->next;
   e->prev = h;
   h->next->prev = e;
   h->next = e;
}

void add_at_last(head *h, entry *e)
{
   e->prev = h->prev;
   e->next = h;
   h->prev->next = e;
   h->prev = e;
}

int remove_first(head *h)
{
   entry *e = h->next;
   int data = e->data;

   e->next->prev = h;
   h->next = e->next;
   free(e);
   return data;
}

int remove_last(head *h)
{
   entry *e = h->prev;
   int data = e->data;

   e->prev->next = h;
   h->prev = e->prev;
   free(e);
   return data;
}

int list_remove(entry *e)
{
   int data = e->data;
   e->next->prev = e->prev;
   e->prev->next = e->next;
   free(e);
   return data;
}

void print_list_reverse(head *h)
{
   entry *e = h->prev;
   printf("Contents (Reverse): ");
   while (e != h) {
      printf("%d -> ", e->data);
      e = e->prev;
   }
   printf("\n");
}

void print_list(head *h)
{
   entry *e = h->next;
   printf("Contents (Forward): ");
   while (e != h) {
      printf("%d -> ", e->data);
      e = e->next;
   }
   printf("\n");
   print_list_reverse(h);
}

void list_add_first_test(head *h, int data)
{
   entry *e = alloc_entry(data);
   add_at_first(h, e);
   print_list(h);
}

void list_add_last_test(head *h, int data)
{
   entry *e = alloc_entry(data);
   add_at_last(h, e);
   print_list(h);
}

void list_remove_fist_test(head *h, int expected)
{
   int data = remove_first(h);
   assert(data == expected);
   print_list(h);
}

void list_remove_last_test(head *h, int expected)
{
   int data = remove_last(h);
   assert(data == expected);
   print_list(h);
}

void run_unit_tests(head *h)
{
   init_head(h);
   print_list(h);
   list_add_first_test(h, 10);
   list_add_last_test(h, 20);
   list_add_first_test(h, 30);
   list_add_last_test(h, 40);
   list_add_first_test(h, 50);
   list_add_last_test(h, 60);
   list_remove_fist_test(h, 50);
   list_remove_last_test(h, 60);
   list_remove(h->next->next);
   print_list(h);
}

int main()
{
   head h;
   run_unit_tests(&h);
   return 0;
}