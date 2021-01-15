#include "beebScreen/beebScreen.h"
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "sqrtf.h"

#include "beebScreen/bagiCode.c"

int palette[256];
int USE_NULA = 0;

#define BALLS 5
#define COLOURS 5

unsigned char buffer[128*192];

void syntax()
{
    printf("Syntax: rt [-n]\n");

    exit(1);
}

typedef struct {
    float x,y,z,r,q,v;
} Ball;

#define BALL(i,xf,yf,zf,rad) balls[i].x=xf; balls[i].y=yf; balls[i].z=zf; balls[i].r =rad; balls[i].q=rad*rad; balls[i].v=0;

Ball balls[BALLS];

const int c1[] = {1,6,0,7,5,8,15};

void init()
{
    BALL(0, -0.8,-1,3.2, 0.7);
    BALL(1, 0,-0.45,2, 0.3);
    BALL(2, 1.2,-0.7,2.5, 0.5);
    BALL(3, 0.4,-1,4, 0.4);
    BALL(4, 0.5,-0.5,1.5, 0.1);
}
	      
void plot(int x, int y, int col)
{
    int localx = x / 10;
    int localy = y / 5;
    buffer[(localy * 128) + localx] = col;
}

float SQRT(float N)
{
#if 0
  return sqrtf(N);
#else
  return __ieee754_sqrtf(N);
#endif
}

const int palSource[] = {
    0xff0000, 0x000000, 0x0000ff, 0xff00ff, 0x00ff00, 0xffff00, 0x00ffff,0xffffff,
    0x7f0000, 0x000000, 0x00007f, 0x7f007f, 0x007f00, 0x7f7f00, 0x007f7f,0x7f7f7f
};

const int remap[] = {1,2,4,6,0,3,5,7};

void setupPalette()
{
    beebScreen_SetNulaPal(palSource,palette,16,beebScreen_extractRGB888);
}

const char *fontChars="0123456789:.";

#define OOO 0
#define OOX 1
#define OXO 2
#define OXX 3
#define XOO 4
#define XOX 5
#define XXO 6
#define XXX 7

char fontData[]={
    XXX,XOX,XOX,XOX,XXX,0,    // 0
    OXO,XXO,OXO,OXO,XXX,0,    // 1
    XXO,OOX,XXO,XOO,XXX,0,    // 2
    XXO,OOX,OXO,OOX,XXO,0,    // 3
    XOX,XOX,XXX,OOX,OOX,0,    // 4
    XXX,XOO,OXX,OOX,XXO,0,    // 5
    OXX,XOO,XXX,XOX,XXX,0,    // 6
    XXX,OOX,OXO,XOO,XOO,0,    // 7
    XXX,XOX,OXO,XOX,XXX,0,    // 8
    XXX,XOX,XXX,OOX,XXO,0,    // 9
    OOO,OXO,OOO,OXO,OOO,0,    // :
    OOO,OOO,OOO,OXO,OOO,0     // .
};

void drawUI(unsigned char *buffer,int w,int h,float fps)
{
    char timeText[32];

    int bg = 1;
    int fg = 7;

    snprintf(timeText,32,"%4.1f",fps);
    int x = w-16;
    int y = 0;
    char *p=timeText;
    while(*p)
    {
        if (strchr(fontChars,*p))
        {
            unsigned char *ptr=&buffer[y*w+x];
            int chr = strchr(fontChars,*p)-fontChars;
            char *c = &fontData[chr*6];
            for(int r=0;r<6;++r,ptr+=w-4)
            {
                int m=8;
                int v=*c++;
                for(int col=0;col<4;col++,m>>=1,ptr++)
                {
                    if (v & m)
                    {
                        *ptr = fg;
                    }
                    else
                    {
                        *ptr = bg;
                    }
                }
            }
        }   
        x+=4;
        p++;
    }
}

int usePic = 0;
unsigned char *pic;
int picIndex = 0;
int picSize = 64;

void raytraceScene()
{
    float qpicSize =(float)picSize/4.0;

    for(int i = 0; i < 960; i+=5)
    {
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
                    Ball *b=&balls[k];
                    float px = b->x - x;
                    float py = b->y - y;
                    float pz = b->z - z;
                    float sc = px * dx + py * dy + pz * dz;
                    if (sc > 0) 
                    {
                        float bb = sc * sc / dd;
                        float aa = b->q - (px * px + py * py + pz * pz) + bb;
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
                        Ball *b=&balls[n-1];
                        float nx = x - b->x;
                        float ny = y - b->y;
                        float nz = z - b->z;
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
                            Ball *b=&balls[k];
                            float u = b->x - x;
                            float v = b->z - z;
                            if (u * u + v * v <= b->q)
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
}

int main(int argc, char *argv[])
{
    int index = -1;
    int audio = 1;
    int stream = 0;

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

    _swi(OS_ReadC, _OUT(0), &ch);

    if (USE_NULA)
    {
        beebScreen_Init(2, BS_INIT_NULA | BS_INIT_DOUBLE_BUFFER);
    }
    else
    {
        beebScreen_Init(2, BS_INIT_DOUBLE_BUFFER);
    }
    beebScreen_InjectCode(bagiCode_bin, bagiCode_bin_len, 0x900);

    if (USE_NULA)
    {
        beebScreen_SetUserVector(BS_VECTOR_VSYNC, 0x960);
    }

    beebScreen_SetGeometry(128, 192, 1);
    beebScreen_UseDefaultScreenBases();
    beebScreen_SetBuffer(buffer,BS_BUFFER_FORMAT_8BPP, 128, 192);

    setupPalette();
   
    beebScreen_SendPal(palette, 16);

    // TODO - Do Raytrace code here
   
    init();

    memset(buffer, 0, 128*192);

    int quit = 0;
    int lastFrameCount = beebScreen_GetFrameCounter();

    int frameDeltas[5]={1,1,1,1,1};
    int deltaIndex=0;

    while(!quit)
    {
        // Raytrace the entire scene
        raytraceScene();

        // Update balls positions
        for(int k=0; k < BALLS; ++k)
        {
            Ball *b=&balls[k];
            b->y += b->v;
            if (b->y > -b->r)
            {
                b->y = -b->r;
                b->v= -b->v;
            }
            else
            {
                b->v+=0.01;
            }
        }

        // Show frame counter
        int nextFrameCount = beebScreen_GetFrameCounter();
        int delta = nextFrameCount - lastFrameCount;
        if (delta < 0)
        {
            delta+=256;
        }
        frameDeltas[deltaIndex] = delta;
        deltaIndex = (deltaIndex + 1) % 5;
        lastFrameCount = nextFrameCount;

        int deltaCount = 0;
        for(int i=0;i<5;++i)
        {
            deltaCount+=frameDeltas[i];
        }

        drawUI(buffer,128,192,250.0f/(float)deltaCount);

        beebScreen_Flip();
    }

    _swi(OS_ReadC,_OUT(0),&ch);

    beebScreen_Quit();

    _VDU(22);_VDU(7);

    printf("Testing.\n");
}