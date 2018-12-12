#include <stdio.h>
#include <stdlib.h>

#include <llh.h>
#include "llh-test.h"

static void test(double *array, int array_size)
{
  struct llh *h = llh_new();
  if (array_size != -1) {
    for (int i = 0; i < array_size; i++) {
      llh_observe(h, array[i], 1);
    }
  } else {
    for (double i = 1; i < array[0]; i++) {
      llh_observe(h, i, 1);
    }
  }
  printf("Sample {");
  if (array_size == -1) {
    printf(" 1...%g", array[0]-1.0);
  } else {
    for (int i = 0; i < array_size; i++) {
      printf("%s%g", i > 0 ? ", " : " ", array[i]);
    }
  }
  printf(" } (quantiles=1s)\n");
  for (double i = 1.00; i <= 100.00; i+= 1.00) {
    printf("%10g", llh_quantile(h, i/100.00));
    if ((int)i > 0 && (int)i % 10 == 0) {
      puts("");
    }
  }
  puts("");
  llh_free(h);
}

static int is_prime(uint64_t n)
{
  if (n <= 3) {
    return n > 1;
  } else if (n%2 == 0 || n%3 == 0) {
    return 0;
  }
  for (uint64_t i = 5; i*i <= n; i += 6) {
    if (n%i == 0 || n%(i+2) == 0) {
      return 0;
    }
  }
  return 1;
}

static void test_prime(uint64_t max)
{
  int x = 0;
  struct llh *h = llh_new();
  for (uint64_t i = 1; i < max; i += 2) {
    if (is_prime(i)) {
      llh_observe(h, (double)i, 1);
    }
  }
  printf("Sample { Prime-%lu } (quantiles=0.1s)\n", max);
  for (double i = 0.00; i <= 100.00; i += 0.10, x++) {
    printf("%10g", llh_quantile(h, i/100));
    if (x > 0 && (x+1) % 10 == 0) {
      puts("");
    }
  }
  puts("");
  llh_free(h);
}

static double fib_memo[101] = { 0 };
static double fib(struct llh *h, double n)
{
  double r;
  if (n <= 1) {
    return n;
  }
  r = fib_memo[(int)n];
  if (r == 0) {
    r = fib_memo[(int)n] = fib(h, n-1) + fib(h, n-2);
    llh_observe(h, r, 1);
  }
  return r;
}

static void test_fib(void)
{
  struct llh *h = llh_new();
  const double fibseq[100] = {
    1.0, 1.0, 2.0, 3.0, 5.0, 8.0, 13.0, 21.0, 34.0, 55.0, 89.0, 144.0, 233.0,
    377.0, 610.0, 987.0, 1597.0, 2584.0, 4181.0, 6765.0, 10946.0, 17711.0,
    28657.0, 46368.0, 75025.0, 121393.0, 196418.0, 317811.0, 514229.0,
    832040.0, 1346269.0, 2178309.0, 3524578.0, 5702887.0, 9227465.0,
    14930352.0, 24157817.0, 39088169.0, 63245986.0, 102334155.0, 165580141.0,
    267914296.0, 433494437.0, 701408733.0, 1134903170.0, 1836311903.0,
    2971215073.0, 4807526976.0, 7778742049.0, 12586269025.0, 20365011074.0,
    32951280099.0, 53316291173.0, 86267571272.0, 139583862445.0,
    225851433717.0, 365435296162.0, 591286729879.0, 956722026041.0,
    1548008755920.0, 2504730781961.0, 4052739537881.0, 6557470319842.0,
    10610209857723.0, 17167680177565.0, 27777890035288.0, 44945570212853.0,
    72723460248141.0, 117669030460994.0, 190392490709135.0, 308061521170129.0,
    498454011879264.0, 806515533049393.0, 1304969544928657.0,
    2111485077978050.0, 3416454622906707.0, 5527939700884757.0,
    8944394323791464.0, 14472334024676221.0, 23416728348467685.0,
    37889062373143906.0, 61305790721611591.0, 99194853094755497.0,
    160500643816367088.0, 259695496911122585.0, 420196140727489673.0,
    679891637638612258.0, 1100087778366101931.0, 1779979416004714189.0,
    2880067194370816120.0, 4660046610375530309.0, 7540113804746346429.0,
    12200160415121876738.0, 19740274219868223167.0, 31940434634990099905.0,
    51680708854858323072.0, 83621143489848422977.0, 135301852344706746049.0,
    218922995834555169026.0, 354224848179261915075.0
  };
  fib(h, ARRAY_SIZE(fibseq));
  puts("Sample { Fib-100 } (quantile-est-error:%)");
  for (double i = 0.00; i < 100.00; i+= 1.00) {
    double fib = fibseq[(int)i];
    double nth = llh_quantile(h, i/100);
    printf("%8.2f%%", 100-fib/nth*100.00);
    if ((int)i+1 > 0 && (int)(i+1) % 10 == 0) {
      puts("");
    }
  }
  llh_free(h);
}

int main(void)
{
  test((double []){ 10 }, -1);
  test((double []){ 100 }, -1);
  test((double []){ 1000 }, -1);
  test((double []){ 10000 }, -1);
  test((double []){ 1000000 }, -1);
  test((double []){ 20 }, -1 );
  test((double []){ 300 }, -1 );
  test((double []){ 4000 }, -1 );
  test((double []){ 50000 }, -1 );
  test((double []){ 6000000 }, -1 );
  test((double []){ 10, 99999, 77, 41 }, 4);
  test((double []){ 1, 10, 500, 8000, 15000 }, 5);
  test((double []){ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 }, 10);
  test((double []){ 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1 }, 10);
  test((double []){ 1000000000, 900000000, 80000000, 7000000, 600000, 50000, 4000, 300, 20, 1 }, 10);
  for (uint64_t limit = 10; limit <= 1000000; limit *= 10) {
    test_prime(limit);
  }
  test_fib();
  return 0;
}
