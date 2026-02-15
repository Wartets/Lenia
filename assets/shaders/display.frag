#version 450 core

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 fragColor;

layout(binding = 0) uniform sampler2D uStateTex;
layout(binding = 1) uniform sampler1D uColormapTex;

uniform float uZoom;
uniform vec2  uPan;
uniform int   uColormapMode;
uniform float uBrightness;
uniform float uContrast;
uniform float uGridAspect;
uniform float uViewAspect;
uniform int   uFilterMode;
uniform float uEdgeStrength;
uniform float uGlowStrength;
uniform float uGamma;
uniform int   uInvertColors;
uniform int   uShowGrid;
uniform float uGridOpacity;
uniform int   uGridW;
uniform int   uGridH;
uniform int   uMultiChannel;
uniform int   uDisplayMode;

uniform int   uClipNullCells;
uniform float uClipThreshold;
uniform vec3  uBgColor;

uniform vec3  uGridLineColor;
uniform float uGridLineThickness;
uniform int   uGridSpacingMode;
uniform int   uGridCustomSpacing;
uniform int   uGridMajorLines;
uniform int   uGridMajorEvery;
uniform float uGridMajorOpacity;

uniform float uCmapOffset;
uniform float uCmapRange0;
uniform float uCmapRange1;
uniform float uCmapPower;
uniform float uCmapHueShift;
uniform float uCmapSaturation;
uniform int   uCmapReverse;

uniform int   uShowBoundary;
uniform vec3  uBoundaryColor;
uniform float uBoundaryOpacity;
uniform int   uBoundaryStyle;
uniform float uBoundaryThickness;
uniform int   uBoundaryAnimate;
uniform float uBoundaryDashLength;
uniform float uTime;

uniform int   uEdgeModeX;
uniform int   uEdgeModeY;
uniform float uEdgeValueX;
uniform float uEdgeValueY;
uniform float uEdgeFadeX;
uniform float uEdgeFadeY;
uniform int   uDisplayEdgeMode;

uniform int   uContourLevels;
uniform float uContourThickness;
uniform float uVectorFieldScale;
uniform int   uVectorFieldDensity;
uniform vec3  uGlowColor;
uniform float uGlowIntensity;

uniform int   uMultiChannelBlend;
uniform vec3  uChannelWeights;
uniform int   uUseColormapForMultichannel;

vec3 viridis(float t) {
    vec3 c0 = vec3(0.2777, 0.0054, 0.3340);
    vec3 c1 = vec3(0.1050, 0.4114, 0.5036);
    vec3 c2 = vec3(0.1270, 0.5660, 0.5506);
    vec3 c3 = vec3(0.2302, 0.6860, 0.5410);
    vec3 c4 = vec3(0.4775, 0.8212, 0.3180);
    vec3 c5 = vec3(0.9930, 0.9062, 0.1439);
    t = clamp(t, 0.0, 1.0);
    if (t < 0.2) return mix(c0, c1, t / 0.2);
    if (t < 0.4) return mix(c1, c2, (t - 0.2) / 0.2);
    if (t < 0.6) return mix(c2, c3, (t - 0.4) / 0.2);
    if (t < 0.8) return mix(c3, c4, (t - 0.6) / 0.2);
    return mix(c4, c5, (t - 0.8) / 0.2);
}

vec3 magma(float t) {
    vec3 c0 = vec3(0.0015, 0.0005, 0.0139);
    vec3 c1 = vec3(0.2776, 0.0510, 0.3755);
    vec3 c2 = vec3(0.5756, 0.1476, 0.4526);
    vec3 c3 = vec3(0.8584, 0.3167, 0.3378);
    vec3 c4 = vec3(0.9824, 0.6004, 0.3595);
    vec3 c5 = vec3(0.9870, 0.9914, 0.7497);
    t = clamp(t, 0.0, 1.0);
    if (t < 0.2) return mix(c0, c1, t / 0.2);
    if (t < 0.4) return mix(c1, c2, (t - 0.2) / 0.2);
    if (t < 0.6) return mix(c2, c3, (t - 0.4) / 0.2);
    if (t < 0.8) return mix(c3, c4, (t - 0.6) / 0.2);
    return mix(c4, c5, (t - 0.8) / 0.2);
}

vec3 inferno(float t) {
    vec3 c0 = vec3(0.0015, 0.0005, 0.0139);
    vec3 c1 = vec3(0.2581, 0.0388, 0.4065);
    vec3 c2 = vec3(0.5783, 0.1481, 0.4040);
    vec3 c3 = vec3(0.8490, 0.2897, 0.2001);
    vec3 c4 = vec3(0.9882, 0.5766, 0.0399);
    vec3 c5 = vec3(0.9882, 0.9985, 0.6449);
    t = clamp(t, 0.0, 1.0);
    if (t < 0.2) return mix(c0, c1, t / 0.2);
    if (t < 0.4) return mix(c1, c2, (t - 0.2) / 0.2);
    if (t < 0.6) return mix(c2, c3, (t - 0.4) / 0.2);
    if (t < 0.8) return mix(c3, c4, (t - 0.6) / 0.2);
    return mix(c4, c5, (t - 0.8) / 0.2);
}

vec3 plasma(float t) {
    vec3 c0 = vec3(0.0504, 0.0298, 0.5280);
    vec3 c1 = vec3(0.4177, 0.0056, 0.6582);
    vec3 c2 = vec3(0.6942, 0.1651, 0.5364);
    vec3 c3 = vec3(0.8810, 0.3924, 0.3267);
    vec3 c4 = vec3(0.9882, 0.6524, 0.0399);
    vec3 c5 = vec3(0.9400, 0.9752, 0.1313);
    t = clamp(t, 0.0, 1.0);
    if (t < 0.2) return mix(c0, c1, t / 0.2);
    if (t < 0.4) return mix(c1, c2, (t - 0.2) / 0.2);
    if (t < 0.6) return mix(c2, c3, (t - 0.4) / 0.2);
    if (t < 0.8) return mix(c3, c4, (t - 0.6) / 0.2);
    return mix(c4, c5, (t - 0.8) / 0.2);
}

vec3 grayscale(float t) {
    return vec3(t);
}

vec3 grayscaleInv(float t) {
    return vec3(1.0 - t);
}

vec3 jet(float t) {
    t = clamp(t, 0.0, 1.0);
    float r = clamp(1.5 - abs(t - 0.75) * 4.0, 0.0, 1.0);
    float g = clamp(1.5 - abs(t - 0.50) * 4.0, 0.0, 1.0);
    float b = clamp(1.5 - abs(t - 0.25) * 4.0, 0.0, 1.0);
    return vec3(r, g, b);
}

vec3 rgb2hsv(vec3 c) {
    vec4 K = vec4(0.0, -1.0/3.0, 2.0/3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0/3.0, 1.0/3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

float applyColormapDeformation(float t) {
    float range = uCmapRange1 - uCmapRange0;
    if (range > 0.0) {
        t = clamp((t - uCmapRange0) / range, 0.0, 1.0);
    }
    if (uCmapPower != 1.0 && uCmapPower > 0.0) {
        t = pow(t, uCmapPower);
    }
    if (uCmapReverse != 0) {
        t = 1.0 - t;
    }
    if (uCmapOffset != 0.0) {
        t = fract(t + uCmapOffset);
    }
    return clamp(t, 0.0, 1.0);
}

vec3 applyHueSatShift(vec3 col) {
    if (uCmapHueShift == 0.0 && uCmapSaturation == 1.0) return col;
    vec3 hsv = rgb2hsv(col);
    hsv.x = fract(hsv.x + uCmapHueShift);
    hsv.y *= uCmapSaturation;
    hsv.y = clamp(hsv.y, 0.0, 1.0);
    return hsv2rgb(hsv);
}

vec2 applyEdgeMode(vec2 coord) {
    vec2 result = coord;
    if (uEdgeModeX == 0) {
        result.x = fract(result.x);
    } else if (uEdgeModeX == 1) {
        result.x = clamp(result.x, 0.0, 1.0);
    } else if (uEdgeModeX == 2) {
        float mx = mod(result.x, 2.0);
        result.x = mx < 1.0 ? mx : 2.0 - mx;
    }
    if (uEdgeModeY == 0) {
        result.y = fract(result.y);
    } else if (uEdgeModeY == 1) {
        result.y = clamp(result.y, 0.0, 1.0);
    } else if (uEdgeModeY == 2) {
        float my = mod(result.y, 2.0);
        result.y = my < 1.0 ? my : 2.0 - my;
    }
    return result;
}

bool isOutsideGrid(vec2 coord) {
    return coord.x < 0.0 || coord.x >= 1.0 || coord.y < 0.0 || coord.y >= 1.0;
}

float getEdgeFade(vec2 coord) {
    float fade = 1.0;
    if (uEdgeModeX != 0 && uEdgeFadeX > 0.0) {
        float distX = min(coord.x, 1.0 - coord.x);
        fade *= smoothstep(0.0, uEdgeFadeX, distX);
    }
    if (uEdgeModeY != 0 && uEdgeFadeY > 0.0) {
        float distY = min(coord.y, 1.0 - coord.y);
        fade *= smoothstep(0.0, uEdgeFadeY, distY);
    }
    return fade;
}

void main() {
    vec2 uv = vUV - 0.5;
    float relAspect = uViewAspect / uGridAspect;
    if (relAspect > 1.0)
        uv.x *= relAspect;
    else
        uv.y /= relAspect;
    uv = uv / uZoom + 0.5 - uPan;

    vec2 rawUV = uv;
    bool outside = isOutsideGrid(rawUV);

    if (uDisplayEdgeMode == 1 && outside) {
        fragColor = vec4(uBgColor, 1.0);
        return;
    }
    if (uDisplayEdgeMode == 2 && outside) {
        vec3 checker = mod(floor(rawUV.x * 10.0) + floor(rawUV.y * 10.0), 2.0) < 1.0 ? 
                       vec3(0.1) : vec3(0.15);
        fragColor = vec4(checker, 1.0);
        return;
    }

    uv = applyEdgeMode(rawUV);

    vec2 texSize = vec2(textureSize(uStateTex, 0));
    vec2 texel = 1.0 / texSize;

    if (uDisplayMode == 1 || uDisplayMode == 2) {
        vec4 raw = texture(uStateTex, uv);
        vec3 pos = clamp(raw.rgb, 0.0, 1.0);
        vec3 neg = clamp(-raw.rgb, 0.0, 1.0);
        vec3 col = pos - neg * 0.5;
        col = clamp((col - 0.5) * uContrast + 0.5 + uBrightness - 0.5, vec3(0.0), vec3(1.0));
        if (uGamma != 1.0 && uGamma > 0.0)
            col = pow(col, vec3(1.0 / uGamma));
        fragColor = vec4(col, 1.0);
        return;
    }
    if (uDisplayMode == 3) {
        vec2 kernelUV = vUV;
        kernelUV = kernelUV - 0.5;
        kernelUV = kernelUV / uZoom;
        float kernelAspect = 1.0;
        float viewAspectRatio = uViewAspect;
        if (viewAspectRatio > 1.0)
            kernelUV.x *= viewAspectRatio;
        else
            kernelUV.y /= viewAspectRatio;
        kernelUV = kernelUV + 0.5;
        if (kernelUV.x < 0.0 || kernelUV.x > 1.0 || kernelUV.y < 0.0 || kernelUV.y > 1.0) {
            fragColor = vec4(uBgColor, 1.0);
            return;
        }
        float k = texture(uStateTex, kernelUV).r;
        float maxK = 0.0;
        for (int dy = -2; dy <= 2; ++dy) {
            for (int dx = -2; dx <= 2; ++dx) {
                vec2 sampleUV = kernelUV + vec2(dx, dy) / vec2(textureSize(uStateTex, 0));
                maxK = max(maxK, texture(uStateTex, sampleUV).r);
            }
        }
        float normalizedK = maxK > 0.0 ? k / maxK : k;
        float cmapT = applyColormapDeformation(normalizedK);
        vec3 col;
        switch (uColormapMode) {
            case 1: col = viridis(cmapT); break;
            case 2: col = magma(cmapT); break;
            case 3: col = inferno(cmapT); break;
            case 4: col = plasma(cmapT); break;
            case 5: col = grayscale(cmapT); break;
            case 6: col = grayscaleInv(cmapT); break;
            case 7: col = jet(cmapT); break;
            default: col = texture(uColormapTex, cmapT).rgb; break;
        }
        col = clamp((col - 0.5) * uContrast + 0.5 + uBrightness - 0.5, vec3(0.0), vec3(1.0));
        fragColor = vec4(col, 1.0);
        return;
    }
    
    if (uDisplayMode == 5) {
        float here = texture(uStateTex, uv).r;
        float dx = texture(uStateTex, uv + vec2(texel.x, 0.0)).r - texture(uStateTex, uv - vec2(texel.x, 0.0)).r;
        float dy = texture(uStateTex, uv + vec2(0.0, texel.y)).r - texture(uStateTex, uv - vec2(0.0, texel.y)).r;
        vec2 grad = vec2(dx, dy) * uVectorFieldScale;
        float gridX = mod(uv.x * float(uVectorFieldDensity), 1.0);
        float gridY = mod(uv.y * float(uVectorFieldDensity), 1.0);
        vec2 cellCenter = vec2(0.5);
        vec2 toCenter = vec2(gridX, gridY) - cellCenter;
        float distToCenter = length(toCenter);
        float mag = length(grad);
        vec2 normGrad = mag > 0.001 ? grad / mag : vec2(0.0);
        float arrowBody = 1.0 - smoothstep(0.0, 0.08, abs(dot(toCenter, vec2(-normGrad.y, normGrad.x))));
        float arrowLen = min(mag * 5.0, 0.4);
        float inArrow = arrowBody * step(dot(toCenter, normGrad), arrowLen) * step(-0.4, dot(toCenter, normGrad));
        float tipDist = length(toCenter - normGrad * arrowLen);
        float arrowTip = 1.0 - smoothstep(0.0, 0.12, tipDist);
        float arrow = max(inArrow, arrowTip) * step(0.01, mag);
        vec3 bgCol = vec3(here * 0.3);
        vec3 arrowCol = vec3(0.2, 0.8, 1.0);
        vec3 col = mix(bgCol, arrowCol, arrow * 0.9);
        fragColor = vec4(col, 1.0);
        return;
    }
    
    if (uDisplayMode == 6) {
        float val6 = texture(uStateTex, uv).r;
        float levelF = val6 * float(uContourLevels);
        float fracLevel = fract(levelF);
        float lineWidth = uContourThickness * 0.02;
        float contour = 1.0 - smoothstep(lineWidth, lineWidth * 2.0, min(fracLevel, 1.0 - fracLevel));
        float cmapT = applyColormapDeformation(val6);
        vec3 baseCol;
        switch (uColormapMode) {
            case 1: baseCol = viridis(cmapT); break;
            case 2: baseCol = magma(cmapT); break;
            case 3: baseCol = inferno(cmapT); break;
            case 4: baseCol = plasma(cmapT); break;
            case 5: baseCol = grayscale(cmapT); break;
            case 6: baseCol = grayscaleInv(cmapT); break;
            case 7: baseCol = jet(cmapT); break;
            default: baseCol = texture(uColormapTex, cmapT).rgb; break;
        }
        vec3 lineCol = vec3(1.0) - baseCol;
        vec3 col = mix(baseCol, lineCol, contour * 0.7);
        fragColor = vec4(col, 1.0);
        return;
    }
    
    if (uDisplayMode == 7) {
        float val7 = texture(uStateTex, uv).r;
        float cmapT = applyColormapDeformation(val7);
        vec3 col = inferno(cmapT);
        col = clamp((col - 0.5) * uContrast + 0.5 + uBrightness - 0.5, vec3(0.0), vec3(1.0));
        fragColor = vec4(col, 1.0);
        return;
    }
    
    if (uDisplayMode == 8) {
        vec4 raw8 = texture(uStateTex, uv);
        float activity = length(raw8.rgb);
        float cmapT = applyColormapDeformation(activity);
        vec3 col = plasma(cmapT);
        col = clamp((col - 0.5) * uContrast + 0.5 + uBrightness - 0.5, vec3(0.0), vec3(1.0));
        fragColor = vec4(col, 1.0);
        return;
    }
    
    if (uDisplayMode == 9) {
        float here = texture(uStateTex, uv).r;
        float diff = abs(here - 0.5) * 2.0;
        vec3 col = vec3(diff, 0.0, 1.0 - diff);
        col = clamp((col - 0.5) * uContrast + 0.5 + uBrightness - 0.5, vec3(0.0), vec3(1.0));
        fragColor = vec4(col, 1.0);
        return;
    }

    float val = texture(uStateTex, uv).r;

    bool multiCh = (uMultiChannel != 0);
    vec4 rawPixel = texture(uStateTex, uv);

    if (uClipNullCells != 0) {
        float rawVal = multiCh ? max(rawPixel.r, max(rawPixel.g, rawPixel.b)) : val;
        if (rawVal < uClipThreshold) {
            fragColor = vec4(uBgColor, 1.0);
            return;
        }
    }

    if (uFilterMode == 2) {
        float center = val * 5.0;
        float neighbors = 0.0;
        neighbors += texture(uStateTex, uv + vec2(-texel.x, 0.0)).r;
        neighbors += texture(uStateTex, uv + vec2( texel.x, 0.0)).r;
        neighbors += texture(uStateTex, uv + vec2(0.0, -texel.y)).r;
        neighbors += texture(uStateTex, uv + vec2(0.0,  texel.y)).r;
        val = clamp(center - neighbors, 0.0, 1.0);
    }

    float edgeVal = 0.0;
    if (uEdgeStrength > 0.0) {
        float tl = texture(uStateTex, uv + vec2(-texel.x, -texel.y)).r;
        float tc = texture(uStateTex, uv + vec2(    0.0, -texel.y)).r;
        float tr = texture(uStateTex, uv + vec2( texel.x, -texel.y)).r;
        float ml = texture(uStateTex, uv + vec2(-texel.x,     0.0)).r;
        float mr = texture(uStateTex, uv + vec2( texel.x,     0.0)).r;
        float bl = texture(uStateTex, uv + vec2(-texel.x,  texel.y)).r;
        float bc = texture(uStateTex, uv + vec2(    0.0,  texel.y)).r;
        float br = texture(uStateTex, uv + vec2( texel.x,  texel.y)).r;

        float gx = -tl - 2.0*ml - bl + tr + 2.0*mr + br;
        float gy = -tl - 2.0*tc - tr + bl + 2.0*bc + br;
        edgeVal = sqrt(gx*gx + gy*gy);
    }

    float glowVal = 0.0;
    if (uGlowStrength > 0.0) {
        float sum = 0.0;
        float wt = 0.0;
        for (int dy = -2; dy <= 2; ++dy) {
            for (int dx = -2; dx <= 2; ++dx) {
                float d = float(dx*dx + dy*dy);
                float w = exp(-d * 0.5);
                sum += texture(uStateTex, uv + vec2(float(dx), float(dy)) * texel).r * w;
                wt += w;
            }
        }
        glowVal = sum / wt;
    }

    float finalVal = val;
    if (uGlowStrength > 0.0) {
        finalVal = mix(finalVal, max(finalVal, glowVal * 1.3), uGlowStrength);
    }

    finalVal = clamp((finalVal - 0.5) * uContrast + 0.5 + uBrightness - 0.5, 0.0, 1.0);

    if (uGamma != 1.0 && uGamma > 0.0) {
        finalVal = pow(finalVal, 1.0 / uGamma);
    }

    vec3 col;
    if (multiCh) {
        if (uUseColormapForMultichannel != 0) {
            float blended = 0.0;
            vec3 rgb = rawPixel.rgb;
            if (uMultiChannelBlend == 0) {
                blended = dot(rgb, uChannelWeights);
            } else if (uMultiChannelBlend == 1) {
                blended = (rgb.r + rgb.g + rgb.b) / 3.0;
            } else if (uMultiChannelBlend == 2) {
                blended = max(rgb.r, max(rgb.g, rgb.b));
            } else if (uMultiChannelBlend == 3) {
                blended = min(rgb.r, min(rgb.g, rgb.b));
            } else if (uMultiChannelBlend == 4) {
                blended = rgb.r;
            } else if (uMultiChannelBlend == 5) {
                blended = rgb.g;
            } else if (uMultiChannelBlend == 6) {
                blended = rgb.b;
            }
            float cmapT = applyColormapDeformation(blended);
            switch (uColormapMode) {
                case 1: col = viridis(cmapT); break;
                case 2: col = magma(cmapT); break;
                case 3: col = inferno(cmapT); break;
                case 4: col = plasma(cmapT); break;
                case 5: col = grayscale(cmapT); break;
                case 6: col = grayscaleInv(cmapT); break;
                case 7: col = jet(cmapT); break;
                default: col = texture(uColormapTex, cmapT).rgb; break;
            }
            col = applyHueSatShift(col);
        } else {
            col = rawPixel.rgb;
            col.r = applyColormapDeformation(col.r);
            col.g = applyColormapDeformation(col.g);
            col.b = applyColormapDeformation(col.b);
            col = applyHueSatShift(col);
        }
        col = clamp((col - 0.5) * uContrast + 0.5 + uBrightness - 0.5, vec3(0.0), vec3(1.0));
        if (uGamma != 1.0 && uGamma > 0.0) {
            col = pow(col, vec3(1.0 / uGamma));
        }
    } else {
        float cmapT = applyColormapDeformation(finalVal);
        switch (uColormapMode) {
            case 1: col = viridis(cmapT); break;
            case 2: col = magma(cmapT); break;
            case 3: col = inferno(cmapT); break;
            case 4: col = plasma(cmapT); break;
            case 5: col = grayscale(cmapT); break;
            case 6: col = grayscaleInv(cmapT); break;
            case 7: col = jet(cmapT); break;
            default: col = texture(uColormapTex, cmapT).rgb; break;
        }
        col = applyHueSatShift(col);
    }

    if (uEdgeStrength > 0.0) {
        col = mix(col, vec3(1.0), edgeVal * uEdgeStrength);
    }

    if (uShowGrid != 0 && uGridW > 0 && uGridH > 0) {
        int spacingX = 1;
        int spacingY = 1;
        if (uGridSpacingMode == 1) {
            spacingX = uGridCustomSpacing;
            spacingY = uGridCustomSpacing;
        }

        float gW = float(uGridW) / float(spacingX);
        float gH = float(uGridH) / float(spacingY);
        vec2 cellUV = uv * vec2(gW, gH);
        vec2 gridDist = abs(fract(cellUV) - 0.5);
        float lineW = uGridLineThickness * 0.5 * uZoom;
        float grid = 1.0 - smoothstep(0.48 - 0.02/lineW, 0.5, min(gridDist.x, gridDist.y));
        col = mix(col, uGridLineColor, grid * uGridOpacity);

        if (uGridMajorLines != 0 && uGridMajorEvery > 1) {
            float mjW = float(uGridW) / float(spacingX * uGridMajorEvery);
            float mjH = float(uGridH) / float(spacingY * uGridMajorEvery);
            vec2 mjCellUV = uv * vec2(mjW, mjH);
            vec2 mjDist = abs(fract(mjCellUV) - 0.5);
            float mjLineW = uGridLineThickness * 1.5 * uZoom;
            float mjGrid = 1.0 - smoothstep(0.48 - 0.03/mjLineW, 0.5, min(mjDist.x, mjDist.y));
            col = mix(col, uGridLineColor * 0.7, mjGrid * uGridMajorOpacity);
        }
    }

    if (uInvertColors != 0) {
        col = vec3(1.0) - col;
    }

    if (uShowBoundary != 0) {
        float bScale = uBoundaryThickness / (float(min(uGridW, uGridH)) * uZoom);
        float dL = abs(uv.x);
        float dR = abs(uv.x - 1.0);
        float dT = abs(uv.y);
        float dB = abs(uv.y - 1.0);
        float minD = min(min(dL, dR), min(dT, dB));
        
        float line = 1.0 - smoothstep(0.0, bScale, minD);
        
        if (uBoundaryStyle == 1 || uBoundaryStyle == 2) {
            float dashScale = uBoundaryDashLength / float(min(uGridW, uGridH));
            float edgePos = 0.0;
            if (dL <= bScale || dR <= bScale) edgePos = uv.y;
            else edgePos = uv.x;
            float animOffset = uBoundaryAnimate != 0 ? uTime * 0.1 : 0.0;
            float dashPattern = mod(edgePos / dashScale + animOffset, 1.0);
            float dashThreshold = (uBoundaryStyle == 1) ? 0.5 : 0.7;
            line *= step(dashPattern, dashThreshold);
        }
        
        if (uBoundaryStyle == 3) {
            float innerD = minD - bScale * 0.5;
            float innerLine = 1.0 - smoothstep(0.0, bScale * 0.3, innerD);
            line = max(line, innerLine * 0.7);
        }
        
        if (uBoundaryStyle == 4) {
            float glow = exp(-minD * float(min(uGridW, uGridH)) * uZoom * 0.5);
            line = max(line, glow * 0.5);
        }
        
        col = mix(col, uBoundaryColor, line * uBoundaryOpacity);
    }

    fragColor = vec4(col, 1.0);
}
