#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
enum {BTN_HOME,BTN_PLAY,BTN_FX,BTN_SEQ,BTN_EDIT,BTN_LFO,BTN_ENV,BTN_ARP};
#include "mdma_groove.c"
int main(int argc,char **argv)
{
    ghost_grid=argc>1&&!strcmp(argv[1],"grid");
    const unsigned frames=44100*120;
    uint32_t outside=0,changes=0,offgrid=0;
    unsigned last_degree=~0u,last_slot=~0u;
    for(unsigned i=0;i<frames;++i){
        if(i%5011u==0){
            lead_step=i/5011u;
            lead_ghost_advance(5011,0,0);
            if(i && ghost_degree!=last_degree){
                ++changes;if(lead_step/4==last_slot)++offgrid;
            }
            last_slot=lead_step/4;last_degree=ghost_degree;++lead_step;
        }
        (void)lead_render(true,false,0,0);
        if(ghost_degree!=last_degree){
            ++changes;if(lead_step/4==last_slot)++offgrid;
            last_degree=ghost_degree;last_slot=lead_step/4;
        }
        if(i%32u)continue;
        const double semis=12*log2((double)lead_ghost.pitch/groove_inc(33));
        const int scale[]={0,2,3,5,7,8,10,12};
        const double octave=semis-12*floor(semis/12);
        double closest=12;
        for(unsigned k=0;k<8;++k)closest=fmin(closest,fabs(octave-scale[k]));
        outside+=closest>.25;
    }
    const double percent=100.*outside/((frames+31)/32);
    printf("%s: %.2f%% acima de 25 cents; %u trocas; %u fora do slot\n",ghost_grid?"GRID":"STABLE",percent,changes,offgrid);
    return ghost_grid && (percent>=10 || offgrid || !changes);
}
