#ifndef _TIMYOS_H
#define _TIMYOS_H

#include <stdint.h>

typedef uint32_t tTaskStack;

typedef struct tTask{

	tTaskStack * stack;
}tTask;

#endif

