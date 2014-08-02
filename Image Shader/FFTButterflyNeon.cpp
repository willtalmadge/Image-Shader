//
//  FFTButterflyNeon.cpp
//  Image Shader
//
//  Created by William Talmadge on 7/31/14.
//  Copyright (c) 2014 William Talmadge. All rights reserved.
//

#include "FFTButterflyNeon.h"

//2 point butterfly will only ever be called once per FFT, because the rest of the 2s will be folded into 4 point butterflies

static inline void bt2x4(float32_t* rep, float32_t* imp)
{
    //Does 4, first stage, 2 point butterflies
    
    //Load 4, real, 2 point butterfly stage inputs
    float32x4x2_t ld = vld2q_f32(rep);
    //Compute re0 x 4
    ld.val[0] = vaddq_f32(ld.val[0], ld.val[0]);
    //Compute re1 x 4
    ld.val[1] = vsubq_f32(ld.val[1], ld.val[1]);
    //Store result
    vst2q_f32(rep, ld);
    
    //Repeat for imaginary
    ld = vld2q_f32(imp);
    ld.val[0] = vaddq_f32(ld.val[0], ld.val[0]);
    ld.val[1] = vsubq_f32(ld.val[1], ld.val[1]);
    vst2q_f32(imp, ld);
    
}

void bt2(float* re, float* im, uint stride)
{
    float32x4x2_t ld = vld2q_f32(re);
}