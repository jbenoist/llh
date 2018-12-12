#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "llh.h"

static const double power_of_ten[128] = {
  1e+0,  1e+1,  1e+2,  1e+3,  1e+4,  1e+05, 1e+06, 1e+07,
  1e+08, 1e+09, 1e+10, 1e+11, 1e+12, 1e+13, 1e+14, 1e+15,
  1e+16, 1e+17, 1e+18, 1e+19, 1e+20, 1e+21, 1e+22, 1e+23,
  1e+24, 1e+25, 1e+26, 1e+27, 1e+28, 1e+29, 1e+30, 1e+31,
  1e+32, 1e+33, 1e+34, 1e+35, 1e+36, 1e+37, 1e+38, 1e+39,
  1e+40, 1e+41, 1e+42, 1e+43, 1e+44, 1e+45, 1e+46, 1e+47,
  1e+48, 1e+49, 1e+50, 1e+51, 1e+52, 1e+53, 1e+54, 1e+55,
  1e+56, 1e+57, 1e+58, 1e+59, 1e+60, 1e+61, 1e+62, 1e+63,
  1e+64, 1e+65, 1e+66, 1e+67, 1e+68, 1e+69, 1e+70, 1e+71,
  1e+72, 1e+73, 1e+74, 1e+75, 1e+76, 1e+77, 1e+78, 1e+79,
  1e+80, 1e+81, 1e+82, 1e+83, 1e+84, 1e+85, 1e+86, 1e+87,
  1e+88, 1e+89, 1e+90, 1e+91, 1e+92, 1e+93, 1e+94, 1e+95,
  1e+96, 1e+97, 1e+98, 1e+99, 1e+100,1e+101,1e+102,1e+103,
  1e+104,1e+105,1e+106,1e+107,1e+108,1e+109,1e+110,1e+111,
  1e+112,1e+113,1e+114,1e+115,1e+116,1e+117,1e+118,1e+119,
  1e+120,1e+121,1e+122,1e+123,1e+124,1e+125,1e+126,1e+127,
};

struct llh {
  int bin_size;
  uint64_t *bins;
};

struct llh *llh_new(void)
{
  return calloc(1, sizeof(struct llh));
}

void llh_free(struct llh *h)
{
  free(h->bins);
  free(h);
}

void llh_flush(struct llh *h)
{
  bzero(h->bins, sizeof(*h->bins) * h->bin_size);
}

void llh_merge(struct llh *dst, struct llh *src)
{
  if (dst->bin_size < src->bin_size) {
    dst->bins = realloc(dst->bins, src->bin_size * sizeof(*dst->bins));
    bzero(dst->bins + dst->bin_size, sizeof(*dst->bins) * (src->bin_size-dst->bin_size));
    dst->bin_size = src->bin_size;
  }
  for (int i = 0; i < src->bin_size; i++) {
    dst->bins[i] += src->bins[i];
  }
}

uint64_t llh_cum(struct llh *h)
{
  uint64_t count = 0;
  for (int i = 0; i < h->bin_size; i++) {
    count += h->bins[i];
  }
  return count;
}

static int val_to_bin(double v)
{
  int idx = 0;
  if (v < 1.0) {
    idx = v/(1.0/90.00);
  } else {
    int exp = (int)floor(log10(v));
    idx = 90 * (1+exp) + (v/power_of_ten[exp]*10) - 10;
  }
  return idx;
}

static double bin_to_val(int idx)
{
  if (idx < 90) {
    return (double)1.0/90.0*(double)idx;
  }
  double p10 = power_of_ten[(idx/90)-1];
  return p10 + p10 * ((double)(idx%90)/10.0);
}

void llh_observe(struct llh *h, double v, uint64_t n)
{
  int idx = val_to_bin(v);
  if (idx >= h->bin_size) {
    h->bins = realloc(h->bins, (idx+1) * sizeof(*h->bins));
    bzero(h->bins + h->bin_size, sizeof(*h->bins) * (idx+1-h->bin_size));
    h->bin_size = idx+1;
  }
  h->bins[idx] += n;
}

double llh_quantile(struct llh *h, double q)
{
  double lo = 0.0, hi = 0.0;
  q *= llh_cum(h);
  for (int i = 0; i < h->bin_size; i++) {
    hi += h->bins[i];
    if (q > lo && q <= hi) {
      double l = bin_to_val(i);
      double r = bin_to_val(i+1);
      return l + (q-lo)/(hi-lo)*(r-l);
    }
    lo = hi;
  }
  return 0.00;
}
