#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#define SIGNATURE    "God save the Queen!"
#define MESSAGE_MAX  8192
#define AGENTS_MAX   16
#define KEY_LENGTH   32

typedef struct TMessage
{
	unsigned char m_Message[MESSAGE_MAX];
	int m_Length;
}
TMESSAGE;

typedef struct TResults
{
	int m_Agent;
	int m_Shift;
}
TRESULTS;


void
SecretService (int agents, const unsigned char (*keys)[KEY_LENGTH],
	int threads, const TMESSAGE *(*receiver) (void),
	void (*officer) (const TMESSAGE *, TRESULTS *, int));

#endif /* ! __COMMON_H__ */
