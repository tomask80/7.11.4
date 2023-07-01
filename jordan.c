#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <math.h>

#define POKAZIL_SA_ALLOC 2
#define PRAVOTOCIVY 3
#define LAVOTOCIVY 4

#define DEF_XY(sx, sy, i, x, y) \
    sx[i] = x;                  \
    sy[i] = y;

static int FACTOR = 200;

typedef struct
{
    float x;
    float y;
} POINT;

typedef struct
{
    POINT beg;
    POINT end;
} SEGMENT;


SEGMENT *createSEGMENT(float x1, float y1, float x2, float y2)
{
    SEGMENT *s = malloc(sizeof(SEGMENT));
      if(s == NULL) {
        return POKAZIL_SA_ALLOC;
    }
    s->beg.x = x1;
    s->beg.y = y1;
    s->end.x = x2;
    s->end.y = y2;
    return s;
}

void destroySEGMENT(SEGMENT *s)
{
    free(s);
}

void destroySEGMENTOV(SEGMENT **segmenty, unsigned int l)
{
    for (int i = 0; i < l; i++)
    {
        if (segmenty[i] != NULL)
        {
            destroySEGMENT(segmenty[i]);
            segmenty[i] = NULL;
        }
    }
    free(segmenty);
}

void svg_hlavicka(int width, int height,char *filename)
{
    FILE *subor = fopen(filename, "w");
    // error handling - subor sa neotvori ...
    if(subor==NULL){
        printf("Subor sa nepodarilo otvorit\n");
        exit(1);
    }
        
    fprintf(subor, "<!DOCTYPE html>\n");
    fprintf(subor, "<html>\n");
    fprintf(subor, "<body>\n");
    fprintf(subor, "<svg height=\"%d\" width=\"%d\">\n", width, height);
    fprintf(subor, "<defs>\
    <marker\
      id=\"triangle\"\
      viewBox=\"0 0 30 10\"\
      refX=\"1\"\
      refY=\"5\"\
      markerUnits=\"strokeWidth\"\
      markerWidth=\"10\"\
      markerHeight=\"10\"\
      orient=\"auto\">\
      <path d=\"M 0 0 L 10 5 L 0 10 z\" fill=\"#00f\" />\
    </marker>\
  </defs>");
    fflush(subor);
    fclose(subor);
}

int scale(float val)
{
    return val * FACTOR;
}

void svg_kresli(float x1, float y1, float x2, float y2, char *filename)
{
    FILE *subor = fopen(filename, "a");
    fprintf(subor, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" style=\" stroke:rgb(228, 21, 21); stroke-width:1\" marker-start=\"url(#triangle)\"/>\n",
            scale(x1), scale(y1),
            scale(x2), scale(y2));
    fflush(subor);
    fclose(subor);
}

void svg_chvost(char *filename)
{
    FILE *subor = fopen(filename, "a");
    fprintf(subor, "</svg>\n");
    fprintf(subor, "</body>\n");
    fprintf(subor, "</html>\n");
    fclose(subor);
}

void createSvg(float *x, float *y, unsigned int l,char *filename)
{
    svg_hlavicka(FACTOR + 1, FACTOR + 1,filename);
    for (int i = 0; i < l-1; i++)
    {
        svg_kresli(x[i],y[i],x[i+1], y[i+1], filename);
    }

    svg_kresli(x[l-1], y[l-1],x[0], y[0], filename);

    svg_chvost(filename);
    printf("SVG %s vytvorene\n", filename);
}

// vypocitam vektor a uhol medzi nimi
double vypocitajUholSegmentov(SEGMENT *segmentA, SEGMENT *segmentB)
{
    // vektor A
    float vektorAx = segmentA->end.x - segmentA->beg.x;
    float vektorAy = segmentA->end.y - segmentA->beg.y;
    // vektor B
    float vektorBx = segmentB->end.x - segmentB->beg.x;
    float vektorBy = segmentB->end.y - segmentB->beg.y;

    double arcVektoraA = atan2(vektorAy, vektorAx);
    double arcVektoraB = atan2(vektorBy, vektorBx);

    double clockAngleADgs = arcVektoraA * 180 / M_PI;
    double clockAngleBDgs = arcVektoraB * 180 / M_PI;

    // dlzka B vektora
    double rB = sqrt(pow(vektorBx, 2) + pow(vektorBy, 2));

    // potocim vector B of uhol vectora spat - zorientujem podla podla vstupneho vektora
    double xB = cos(arcVektoraB-arcVektoraA)*rB;
    double yB = sin(arcVektoraB-arcVektoraA)*rB;

    double angleABDgs = atan2(yB, xB) * 180 / M_PI;
    return angleABDgs;

}

unsigned int pocitaj_orientaciu(float *x, float *y, unsigned int l)
{
    float counter = 0;
    SEGMENT **segmenty = malloc(l * sizeof(SEGMENT *));
      if(segmenty == NULL) {
        return POKAZIL_SA_ALLOC;
    }
    // init segmentov
    for (int i = 0; i < l; i++)
    {
        segmenty[i] = NULL;
    }

    // nainicializujem segmenty
    for (int i = 1; i < l; i++)
    {
        segmenty[i - 1] = createSEGMENT(x[i - 1], y[i - 1], x[i], y[i]);
    }
    // zapojenie posledneho segmentu do kruhu
    segmenty[l - 1] = createSEGMENT(x[l - 1], y[l - 1], x[0], y[0]);


    double sumDeltaAngles = 0;
    for (int i = 0; i < l-1; i++)
    {
        double angleDgs = vypocitajUholSegmentov(segmenty[i], segmenty[i + 1]);
        sumDeltaAngles += angleDgs;
    }
    double angleDgs = vypocitajUholSegmentov(segmenty[l-1], segmenty[0]);
    sumDeltaAngles += angleDgs;


    if (sumDeltaAngles>0) {
        return LAVOTOCIVY;
    } else {
        return PRAVOTOCIVY;
    }


    // uvolnenie smegmentov v pamati
    destroySEGMENTOV(segmenty, l);
}

unsigned int napchajStvorec()
{
    float setX[4];
    float setY[4];

    // stvorec
    DEF_XY(setX, setY, 0, 0.10, 0.10);
    DEF_XY(setX, setY, 1, 0.20, 0.10);
    DEF_XY(setX, setY, 2, 0.20, 0.20);
    DEF_XY(setX, setY, 3, 0.10, 0.20);

    //  vizualizacia cez SVG
    createSvg(setX, setY, 4, "stvorec.html");

    return pocitaj_orientaciu(setX, setY, 4);
}

unsigned int napchajStvorecOpacne()
{
    float setX[4];
    float setY[4];

    // stvorec opacne
    DEF_XY(setX, setY, 3, 0.10, 0.10);
    DEF_XY(setX, setY, 2, 0.20, 0.10);
    DEF_XY(setX, setY, 1, 0.20, 0.20);
    DEF_XY(setX, setY, 0, 0.10, 0.20);

    //  vizualizacia cez SVG
    createSvg(setX,setY, 4, "stvorec-opacne.html");
    return pocitaj_orientaciu(setX, setY, 4);
}

unsigned int krivka()
{
    float setX[8];
    float setY[8];

    // stvorec opacne
    DEF_XY(setX, setY, 0, 0.10, 0.10);
    DEF_XY(setX, setY, 1, 0.20, 0.10);
    DEF_XY(setX, setY, 2, 0.20, 0.20);
    DEF_XY(setX, setY, 3, 0.30, 0.20);
    DEF_XY(setX, setY, 4, 0.30, 0.10);
    DEF_XY(setX, setY, 5, 0.40, 0.20);
    DEF_XY(setX, setY, 6, 0.40, 0.30);
    DEF_XY(setX, setY, 7, 0.10, 0.30);

    //  vizualizacia cez SVG
    createSvg(setX, setY, 8, "krivka.html");

    return pocitaj_orientaciu(setX, setY, 8);
}

int main(void)
{
   if(napchajStvorec()==3)
   {
    printf("pravotociva krivka\n");
   }
   else{
    printf("lavotociva krivka\n");
   }

    if(napchajStvorecOpacne()==3)
   {
    printf("pravotociva krivka\n");
   }
   else{
    printf("lavotociva krivka\n");
   }

   if(krivka()==3)
   {
    printf("pravotociva krivka\n");
   }
   else{
    printf("lavotociva krivka\n");
   }
   
}
