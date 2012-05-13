#ifndef __PROGTEST__
#include "common_ssvc.h"
#endif /* __PROGTEST__ */


#define SHIFT_SIZE 64

/* Unnnnh! */
typedef const unsigned char Key[KEY_LENGTH];
typedef const unsigned char (*KeyArray)[KEY_LENGTH];

typedef struct WorkUnit WorkUnit;
typedef struct MsgCtx MsgCtx;
typedef struct SecretSvcCtx SecretSvcCtx;

/** A single unit of decryption work. */
struct WorkUnit
{
	MsgCtx *msg_ctx;
	unsigned shift;
	unsigned key;

	WorkUnit *next;
};

/** To queue up results. */
struct MsgCtx
{
	const TMESSAGE *msg_in;
	unsigned units_remaining;

	TRESULTS *res;
	int n_res, res_size;
};

/** Secret service context structure. */
struct SecretSvcCtx
{
	pthread_mutex_t mtx;
	pthread_mutex_t msg_mtx;
	pthread_cond_t cond;
	bool finishing;
	WorkUnit *units;

	KeyArray keys;
	int n_keys;
	void (*officer) (const TMESSAGE *, TRESULTS *, int);
};

/* ===== Message processing procedures ==================================== */

static bool
message_run (Key key, int shift, const TMESSAGE *in)
{
	int i, msg_i, key_i;
	int prev = 0;
	unsigned char out[MESSAGE_MAX];

	msg_i = shift;
	key_i = 0;
	for (i = 0; i < in->m_Length; i++)
	{
		prev = i ^ key[key_i] ^ prev;
		out[i] = in->m_Message[msg_i] ^ prev;
		if (++msg_i == in->m_Length)
			msg_i = 0;
		if (++key_i == KEY_LENGTH)
			key_i = 0;
	}

	int n = strlen (SIGNATURE);

	if (in->m_Length < n)
		return false;
	return !strncmp (SIGNATURE, (const char *)
		(const char *) out + (in->m_Length - n), n);
}

/* ===== Secret service =================================================== */

/** Decryption worker. */
static void *
worker (void *param)
{
	SecretSvcCtx *ctx = (SecretSvcCtx *) param;

	while (true)
	{

		pthread_mutex_lock (&ctx->mtx);
		while (!ctx->units)
		{
			if (ctx->finishing)
			{
				pthread_mutex_unlock (&ctx->mtx);
				pthread_exit (NULL);
			}

			pthread_cond_wait (&ctx->cond, &ctx->mtx);
		}

		/* Unlink a unit. */
		WorkUnit unit = *ctx->units;
		free (ctx->units);
		ctx->units = unit.next;
		pthread_mutex_unlock (&ctx->mtx);

		/* Compute the unit. */
		MsgCtx *msg_ctx = unit.msg_ctx;
		const TMESSAGE *msg = msg_ctx->msg_in;

		int max = unit.shift + SHIFT_SIZE;
		if (max > msg->m_Length)
			max = msg->m_Length;

		for (int sh = unit.shift; sh < max; sh++)
		{
			/* If the run is successful, append the result. */
			if (message_run (ctx->keys[unit.key], sh, msg))
			{
				pthread_mutex_lock (&ctx->msg_mtx);

				if (msg_ctx->n_res == msg_ctx->res_size)
					msg_ctx->res = (TRESULTS *)
						realloc (msg_ctx->res, sizeof *msg_ctx->res
							* (msg_ctx->res_size <<= 1));

				msg_ctx->res[msg_ctx->n_res].m_Agent = unit.key;
				msg_ctx->res[msg_ctx->n_res].m_Shift = sh;
				msg_ctx->n_res++;

				pthread_mutex_unlock (&ctx->msg_mtx);
			}
		}

		pthread_mutex_lock (&ctx->msg_mtx);
		if (!--unit.msg_ctx->units_remaining)
		{
			pthread_mutex_unlock (&ctx->msg_mtx);

			/* Send results to the officer. */
			ctx->officer (msg_ctx->msg_in, msg_ctx->res, msg_ctx->n_res);
			free (msg_ctx->res);
			free (msg_ctx);
		}
		else
			pthread_mutex_unlock (&ctx->msg_mtx);
	}

	return NULL;
}

/** @param[in] agents  Count of active agents, also number of active keys.
 *  @param[in] keys  Array of keys used by individual agents.
 *  @param[in] threads  The number of threads to use for decrypting.
 *  @param[in] receiver  Callback for retrieving encrypted messages.
 *                       NULL for none left.
 *  @param[in] officer  Callback for delivering decrypted messages.
 */
void
SecretService (int agents, KeyArray keys,
	int threads, const TMESSAGE *(*receiver) (void),
	void (*officer) (const TMESSAGE *, TRESULTS *, int))
{
	SecretSvcCtx ctx;
	pthread_attr_t attr;
	pthread_t *th;
	int i, k;

	/* Initialize the context. */
	pthread_mutex_init (&ctx.mtx, NULL);
	pthread_mutex_init (&ctx.msg_mtx, NULL);
	pthread_cond_init (&ctx.cond, NULL);
	ctx.finishing = false;
	ctx.units = NULL;

	ctx.keys = keys;
	ctx.n_keys = agents;
	ctx.officer = officer;

	/* Spawn workers. */
	pthread_attr_init (&attr);
	pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_JOINABLE);

	th = (pthread_t *) malloc (sizeof *th * threads);
	for (i = 0; i < threads; i++)
		pthread_create (&th[i], &attr, worker, &ctx);

	pthread_attr_destroy (&attr);

	/* Divide and conquer. */
	const TMESSAGE *msg;
	while ((msg = receiver ()))
	{
		pthread_mutex_lock (&ctx.mtx);

		/* Create an entry for the message. */
		MsgCtx *msg_ctx = (MsgCtx *) malloc (sizeof *msg_ctx);
		msg_ctx->msg_in = msg;
		msg_ctx->units_remaining = 0;
		msg_ctx->n_res = 0;
		msg_ctx->res = (TRESULTS *)
			malloc (sizeof (TRESULTS) * (msg_ctx->res_size = 2));

		/* Split work into units. */
		for (k = ctx.n_keys; k--; )
		for (i = (msg->m_Length + SHIFT_SIZE - 1) / SHIFT_SIZE; i--; )
		{
			WorkUnit *unit = (WorkUnit *) malloc (sizeof *ctx.units);
			unit->msg_ctx = msg_ctx;
			unit->shift = i * SHIFT_SIZE;
			unit->key = k;
			unit->next = ctx.units;
			ctx.units = unit;
			msg_ctx->units_remaining++;
		}

		pthread_cond_signal (&ctx.cond);
		pthread_mutex_unlock (&ctx.mtx);
	}

	pthread_mutex_lock (&ctx.mtx);
	ctx.finishing = true;
	pthread_cond_broadcast (&ctx.cond);
	pthread_mutex_unlock (&ctx.mtx);

	for (i = 0; i < threads; i++)
		pthread_join (th[i], NULL);

	/* Clean up resources. */
	free (th);
	pthread_mutex_destroy (&ctx.mtx);
	pthread_mutex_destroy (&ctx.msg_mtx);
	pthread_cond_destroy (&ctx.cond);
}

