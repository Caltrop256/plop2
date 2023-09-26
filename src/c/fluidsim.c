#include "main.h"
#include "fluidsim.h"
#include "elements/elements.h"
#include "walloc.h"

inline f32 unsafeX(f32 *a, u8 x, u8 y) {
    if(x < 0 || x >= N) return 0.0f;
    return a[y * N + x];
}

inline f32 unsafeY(f32 *a, u8 x, u8 y) {
    if(y < 0 || y >= N) return 0.0f;
    return a[y * N + x];
}

inline f32 unsafeXY(f32 *a, u8 x, u8 y) {
    if(x < 0 || x >= N || y < 0 || y >= N) return 0.0f;
    return a[y * N + x];
}

void lin_solve(f32 *x, f32 *x0, f32 a, f32 c) {
    f32 cRecip = 1.0 / c;
    for (int k = 0; k < ITER; k++) {
        for (int j = 0; j < N; j++) {
            for (int i = 0; i < N; i++) {
                x[IX(i, j)] = (x0[IX(i, j)] + a * (unsafeX(x, i + 1, j) + unsafeX(x, i - 1, j) + unsafeY(x, i, j + 1) + unsafeY(x, i, j - 1))) * cRecip;
            }
        }
    }
}

void diffuse(f32 *x, f32 *x0, f32 diff, f32 dt) {
    f32 a = dt * diff * (N) * (N);
    lin_solve(x, x0, a, 1 + 4 * a);
}

void project(f32 *velocX, f32 *velocY, f32 *p, f32 *div) {
    for (int j = 0; j < N; j++) {
        for (int i = 0; i < N; i++) {
            div[IX(i, j)] = -0.5f * (unsafeX(velocX, i + 1, j) - unsafeX(velocX, i - 1, j) + unsafeY(velocY, i, j + 1) - unsafeY(velocY, i, j - 1)) / N;
            p[IX(i, j)] = 0;
        }
    }

    lin_solve(p, div, 1, 4);

    for (int j = 0; j < N; j++) {
        for (int i = 0; i < N; i++) {
            velocX[IX(i, j)] -= 0.5f * (unsafeX(p, i + 1, j) - unsafeX(p, i - 1, j)) * N;
            velocY[IX(i, j)] -= 0.5f * (unsafeY(p, i, j + 1) - unsafeY(p, i, j - 1)) * N;
        }
    }
}

void advect(f32 *d, f32 *d0, f32 *velocX, f32 *velocY, f32 dt) {
    f32 i0, i1, j0, j1;

    f32 dtx = dt * (N);
    f32 dty = dt * (N);

    f32 s0, s1, t0, t1;
    f32 tmp1, tmp2, x, y;

    f32 NF32 = N;
    f32 iF32, jF32;
    int i, j;

    for (j = 0, jF32 = 0; j < N; j++, jF32++) { 
        for (i = 0, iF32 = 0; i < N; i++, iF32++) {
            tmp1 = dtx * velocX[IX(i, j)];
            tmp2 = dty * velocY[IX(i, j)];
            x    = iF32 - tmp1; 
            y    = jF32 - tmp2;

            if (x < 0.5f) x = 0.5f; 
            if (x > NF32 + 0.5f) x = NF32 + 0.5f; 
            i0 = FLOOR(x); 
            i1 = i0 + 1.0f;
            if (y < 0.5f) y = 0.5f; 
            if (y > NF32 + 0.5f) y = NF32 + 0.5f; 
            j0 = FLOOR(y);
            j1 = j0 + 1.0f; 

            s1 = x - i0; 
            s0 = 1.0f - s1; 
            t1 = y - j0; 
            t0 = 1.0f - t1;

            int i0i = (int)(i0);
            int i1i = (int)(i1);
            int j0i = (int)(j0);
            int j1i = (int)(j1);

            d[IX(i, j)] = 
                s0 * (t0 * unsafeXY(d0, i0i, j0i) + t1 * unsafeXY(d0, i0i, j1i)) +
                s1 * (t0 * unsafeXY(d0, i1i, j0i) + t1 * unsafeXY(d0, i1i, j1i));
        }
    }
}

void stepFluid(FluidSim *fluid) {    
    diffuse(fluid->vx0, fluid->vx, fluid->visc, fluid->dt);
    diffuse(fluid->vy0, fluid->vy, fluid->visc, fluid->dt);

    project(fluid->vx0, fluid->vy0, fluid->vx, fluid->vy);

    advect(fluid->vx, fluid->vx0, fluid->vx0, fluid->vy0, fluid->dt);
    advect(fluid->vy, fluid->vy0, fluid->vx0, fluid->vy0, fluid->dt);

    project(fluid->vx, fluid->vy, fluid->vx0, fluid->vy0);

    // diffuse(fluid->s, fluid->density, fluid->diff, fluid->dt);
    // advect(fluid->density, fluid->s, fluid->vx, fluid->vy, fluid->dt);
}
