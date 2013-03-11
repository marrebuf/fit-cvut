/*
 * BI-PSI assignment #1: TCP
 *
 * vim: set fenc=utf-8 ft=c.doxygen ts=4 sw=4 tw=80 noet:
 *
 * Copyright (c) 2013, Přemysl Janouch <p.janouch@gmail.com>
 * All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#define _POSIX_SOURCE  // getaddrinfo

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <assert.h>
#include <stdbool.h>

#include <regex.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

// ----- Utilities ------------------------------------------------------------

/** Return the number of elements stored in an array. */
#define N_ELEMENTS(a)  (sizeof (a) / sizeof (*(a)))

/** Memory allocation failure handler. */
static void
xalloc_fail (const char *fun, size_t bytes)
{
	fprintf (stderr, "%s: Memory allocation failed for %zu bytes.\n",
		fun, bytes);
	exit (EXIT_FAILURE);
}

/** Safe malloc(). */
static void *xmalloc (size_t)
	__attribute__ ((malloc, alloc_size (1)));

/** Safe calloc(). */
static void *xcalloc (size_t, size_t)
	__attribute__ ((malloc, alloc_size (1, 2)));

/** Safe realloc(). */
static void *xrealloc (void *p, size_t size)
	__attribute__ ((alloc_size (1)));

static void *
xmalloc (size_t size)
{
	void *x = malloc (size);
	if (!x)  xalloc_fail ("malloc", size);
	return x;
}

static void *
xcalloc (size_t n, size_t m)
{
	void *x = calloc (n, m);
	if (!x)  xalloc_fail ("calloc", n * m);
	return x;
}

static void *
xrealloc (void *p, size_t size)
{
	void *x = realloc (p, size);
	if (!x)  xalloc_fail ("realloc", size);
	return x;
}

#define malloc   xmalloc
#define calloc   xcalloc
#define realloc  xrealloc

/** regerror() allocating wrapper. */
static char *
regerror_alloc (int code, const regex_t *preg)
{
	size_t size = regerror (code, preg, NULL, 0);
	char *buf = malloc (size);
	regerror (code, preg, buf, size);
	return buf;
}

/** Change the blocking mode of a file descriptor. */
static bool
set_blocking (int fd, bool blocking)
{
	int flags = fcntl (fd, F_GETFL);
	bool prev = !(flags & O_NONBLOCK);
	if (blocking)
		flags &= ~O_NONBLOCK;
	else
		flags |=  O_NONBLOCK;
	fcntl (fd, F_SETFL, flags);
	return prev;
}

// ----- Sockets wrapper ------------------------------------------------------

typedef struct
{
	int fd;                            //!< Socket.
	FILE *fp;                          //!< FILE wrapper for the FD.

	char *buf;                         //!< Input buffer.
	size_t nbuff_alloc;                //!< Size of the buffer.
	size_t offset;                     //!< Reading offset into the buffer.
}
NetSocket;                             //!< Wraps UNIX sockets.

/** Callback for line reads. */
typedef bool (*NetSocketLineCB) (void *, const char *);

/** Initialize a socket wrapper object. */
static void
net_init (NetSocket *ns, int fd)
{
	assert (ns != NULL);

	ns->fd = fd;
	ns->fp = fdopen (fd, "w+b");

	ns->buf = malloc (ns->nbuff_alloc = 4096);
	ns->offset = 0;
}

/** Destroy a socket wrapper object. */
static void
net_destroy (NetSocket *ns)
{
	assert (ns != NULL);

	free (ns->buf);
	fclose (ns->fp);
	close (ns->fd);
}

/** Read and process lines from the socket. */
static bool
net_read_lines (NetSocket *ns, bool end_stream_result, bool print_fd,
	NetSocketLineCB cb, void *user_data)
{
	assert (ns != NULL);
	assert (cb != NULL);

	ssize_t n_recv;
	while ((n_recv = recv (ns->fd, ns->buf + ns->offset,
		ns->nbuff_alloc - ns->offset, 0)) > 0)
	{
		bool found_nl = false;
		char *p, *start = ns->buf, *end = ns->buf + ns->offset + n_recv;

		// Split and process lines.
		for (p = start; p + 1 < end; p++)
		{
			if (p[0] != '\r' || p[1] != '\n')
				continue;

			*p = 0;
			fputs ("-->", stdout);
			if (print_fd)  printf (" [%d]", ns->fd);
			printf (" \"%s\"\n", start);

			if (!cb (user_data, start))
				return false;

			start = p + 2;
			found_nl = true;
		}

		if (!found_nl)
		{
			ns->offset += n_recv;
			if (ns->offset == ns->nbuff_alloc)
				ns->buf = realloc (ns->buf, ns->nbuff_alloc <<= 1);
		}
		else
			// Save the rest of the input for the next run.
			memmove (ns->buf, start, ns->offset = end - start);
	}

	if (n_recv == -1)
	{
		if (errno == EWOULDBLOCK)
			return true;

		perror ("recv");
		return false;
	}

	return end_stream_result;
}

// ----- Generic --------------------------------------------------------------

#define COORD_UNKNOWN      255         //!< Unknown position.
#define COORD_MIN          -18         //!< Minimum position.
#define COORD_MAX           18         //!< Maximum position.

/** Validate a coordinate value. */
#define COORD_IS_VALID(x)  ((x) >= COORD_MIN && (x) <= COORD_MAX)

#define NL                 "\r\n"      //!< Newline.

typedef enum
{
	DIR_NORTH,                         //!< Decreasing y.
	DIR_WEST,                          //!< Decreasing x.
	DIR_SOUTH,                         //!< Increasing y.
	DIR_EAST,                          //!< Increasing x.
	DIR_UNKNOWN                        //!< Not known yet.
}
Direction;                             //!< Direction of a robot.

// ----- Client ---------------------------------------------------------------

typedef struct
{
	int x;                             //!< Horizontal coordinate.
	int y;                             //!< Vertical coordinate.

	Direction direction;               //!< Direction of the robot.

	NetSocket ns;                      //!< Socket wrapper.
	char *name;                        //!< Server name.

	enum
	{
		CC_S_INTRODUCTION,             //!< Start state, waiting for name.
		CC_S_FIRST_TURN,               //!< Waiting for location.
		CC_S_FIRST_MOVE,               //!< Waiting for location after moving.
		CC_S_GUIDE,                    //!< Guide the robot towards the mark.
		CC_S_PREFINISH,                //!< Waiting for the final message.
		CC_S_FINISHED,                 //!< The final state.
		CC_S_N_STATES                  //!< Number of states.
	}
	state;                             //!< Current state.
	bool repairing;                    //!< Whether we're waiting for a repair.
	bool post_recovery;                //!< Just repaired.
}
ClientCtx;                             //!< Data relevant to a client.

/** Send a command to the server. */
static bool client_send (ClientCtx *ctx, const char *format, ...)
	__attribute__ ((format (printf, 2, 3)));

static bool
client_send (ClientCtx *ctx, const char *format, ...)
{
	bool ret_val = true;
	va_list ap;

	printf ("<-- \"%s ", ctx->name);
	va_start (ap, format);
	vprintf (format, ap);
	va_end (ap);
	fputs ("\"\n", stdout);

	va_start (ap, format);

	if (fputs (ctx->name, ctx->ns.fp) == EOF
	 || fputc (' ', ctx->ns.fp) == EOF
	 || vfprintf (ctx->ns.fp, format, ap) < 0
	 || fputs (NL, ctx->ns.fp) == EOF
	 || fflush (ctx->ns.fp) == EOF)
	{
		fprintf (stderr, "Error: Unsuccessful write.\n");
		ret_val = false;
	}

	va_end (ap);
	return ret_val;
}

static void
client_process_introduction (ClientCtx *ctx, const char *line)
{
	regex_t re;
	int err = regcomp (&re,
		"^210 .*Oslovuj mne ([^ .]([^.]{,510}[^ .])?)\\.", REG_EXTENDED);

	if (err)
	{
		char *msg = regerror_alloc (err, &re);
		fprintf (stderr, "regcomp: %s\n", msg);
		free (msg);
		goto error;
	}

	regmatch_t matches[2];
	if (regexec (&re, line, N_ELEMENTS (matches), matches, 0) == REG_NOMATCH)
	{
		fprintf (stderr, "Error: The server has sent us "
			"an invalid introduction. Ignoring.\n");
		goto error;
	}

	size_t name_len = matches[1].rm_eo - matches[1].rm_so;
	ctx->name = malloc (name_len + 1);
	memcpy (ctx->name, line + matches[1].rm_so, name_len);
	ctx->name[name_len] = 0;

	ctx->state = CC_S_FIRST_TURN;
	client_send (ctx, "VLEVO");

error:
	regfree (&re);
}

/** Handle an expected OK reply. */
static bool
client_read_ok (ClientCtx *ctx, const char *line)
{
	int x, y;
	if (sscanf (line, "240 OK (%d,%d)", &x, &y) != 2
		|| !COORD_IS_VALID (x) || !COORD_IS_VALID (y))
	{
		fprintf (stderr, "Error: Invalid reply.\n");
		return false;
	}

	if (x == 0 && y == 0)
	{
		client_send (ctx, "ZVEDNI");
		ctx->state = CC_S_PREFINISH;
		return false;
	}

	ctx->x = x;
	ctx->y = y;
	return true;
}

/** Handle a potential breakdown. */
static bool
client_read_fail (ClientCtx *ctx, const char *line)
{
	int n;
	if (sscanf (line, "580 SELHANI PROCESORU %d", &n) != 1)
		return false;

	client_send (ctx, "OPRAVIT %d", n);
	ctx->repairing = true;
	return true;
}

static void
client_process_first_turn (ClientCtx *ctx, const char *line)
{
	if (!client_read_ok (ctx, line))
		return;

	ctx->state = CC_S_FIRST_MOVE;
	client_send (ctx, "KROK");
}

/** Make a single step to guide the robot towards the mark. */
static void
client_guide (ClientCtx *ctx)
{
	if ((ctx->x > 0 && ctx->direction != DIR_WEST)
	 || (ctx->x < 0 && ctx->direction != DIR_EAST))
	{
		client_send (ctx, "VLEVO");
		ctx->direction = (ctx->direction + 1) % 4;
		return;
	}

	if (ctx->x != 0)
	{
		client_send (ctx, "KROK");
		return;
	}

	if ((ctx->y > 0 && ctx->direction != DIR_SOUTH)
	 || (ctx->y < 0 && ctx->direction != DIR_NORTH))
	{
		client_send (ctx, "VLEVO");
		ctx->direction = (ctx->direction + 1) % 4;
		return;
	}

	client_send (ctx, "KROK");
}

static void
client_process_first_move (ClientCtx *ctx, const char *line)
{
	if (client_read_fail (ctx, line))
		return;

	int x_orig = ctx->x;
	int y_orig = ctx->y;

	if (!client_read_ok (ctx, line))
		return;
	if (ctx->post_recovery)
	{
		client_send (ctx, "KROK");
		return;
	}

	// Deduce the direction of the bot.
	if      (x_orig < ctx->x)  ctx->direction = DIR_EAST;
	else if (y_orig < ctx->y)  ctx->direction = DIR_NORTH;
	else if (x_orig > ctx->x)  ctx->direction = DIR_WEST;
	else if (y_orig > ctx->y)  ctx->direction = DIR_SOUTH;
	else
	{
		fprintf (stderr, "Error: Robot stands in place.\n");
		return;
	}

	ctx->state = CC_S_GUIDE;
	client_guide (ctx);
}

static void
client_process_guide (ClientCtx *ctx, const char *line)
{
	if (client_read_fail (ctx, line))
		return;
	if (!ctx->post_recovery && !client_read_ok (ctx, line))
		return;
	client_guide (ctx);
}

static void
client_process_prefinish (ClientCtx *ctx, const char *line)
{
	const char s[] = "260 USPECH ";
	if (strncmp (line, s, sizeof s - 1))
	{
		fprintf (stderr, "Error: Invalid reply.\n");
		return;
	}

	printf ("Retrieved secret: '%s'\n", line + sizeof s - 1);
	ctx->state = CC_S_FINISHED;
}

static void
client_process_invalid (ClientCtx *ctx, const char *line)
{
	fprintf (stderr, "Error: Server sends us garbage.\n");
}

/** Process replies coming from the server. */
static bool
client_call_handler (ClientCtx *ctx, const char *line)
{
	static void (*handlers[CC_S_N_STATES]) (ClientCtx *, const char *) =
	{
		client_process_introduction,
		client_process_first_turn,
		client_process_first_move,
		client_process_guide,
		client_process_prefinish,
		client_process_invalid
	};

	if (ctx->repairing)
	{
		ctx->repairing = false;
		if (!client_read_ok (ctx, line))
			return true;
		ctx->post_recovery = true;
	}

	assert (handlers[ctx->state] != NULL);
	handlers[ctx->state] (ctx, line);
	ctx->post_recovery = false;
	return true;
}

/** Retrieve the server's secret. */
static int
client_main (int sockfd)
{
	int ret_val = EXIT_FAILURE;
	ClientCtx ctx;

	ctx.x = COORD_UNKNOWN;
	ctx.y = COORD_UNKNOWN;
	ctx.direction = DIR_UNKNOWN;
	ctx.state = CC_S_INTRODUCTION;
	ctx.repairing = false;
	ctx.post_recovery = false;
	ctx.name = NULL;

	net_init (&ctx.ns, sockfd);
	if (!net_read_lines (&ctx.ns, true, false,
		(NetSocketLineCB) client_call_handler, &ctx))
		goto error;

	if (ctx.state != CC_S_FINISHED)
	{
		fprintf (stderr, "Error: Premature shutdown, secret not retrieved.\n");
		goto error;
	}

	ret_val = EXIT_SUCCESS;

error:
	net_destroy (&ctx.ns);
	free (ctx.name);
	return ret_val;
}

// ----- Server ---------------------------------------------------------------

#define SERVER_NAME  "Emile,"          //!< Name of the server.
#define SERVER_NAME_LEN  6             //!< Length of server name.

/** The secret value stored in the middle of the town. */
#define SERVER_SECRET  "Lyra is best pone."

typedef struct
{
	NetSocket ns;                      //!< Socket wrapper.

	int x;                             //!< Horizontal coordinate.
	int y;                             //!< Vertical coordinate.

	Direction direction;               //!< Direction of the robot.
	int broken_cpu;                    //!< ID of any broken processor.
	int steps;                         //!< Number of steps done.
}
ServerCtx;                             //!< Data relevant to a server.

/** Send a command to the client. */
static bool server_send (ServerCtx *ctx, const char *format, ...)
	__attribute__ ((format (printf, 2, 3)));

static bool
server_send (ServerCtx *ctx, const char *format, ...)
{
	bool ret_val = true;
	va_list ap;

	printf ("<-- [%d] \"", ctx->ns.fd);
	va_start (ap, format);
	vprintf (format, ap);
	va_end (ap);
	fputs ("\"\n", stdout);

	va_start (ap, format);
	set_blocking (ctx->ns.fd, true);

	if (vfprintf (ctx->ns.fp, format, ap) < 0
	 || fputs (NL, ctx->ns.fp) == EOF
	 || fflush (ctx->ns.fp) == EOF)
	{
		fprintf (stderr, "Error: Unsuccessful write.\n");
		ret_val = false;
	}

	va_end (ap);
	set_blocking (ctx->ns.fd, false);
	return ret_val;
}

/** Process a single command coming from a client. */
static bool
server_process (ServerCtx *ctx, const char *line)
{
	if (strncmp (line, SERVER_NAME " ", SERVER_NAME_LEN + 1))
	{
		server_send (ctx, "500 NEZNAMY PRIKAZ");
		return true;
	}

	line += SERVER_NAME_LEN + 1;

	if (!strcmp (line, "KROK"))
	{
		if (ctx->broken_cpu)
		{
			server_send (ctx, "572 ROBOT SE ROZPADL");
			return false;
		}

		if (++ctx->steps == 10)
		{
			ctx->steps = 0;
			server_send (ctx, "580 SELHANI PROCESORU %d",
				ctx->broken_cpu = 1 + rand () % 9);
			return true;
		}

		switch (ctx->direction)
		{
		case DIR_NORTH:  ctx->y++; break;
		case DIR_EAST:   ctx->x++; break;
		case DIR_SOUTH:  ctx->y--; break;
		case DIR_WEST:   ctx->x--; break;
		default:         abort ();
		}

		if (!COORD_IS_VALID (ctx->x) || !COORD_IS_VALID (ctx->y))
		{
			server_send (ctx, "530 HAVARIE");
			return false;
		}

		server_send (ctx, "240 OK (%d,%d)", ctx->x, ctx->y);
		return true;
	}

	if (!strcmp (line, "VLEVO"))
	{
		ctx->direction = (ctx->direction + 1) % 4;
		server_send (ctx, "240 OK (%d,%d)", ctx->x, ctx->y);
		return true;
	}

	if (!strcmp (line, "ZVEDNI"))
	{
		if (ctx->x == 0 && ctx->y == 0)
			server_send (ctx, "260 USPECH " SERVER_SECRET);
		else
			server_send (ctx, "550 NELZE ZVEDNOUT ZNACKU");
		return false;
	}

	unsigned cpu_to_repair;
	if (sscanf (line, "OPRAVIT %ud", &cpu_to_repair) == 1
		&& cpu_to_repair != 0 && cpu_to_repair <= 9)
	{
		if (cpu_to_repair != ctx->broken_cpu)
		{
			server_send (ctx, "571 PROCESOR FUNGUJE");
			return false;
		}

		ctx->broken_cpu = 0;
		server_send (ctx, "240 OK (%d,%d)", ctx->x, ctx->y);
		return true;
	}

	server_send (ctx, "500 NEZNAMY PRIKAZ");
	return true;
}

/** Process incoming requests from clients. */
static int
server_main (int sockfd)
{
	struct pollfd *fds;
	nfds_t nfds = 1, nfds_alloc = 4;

	fds = calloc (sizeof *fds, nfds_alloc);
	fds[0].fd = sockfd;
	fds[0].events = POLLIN;

	ServerCtx *ctxs;
	size_t nctxs = 0, nctxs_alloc = 4;

	ctxs = calloc (sizeof *ctxs, nctxs_alloc);

	set_blocking (sockfd, false);
	while (poll (fds, nfds, -1) >= 0)
	{
		if (fds[0].revents & POLLIN)
		{
			int new_client = accept (sockfd, NULL, NULL);
			if (new_client == -1)
			{
				perror ("accept");
				continue;
			}

			if (nctxs == nctxs_alloc)
				ctxs = realloc (ctxs, sizeof *ctxs * (nctxs_alloc <<= 1));
			if (nfds == nfds_alloc)
				fds = realloc (fds, sizeof *fds * (nfds_alloc <<= 1));

			struct pollfd *new_fd = &fds[nfds++];
			new_fd->events = POLLIN;
			new_fd->revents = 0;
			new_fd->fd = new_client;

			ServerCtx *new_ctx = &ctxs[nctxs++];
			net_init (&new_ctx->ns, new_client);

			new_ctx->x = COORD_MIN + 1 + rand () % (COORD_MAX - COORD_MIN - 2);
			new_ctx->y = COORD_MIN + 1 + rand () % (COORD_MAX - COORD_MIN - 2);
			new_ctx->direction = rand () % 4;
			new_ctx->broken_cpu = 0;
			new_ctx->steps = 0;

			server_send (new_ctx, "210 Nazdar. Oslovuj mne " SERVER_NAME ".");
		}

		int i;
		for (i = 1; i < nfds; i++)
		{
			if (fds[i].revents & POLLIN
			 && !net_read_lines (&ctxs[i - 1].ns, false, true,
					(NetSocketLineCB) server_process, &ctxs[i - 1]))
			{
				fds[i--] = fds[--nfds];
				net_destroy (&ctxs[i].ns);
				ctxs[i] = ctxs[--nctxs];
			}
		}
	}

	perror ("poll");
	return EXIT_FAILURE;
}

// ----- Main program ---------------------------------------------------------

/** Bind to the specified port and run the server. */
static int
run_server (const char *port)
{
	struct addrinfo gai_hints = {0}, *gai_result, *gai_iter;

	// We definitely want TCP, IPv4.
	gai_hints.ai_family = AF_INET;
	gai_hints.ai_socktype = SOCK_STREAM;
	gai_hints.ai_flags = AI_PASSIVE;

	int err = getaddrinfo (NULL, port, &gai_hints, &gai_result);
	if (err)
	{
		fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (err));
		return EXIT_FAILURE;
	}

	int sockfd;
	for (gai_iter = gai_result; gai_iter; gai_iter = gai_iter->ai_next)
	{
		sockfd = socket (gai_iter->ai_family,
			gai_iter->ai_socktype, gai_iter->ai_protocol);
		if (sockfd == -1)
			continue;

		int flag = true;
		setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof flag);

		if (!bind (sockfd, gai_iter->ai_addr, gai_iter->ai_addrlen))
			break;

		close (sockfd);
	}

	freeaddrinfo (gai_result);

	if (!gai_iter)
	{
		fprintf (stderr, "Error: Failed to bind to port %s.\n", port);
		return EXIT_FAILURE;
	}

	if (listen (sockfd, 10) == -1)
	{
		perror ("listen");
		return EXIT_FAILURE;
	}

	return server_main (sockfd);
}

/** Connect to the server and run the client. */
static int
run_client (const char *host, const char *port)
{
	struct addrinfo gai_hints = {0}, *gai_result, *gai_iter;

	// We definitely want TCP.
	gai_hints.ai_socktype = SOCK_STREAM;

	int err = getaddrinfo (host, port, &gai_hints, &gai_result);
	if (err)
	{
		fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (err));
		return EXIT_FAILURE;
	}

	int sockfd;
	for (gai_iter = gai_result; gai_iter; gai_iter = gai_iter->ai_next)
	{
		sockfd = socket (gai_iter->ai_family,
			gai_iter->ai_socktype, gai_iter->ai_protocol);
		if (sockfd == -1)
			continue;

		if (!connect (sockfd, gai_iter->ai_addr, gai_iter->ai_addrlen))
			break;

		close (sockfd);
	}

	freeaddrinfo (gai_result);

	if (!gai_iter)
	{
		fprintf (stderr, "Error: Failed to connect to %s:%s.\n",
			host, port);
		return EXIT_FAILURE;
	}

	return client_main (sockfd);
}

/** Program entry point. */
int
main (int argc, char *argv[])
{
	switch (argc)
	{
	case 2:
		return run_server (argv[1]);
	case 3:
		return run_client (argv[1], argv[2]);
	default:
		fprintf (stderr, "Usage: %s [<server>] <port>\n", argv[0]);
		return EXIT_FAILURE;
	}
}

