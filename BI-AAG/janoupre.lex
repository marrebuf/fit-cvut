%{
enum {END, WS, NUMBER, FLOAT, PLUS, ANY};
%}

ws       [ \t\n]+
number   [0-9]+
float    [0-9]+[.,][0-9]+
plus     [+]
any      [^ \t\n]+
end      ([ \t\n]+|$)

%%

{ws}           { return WS; }
{number}/{end} { return NUMBER; }
{float}/{end}  { return FLOAT; }
{plus}/{end}   { return PLUS; }
{any}          { return ANY; }

%%

struct link
{
	int type;
	char *text;

	struct link *next;
	struct link *prev;
};

struct deque
{
	struct link *head;
	struct link *tail;
};

static int
deque_read_token (struct deque *q)
{
	int type = yylex ();
	if (!type)
		return END;

	struct link *l = malloc (sizeof *l);
	l->type = type;
	l->text = strdup (yytext);
	l->next = NULL;
	l->prev = q->tail;

	if (q->head)
		q->tail = q->tail->next = l;
	else
		q->head = q->tail = l;
	return l->type;
}

static struct link *
deque_remove_head (struct deque *q)
{
	if (!q->head)  return NULL;
	struct link *l = q->head;

	q->head = l->next;
	if (q->head)
		q->head->prev = NULL;
	else
		q->tail = NULL;

	l->next = NULL;
	return l;
}

static struct link *
deque_remove_tail (struct deque *q)
{
	if (!q->tail)  return NULL;
	struct link *l = q->tail;

	q->tail = l->prev;
	if (q->tail)
		q->tail->next = NULL;
	else
		q->head = NULL;

	l->prev = NULL;
	return l;
}

static void
list_discard (struct link *list, int print)
{
	while (list)
	{
		if (print)
			printf ("%s", list->text);

		struct link *next = list->next;
		free (list->text);
		free (list);
		list = next;
	}
}

static void
deque_discard (struct deque *q, int print)
{
	list_discard (q->head, print);
	q->head = q->tail = NULL;
}

static void
try_add (struct deque *q)
{
	int number = atoi (q->head->text);
	deque_discard (q, 0);

	if (deque_read_token (q) != WS)
	{
		printf ("%d", number);
		return;
	}

	struct link *ws = deque_remove_head (q);

	while (deque_read_token (q) == PLUS
	    && deque_read_token (q) == WS
	    && deque_read_token (q) == NUMBER)
	{
		int add = atoi (q->tail->text);

		int next = deque_read_token (q);
		if (next == WS || next == END)
		{
			list_discard (ws, 0);
			if (next == WS)
				ws = deque_remove_tail (q);
			else
				ws = NULL;

			number += add;
			deque_discard (q, 0);
		}
		else
			break;
	}

	printf ("%d", number);
	list_discard (ws, 1);
}

int
main (int argc, char *argv[])
{
	struct deque q = {NULL, NULL};

	while (1)
	{
		if (!q.head)
			deque_read_token (&q);
		if (!q.head || q.head->type == END)
			break;

		if (q.head->type == NUMBER)
			try_add (&q);
		else
			list_discard (deque_remove_head (&q), 1);
	}

	deque_discard (&q, 1);
	return 0;
}

