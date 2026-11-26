#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
enum { BTN_HOME,BTN_PLAY,BTN_FX,BTN_SEQ,BTN_EDIT,BTN_LFO,BTN_ENV,BTN_ARP };
#include "mdma_groove.c"
int main(void){
    panel_defaults();stage_enabled=true;
    for(unsigned l=0;l<6;l++){
        panel_mix[0][l][MIX_FLT]=l%2?84:44;
        for(unsigned k=0;k<4;k++)panel_fx[0][l][k]=55;
    }
    panel_publish();
    for(unsigned step=0;step<128;step++){
        ghost_time_q24=(uint64_t)step<<21;
        ghost_auto_q15=16384u;
        stage_spark_side=step&1?1:-1;stage_spark_remaining=stage_spark_len=1000;
        stage_update(step<<24,GROOVE_ALL);
        for(unsigned l=0;l<6;l++)printf("%u %u\n",stage_mix[l],stage_fx[l]);
    }
}
