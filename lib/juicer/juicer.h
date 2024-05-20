#ifndef JUICER_H
#define JUICER_H

#include <stdlib.h>

// Juicer library, to add 'juice' to animations.

// Easing functions for smoothing movement, useful for animations.
// These work by transforming the 1D input parameter (position, time)
// into values which give smoother animation.
//
// Input is a floating point value (typically a position) the range [0,1],
// or [0,'range'] for the 'R' functions,
// or ['max','min'] for the 'MM' functions.
// Output is a 'smoothed' floating point value in the range [0,1] (can exceed 1 for the 'bounceBack' functions).
// The 'Int1' functions can provide output values > 1, however they 
// have an integral of 1 over the input paramter range [0,1].

float juice_smoothStart2(const float inp);
float juice_smoothStart2R(const float inp, const float range);
float juice_smoothStart2MM(const float inp, const float min, const float max);
float juice_smoothStart2Int1(const float inp);
float juice_smoothStart3(const float inp);
float juice_smoothStart3R(const float inp, const float range);
float juice_smoothStart3MM(const float inp, const float min, const float max);
float juice_smoothStart4(const float inp);
float juice_smoothStart4R(const float inp, const float range);
float juice_smoothStart4MM(const float inp, const float min, const float max);

float juice_smoothStop2(const float inp);
float juice_smoothStop2R(const float inp, const float range);
float juice_smoothStop2MM(const float inp, const float min, const float max);
float juice_smoothStop2Int1(const float inp);
float juice_smoothStop3(const float inp);
float juice_smoothStop3R(const float inp, const float range);
float juice_smoothStop3MM(const float inp, const float min, const float max);
float juice_smoothStop4(const float inp);
float juice_smoothStop4R(const float inp, const float range);
float juice_smoothStop4MM(const float inp, const float min, const float max);

float juice_arch2(const float inp);
float juice_arch2R(const float inp, const float range);
float juice_arch2MM(const float inp, const float min, const float max);
float juice_arch2Int1(const float inp);

float juice_smoothStopArch3(const float inp);
float juice_smoothStopArch3R(const float inp, const float range);
float juice_smoothStopArch3MM(const float inp, const float min, const float max);
float juice_smoothStopArch3Int1(const float inp);

float juice_bounceBack(const float inp);
float juice_bounceBackR(const float inp, const float range);
float juice_bounceBackMM(const float inp, const float min, const float max);

float juice_bounceBack2(const float inp);
float juice_bounceBack2R(const float inp, const float range);
float juice_bounceBack2MM(const float inp, const float min, const float max);

//'bounces back' and oscillates
float juice_bounceBackOsc(const float inp);
float juice_bounceBackOscR(const float inp, const float range);
float juice_bounceBackOscMM(const float inp, const float min, const float max);

#endif
