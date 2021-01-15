#include <stdint.h>

static        const float        one        = 1.0, tiny=1.0e-30;

typedef union
{
  float value;
  uint32_t word;
} ieee_float_shape_type;

/* Get a 32 bit int from a float.  */
# define GET_FLOAT_WORD(i,d)                                        \
do {                                                                \
  ieee_float_shape_type gf_u;                                        \
  gf_u.value = (d);                                                \
  (i) = gf_u.word;                                                \
} while (0)

/* Set a float from a 32 bit int.  */
# define SET_FLOAT_WORD(d,i)                                        \
do {                                                                \
  ieee_float_shape_type sf_u;                                        \
  sf_u.word = (i);                                                \
  (d) = sf_u.value;                                                \
} while (0)

float __ieee754_sqrtf(float x)
{
        float z;
        int32_t sign = (int)0x80000000;
        int32_t ix,s,q,m,t,i;
        uint32_t r;
        GET_FLOAT_WORD(ix,x);
    /* take care of Inf and NaN */
        if((ix&0x7f800000)==0x7f800000) {
            return x*x+x;                /* sqrt(NaN)=NaN, sqrt(+inf)=+inf
                                           sqrt(-inf)=sNaN */
        }
    /* take care of zero */
        if(ix<=0) {
            if((ix&(~sign))==0) return x;/* sqrt(+-0) = +-0 */
            else if(ix<0)
                return (x-x)/(x-x);                /* sqrt(-ve) = sNaN */
        }
    /* normalize x */
        m = (ix>>23);
        if(m==0) {                                /* subnormal x */
            for(i=0;(ix&0x00800000)==0;i++) ix<<=1;
            m -= i-1;
        }
        m -= 127;        /* unbias exponent */
        ix = (ix&0x007fffff)|0x00800000;
        if(m&1)        /* odd m, double x to make it even */
            ix += ix;
        m >>= 1;        /* m = [m/2] */
    /* generate sqrt(x) bit by bit */
        ix += ix;
        q = s = 0;                /* q = sqrt(x) */
        r = 0x01000000;                /* r = moving bit from right to left */
        while(r!=0) {
            t = s+r;
            if(t<=ix) {
                s    = t+r;
                ix  -= t;
                q   += r;
            }
            ix += ix;
            r>>=1;
        }
    /* use floating add to find out rounding direction */
        if(ix!=0) {
            z = one-tiny; /* trigger inexact flag */
            if (z>=one) {
                z = one+tiny;
                if (z>one)
                    q += 2;
                else
                    q += (q&1);
            }
        }
        ix = (q>>1)+0x3f000000;
        ix += (m <<23);
        SET_FLOAT_WORD(z,ix);
        return z;
}
