#include "beebScreen/beebScreen.h"
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "beebScreen/bagiCode.c"

int palette[256];
int USE_NULA = 0;

#define BALLS 4
#define COLOURS 5

int c1[7];
int remap[8];
float c[BALLS][3];
float r[BALLS];
float q[BALLS];
float v[BALLS];

unsigned char buffer[128*192];

void flipCallback()
{
    if (USE_NULA)
    {
        beebScreen_SendPal(palette,16);
    }

}

void syntax()
{
    printf("Syntax: rt [-n]\n");

    exit(1);
}

void init()
{
    c1[0] = 1;
    c1[1] = 6;
    c1[2] = 0;
    c1[3] = 7;
    c1[4] = 5;
    c1[5] = 8;
    c1[6] = 15;

    v[0]=v[1]=v[2]=v[3]=0.0;

    c[0][0] = -0.8;
    c[0][1] = -1;
    c[0][2] = 3.2;
    r[0] = 0.7;
    q[0] = 0.7 * 0.7;

    c[1][0] = 0;
    c[1][1] = -0.45;
    c[1][2] = 2;
    r[1] = 0.3;
    q[1] = 0.3 * 0.3;

    c[2][0] = 1.2;
    c[2][1] = -0.7;
    c[2][2] = 2.5;
    r[2] = 0.5;
    q[2] = 0.5 * 0.5;

    c[3][0] = 0.4;
    c[3][1] = -1;
    c[3][2] = 4;
    r[3] = 0.4;
    q[3] = 0.4 * 0.4;
}
	      
void plot(int x, int y, int col)
{
    int localx = x / 10;
    int localy = y / 5;
    buffer[(localy * 128) + localx] = col;
}

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

float SQRT(float N)
{
#if 0
  return sqrtf(N);
#else
  return __ieee754_sqrtf(N);
#endif
}

void setupPalette()
{
    palette[0]=beebScreen_MakeNulaPal(0xff0000,0,beebScreen_extractRGB888);
    palette[1]=beebScreen_MakeNulaPal(0x000000,1,beebScreen_extractRGB888);
    palette[2]=beebScreen_MakeNulaPal(0x0000ff,2,beebScreen_extractRGB888);
    palette[3]=beebScreen_MakeNulaPal(0xff00ff,3,beebScreen_extractRGB888);
    palette[4]=beebScreen_MakeNulaPal(0x00ff00,4,beebScreen_extractRGB888);
    palette[5]=beebScreen_MakeNulaPal(0xffff00,5,beebScreen_extractRGB888);
    palette[6]=beebScreen_MakeNulaPal(0x00ffff,6,beebScreen_extractRGB888);
    palette[7]=beebScreen_MakeNulaPal(0xffffff,7,beebScreen_extractRGB888);

    palette[8]=beebScreen_MakeNulaPal(0x7f0000,8,beebScreen_extractRGB888);
    palette[9]=beebScreen_MakeNulaPal(0x000000,9,beebScreen_extractRGB888);
    palette[10]=beebScreen_MakeNulaPal(0x00007f,10,beebScreen_extractRGB888);
    palette[11]=beebScreen_MakeNulaPal(0x7f007f,11,beebScreen_extractRGB888);
    palette[12]=beebScreen_MakeNulaPal(0x007f00,12,beebScreen_extractRGB888);
    palette[13]=beebScreen_MakeNulaPal(0x7f7f00,13,beebScreen_extractRGB888);
    palette[14]=beebScreen_MakeNulaPal(0x007f7f,14,beebScreen_extractRGB888);
    palette[15]=beebScreen_MakeNulaPal(0x7f7f7f,15,beebScreen_extractRGB888);

    remap[0] = 1;
    remap[1] = 2;
    remap[2] = 4;
    remap[3] = 6;
    remap[4] = 0;
    remap[5] = 3;
    remap[6] = 5;
    remap[7] = 7;
}


int main(int argc, char *argv[])
{
    int index = -1;
    int audio = 1;
    int stream = 0;
    int usePic = 0;
    unsigned char *pic;
    int picIndex = 0;
    int picSize = 64;

    for(int i=1;i<argc; ++i)
    {
        if (argv[i][0]=='-')
        {
            switch(argv[i][1])
            {
            case 'n':
            case 'N':
                USE_NULA= 1;
                break;
            case 's':
            case 'S':
                picSize=atoi(argv[++i]);
                break;
            }
        }
        else if (!usePic)
        {
            usePic = 1;
            picIndex = i;
        }
        else
        {
            syntax();
        }   
    }

    float qpicSize =(float)picSize/4.0;
    pic = malloc(picSize * picSize);

    printf("Real-time raytracing demo.\n\nMODE 2 128x192. ");
    if (USE_NULA)
    {
        printf("Using NULA.\n");
    }
    else
    {
        printf("Using Beeb Palette.\n");
    }
    if (usePic)
    {
        printf("Loading picture: %s (%dx%d)\n",argv[picIndex],picSize,picSize);
        FILE *fhand=fopen(argv[picIndex],"rb");
        if (fhand)
        {
            fread(pic,picSize,picSize,fhand);
            fclose(fhand);
        }
        else
        {
            usePic = 0;
        }
    }
    printf("\nPress any key to start.\n");

    int ch;

    _swi(OS_ReadC,_OUT(0),&ch);

    if (USE_NULA)
    {
        beebScreen_Init(2, BS_INIT_NULA|BS_INIT_DOUBLE_BUFFER);
    }
    else
    {
        beebScreen_Init(2, BS_INIT_DOUBLE_BUFFER);
    }
    beebScreen_InjectCode(bagiCode_bin,bagiCode_bin_len,0x900);

    if (USE_NULA)
    {
        beebScreen_SetUserVector(BS_VECTOR_VSYNC,0x960);
    }

    beebScreen_SetGeometry(128,192, 1);
    beebScreen_UseDefaultScreenBases();
    beebScreen_SetBuffer(buffer,BS_BUFFER_FORMAT_8BPP,128,192);

    setupPalette();
   
    beebScreen_SendPal(palette,16);

    // TODO - Do Raytrace code here
   
    init();

    memset(buffer,0,128*192);

    int quit = 0;

    while(!quit)
    {
        for(int i = 0; i < 192*5; i+=5)
        {
            // Test for running in beebEm since it's not fast enough to render the whole scene in one pass
            // beebScreen_Flip();

            for(int j = 0; j < 1280; j+=10)
            {

                float x = 0.3;
                float y = -0.5;
                float z = 0;
                int ba = 2;
                float dx = j - 640;
                float dy = i - (96*5);
                float dz = 1300;
                float dd = dx * dx + dy * dy + dz * dz;

                int finish = 0;
                float s = 0;
                do
                {
                    int n = (y >= 0 || dy <= 0) ? -1 : 0;
                    if (n == 0)
                    {
                        s = -y / dy;
                    }

                    for(int k=0; k < BALLS; ++k )
                    {
                        float px = c[k][0] - x;
                        float py = c[k][1] - y;
                        float pz = c[k][2] - z;
                        float sc = px * dx + py * dy + pz * dz;
                        if (sc > 0) 
                        {
                            float bb = sc * sc / dd;
                            float aa = q[k] - (px * px + py * py + pz * pz) + bb;
                            if (aa > 0)
                            {
                                sc = (SQRT(bb) - SQRT(aa)) / SQRT(dd);
                                if (sc < s || n < 0) {
                                    n = k + 1;
                                    s = sc;
                                }
                            }
                        }
                    }
                    
                    if (n < 0)
                    {
                        plot(j,i,0 + (dy*dy/dd) * 7);
                        n = 0;
                        finish = 1;
                    }
                    else
                    {
                        dx = dx * s;
                        dy = dy * s;
                        dz = dz * s;
                        dd = dd * s * s;
                        x = x + dx;
                        y = y + dy;
                        z = z + dz;
                    
                        if (n > 0) {
                            float nx = x - c[n-1][0];//(n, 1)
                            float ny = y - c[n-1][1];//c(n, 2)
                            float nz = z - c[n-1][2];//c(n, 3)
                            float nn = nx * nx + ny * ny + nz * nz;
                            float l = 2 * (dx * nx + dy * ny + dz * nz) / nn;
                            dx = dx - nx * l;
                            dy = dy - ny * l;
                            dz = dz - nz * l;
                        }
                        else
                        {
                            for (int k = 0 ; k < BALLS; ++k)
                            {
                                float u = c[k][0] - x;
                                float v = c[k][2] - z;
                                if (u * u + v * v <= q[k])
                                {
                                    ba = 5;
                                    k = BALLS;
                                }
                            }

                            if (usePic)                        
                            {
                                int u = ((picSize>>1) + (int)(x * qpicSize)) % picSize;
                                int v = (picSize-1) - ((picSize>>1) + (int)(z * qpicSize)) % picSize;

                                int col = remap[pic[(v*picSize)+u] % 8];

                                if (ba == 5)
                                {
                                    col+=8;
                                }
                                plot(j,i,col);
                            }
                            else
                            {
                                if ((x - floor(x) > 0.5) == (z - floor(z) > 0.5))
                                {
                                    plot(j, i, c1[ba]);
                                }
                                else
                                {
                                    plot(j, i, c1[ba + 1]);
                                }
                            }
                            finish = 1;
                        }
                    }
                }
                while(!finish);
            }
        }

        for(int k=0; k < BALLS; ++k)
        {
            c[k][1] += v[k];
            if (c[k][1] > -r[k])
            {
                c[k][1] = -r[k];
                v[k]=-v[k];
            }
            else
            {
                v[k]+=0.01;
            }
        }

        beebScreen_Flip();
    }

    _swi(OS_ReadC,_OUT(0),&ch);

    beebScreen_Quit();

    _VDU(22);_VDU(7);

    printf("Testing.\n");
}