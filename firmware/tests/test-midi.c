#include <assert.h>
#define DINGHY_HOST 1
#define APP_VER "test"
#define main original_dinghy_main
#include "dinghy.c"
#undef main
int main(void)
{
    panel_defaults();
    qa_midi(1,0xB0,10,5);qa_midi(1,0xB0,74,47);qa_midi(1,0xB0,71,90);
    assert(panel_mix[0][0][MIX_PAN]==5&&panel_mix[0][0][MIX_FLT]==47&&panel_mix[0][0][MIX_RES]==90);
    qa_midi(1,0xB2,7,0); assert(panel_mix[0][2][MIX_VOL]==0&&panel_mix[0][3][MIX_VOL]==102);
    qa_midi(2,0xB2,7,127); assert(panel_mix[0][2][MIX_VOL]==127);
    play_from(60, 100, 17);
    stop_from(60, 1); assert(voice.gate);
    stop_from(60, 18); assert(voice.gate);
    stop_from(60, 17); assert(!voice.gate);
    qa_source=1;
    qa_midi(2,0xFA,0,0);assert(!qa_running);
    qa_midi(1,0xFA,0,0);assert(qa_running&&groove_enabled==63);
    for(unsigned i=0;i<25;i++){
        fm1_ms=i*20;
        qa_midi(2,0xF8,0,0);
        qa_midi(1,0xF8,0,0);
    }
    assert(qa_ticks==5&&qa_external_bpm==125);
    qa_midi(1,0xFC,0,0);assert(!qa_running&&!groove_enabled);
    qa_midi(1,0xFB,0,0);assert(qa_running&&groove_enabled==63&&qa_ticks==5);
    qa_save_pending=true;
    qa_midi(1,0xFA,0,0);assert(qa_ticks==5);
    puts("MIDI: isolated source, clock/6, BPM, START/STOP/CONTINUE, CC7 passed");
}
