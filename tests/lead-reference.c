#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static uint32_t groove_inc(unsigned semitone)
{
    static const uint32_t octave[12] = {
        3185015,3374406,3575058,3787642,4012867,4251485,4504291,4772130,5055896,5356535,5675051,6012507
    };
    return octave[semitone % 12u] << (semitone / 12u);
}

#include "mdma_lead.c"

int main(int argc, char **argv)
{
    const unsigned profile = argc > 1 ? (unsigned)atoi(argv[1]) : 0;
    if (profile > 2) return 1;
    lead_alien = argc > 2;
    lead_hand_pitch = 21426141;
    for (unsigned i = 0; i < 220500; ++i) {
        const bool playing = i < 176400;
        const bool manual = i >= 44100 && i < 88200;
        if (playing && i % 5011 == 0) {
            lead_step = i / 5011;
            lead_ghost_advance(5011, profile, 0);
            ++lead_step;
        }
        const int32_t sample = lead_render(playing, manual, profile, 0);
        printf("%d,%u,%d,%d,%d\n", sample, ghost_degree, lead_ghost.target_cutoff,
               lead_ghost.gain, lead_hand.gain);
    }
}
