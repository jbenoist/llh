#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include <llh.h>
#include "llh-test.h"

#define NROUNDS 1000
#define NTHREADS 16

static struct llh *main_h = NULL;
static volatile uint64_t main_agg_round = 0;
static pthread_rwlock_t GL = PTHREAD_RWLOCK_INITIALIZER;

static void *thread_main(void *ctx)
{
  uint32_t val;
  uint64_t cum = 0;
  unsigned char ok;
  struct llh *local_h = llh_new();
  uint64_t local_agg_round = main_agg_round;
  uint32_t div[] = { 10, 100, 1000, 10000, 100000, 1000000, 10000000,  100000000, 1000000000 };
  do {
    asm volatile ("rdseed %0; setc %1" : "=r" (val), "=qm" (ok));
  } while (!ok);
  for (;;) {
    uint32_t mod = div[(val>>16) % ARRAY_SIZE(div)];
    llh_observe(local_h, mod/10+val%(mod-mod/10), 1);
    cum++;
    if (local_agg_round != main_agg_round || main_agg_round == NROUNDS) {
      pthread_rwlock_wrlock(&GL);
      llh_merge(main_h, local_h);
      pthread_rwlock_unlock(&GL);
      llh_flush(local_h);
      if (main_agg_round == NROUNDS) {
        break;
      }
      local_agg_round = main_agg_round;
    }
    val^= val << 13;
    val^= val >> 17;
    val^= val << 5;
  }
  llh_free(local_h);
  *(uint64_t *)ctx = cum;
  return NULL;
}

int main(void)
{
  char *data[10][10];
  uint64_t total = 0;
  pthread_t t[NTHREADS];
  uint64_t tcum[NTHREADS] = { 0 };
  main_h = llh_new();
  for (int i = 0; i < NTHREADS; i++) {
    pthread_create(&t[i], NULL, thread_main, &tcum[i]);
  }
  while (main_agg_round < NROUNDS) {
    int max[10] = { 0 };
    main_agg_round++;
    usleep(10000);
    pthread_rwlock_rdlock(&GL);
    for (double i = 1.00; i <= 100.00; i += 1.00) {
      asprintf(&(data[(int)i/10][(int)i%10-1]), "%0.2f", llh_quantile(main_h, i / 100));
    }
    pthread_rwlock_unlock(&GL);
    for (int y = 0; y < ARRAY_SIZE(max); y++) {
      for (int x = 0; x < ARRAY_SIZE(max); x++) {
        max[y] = MAX(strlen(data[x][y]), max[y]);
      }
    }
    for (int x = 0; x < ARRAY_SIZE(max); x++) {
      for (int y = 0; y < ARRAY_SIZE(max); y++) {
        printf("%*s ", max[y], data[x][y]);
        free(data[x][y]);
      }
      puts("");
    }
    printf("\033[%dA", 10);
  }
  printf("\033[%dB", 10);
  puts("");
  for (int i = 0; i < NTHREADS; i++) {
    pthread_join(t[i], NULL);
    total += tcum[i];
  }
  assert(llh_cum(main_h) == total);
  printf("total observations:%lu\n", total);
  llh_free(main_h);
}
