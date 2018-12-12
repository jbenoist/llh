#ifndef _LLH_H
#define _LLH_H

#include <stdint.h>

struct llh;

struct llh *llh_new(void);
void llh_free(struct llh *h);
void llh_flush(struct llh *h);
uint64_t llh_cum(struct llh *h);
double llh_quantile(struct llh *h, double q);
void llh_merge(struct llh *dst, struct llh *src);
void llh_observe(struct llh *h, double v, uint64_t n);

#endif /* _LLH_H */
