#include "util.h"

HybridStack*
hybrid_stack_create()
{
	HybridStack *stack;

	stack = g_new0(HybridStack, 1);

	return stack;
}

void
hybrid_stack_push(HybridStack *stack, gpointer data)
{
	HybridStackNode *node;

	g_return_if_fail(stack != NULL);

	node = g_new0(HybridStackNode, 1);
	node->data = data;
	node->next = stack->head;

	stack->head = node;
}

gpointer
hybrid_stack_pop(HybridStack *stack)
{
	HybridStackNode *node;
	gpointer data;

	g_return_val_if_fail(stack != NULL, NULL);

	if (!stack->head) {

		hybrid_debug_info("stack", "stack empty");

		return NULL;
	}

	node = stack->head;
	data = node->data;

	stack->head = node->next;

	g_free(node);

	return data;
}
