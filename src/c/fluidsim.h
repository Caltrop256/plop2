#ifndef FLUIDSIM_H
#define FLUIDSIM_H

#define N 75
#define ITER 1
#define IX(x, y) (y * N + x)

typedef struct FluidSim {
    f32 dt;
    f32 diff;
    f32 visc;

    f32 s[N * N];
    // f32 density[N * N];
    f32 vx[N * N];
    f32 vy[N * N];
    f32 vx0[N * N];
    f32 vy0[N * N];
} FluidSim;

void stepFluid(FluidSim *sim);

#endif
