/****************************************************************************
 * Numeric Scheme for Space Domain                                          *
 * Programmer: Huangrui Mo                                                  *
 * - Follow the Google's C/C++ style Guide.                                 *
 * - This file defines the numeric schemes of space domain.                 *
 ****************************************************************************/
/****************************************************************************
 * Required Header Files
 ****************************************************************************/
#include "tvd.h"
#include <stdio.h> /* standard library for input and output */
#include <math.h> /* common mathematical functions */
#include "commons.h"
/****************************************************************************
 * Static Function Declarations
 ****************************************************************************/
static Real Q(const Real x);
static int sgn(const Real x);
static Real minmod(const Real x, const Real y, const Real z);
static Real Min(const Real valueA, const Real valueB);
static Real Max(const Real valueA, const Real valueB);
static int ComputeNonviscousFlux(const Real U[], Real Fx[], Real Fy[], Real Fz[],
        const Flow *flow);
static int ComputeViscousFlux(const Real *U, Real Gx[], Real Gy[], Real Gz[], 
        const int k, const int j, const int i, const Space *space, const Flow *flow);
/****************************************************************************
 * Function definitions
 ****************************************************************************/
int TVD(Real *U, const Real *Un, const Space *space, const Partition *part, const Flow *flow)
{
    /*
     * When exchange a large bunch of data between two arrays, if there is no
     * new data generation but just data exchange and update, then the rational
     * way is to exchange the head address that they  points rather than values
     * of data entries.
     */
    return 0;
}
static int Lx(Real *U, const Real *Un, const Space *space, const Partition *part, const Flow *flow)
{
    /*
     * Decompose the nonviscous flux variables into each component
     */
    Real Fx[5] = {0, 0, 0, 0, 0};
    /*
     * Decompose the viscous flux variables into each component
     */
    Real Gx[5] = {0, 0, 0, 0, 0};
    /*
     * Define the primitive field variables.
     */
    Real rho = 0; 
    Real u = 0;
    Real v = 0;
    Real w = 0;
    Real p = 0;
    Real eT = 0;
    /*
     * Indices
     */
    int k = 0; /* loop count */
    int j = 0; /* loop count */
    int i = 0; /* loop count */
    int idx = 0; /* calculated index */
    int idxW = 0; /* index at West */
    int idxE = 0; /* index at East */
    return 0;
}
static Real Q(const Real x)
{
    const Real e = 0.01;
    if (fabs(x) >= e) {
        return fabs(x);
    }
    return (0.5 * (x * x / e + e));
}
static int sgn(const Real x)
{
    if (x > 0) {
        return 1;
    }
    if (x < 0) {
        return -1;
    }
    return 0;
}
static Real minmod(const Real x, const Real y, const Real z)
{
    if ((x * y <= 0) || (x * z <= 0)) {
        return 0;
    }
    return (sgn(x) * Min(fabs(x), Min(fabs(y), fabs(z))));
}
static Real Min(const Real valueA, const Real valueB)
{
    if (valueA < valueB) {
        return valueA;
    }
    return valueB;
}
static Real Max(const Real valueA, const Real valueB)
{
    if (valueA > valueB) {
        return valueA;
    }
    return valueB;
}
static int ComputeNonviscousFlux(const Real U[], Real Fx[], Real Fy[], Real Fz[], const Flow *flow)
{
    /*
     * Define the primitive field variables.
     */
    Real rho = U[0];
    Real u = U[1] / rho;
    Real v = U[2] / rho;
    Real w = U[3] / rho;
    Real eT = U[4] / rho;
    Real p = (flow->gamma - 1) * rho * (eT - 0.5 * (u * u + v * v + w * w));

    if (Fx != NULL) {
        Fx[0] = rho * u;
        Fx[1] = rho * u * u + p;
        Fx[2] = rho * u * v;
        Fx[3] = rho * u * w;
        Fx[4] = (rho * eT + p) * u;
    }

    if (Fy != NULL) {
        Fy[0] = rho * v;
        Fy[1] = rho * v * u;
        Fy[2] = rho * v * v + p;
        Fy[3] = rho * v * w;
        Fy[4] = (rho * eT + p) * v;
    }

    if (Fz != NULL) {
        Fz[0] = rho * w;
        Fz[1] = rho * w * u;
        Fz[2] = rho * w * v;
        Fz[3] = rho * w * w + p;
        Fz[4] = (rho * eT + p) * w;
    }
    return 0;
}
static int ComputeViscousFlux(const Real *U, Real Gx[], Real Gy[], Real Gz[], 
        const int k, const int j, const int i, const Space *space, const Flow *flow)
{
    /*
     * Define the primitive field variables.
     */
    Real rho = 0; 
    Real rho_h = 0; 
    Real u = 0;
    Real u_h = 0;
    Real v = 0;
    Real v_h = 0;
    Real w = 0;
    Real w_h = 0;
    Real eT = 0;
    Real eT_h = 0;
    Real T = 0;
    Real T_h = 0;
    /*
     * Fluid and flow properties
     */
    Real mu = 0; /* normalized dynamic viscosity */
    Real heatK = 0; /* normalized heat conductivity */
    /*
     * Auxiliary variables
     */
    Real divV = 0; /* divergence of velocity V */
    Real du_dx = 0; /* partial u partial x */
    Real du_dy = 0;
    Real du_dz = 0;
    Real dv_dx = 0; /* partial v partial x */
    Real dv_dy = 0;
    Real dv_dz = 0;
    Real dw_dx = 0; /* partial w partial x */
    Real dw_dy = 0;
    Real dw_dz = 0;
    Real dT_dx = 0; /* partial T partial x */
    Real dT_dy = 0;
    Real dT_dz = 0;
    /*
     * Indices
     */
    int idx = 0; /* linear array index math variable */
    int idxW = 0; /* index at West */
    int idxE = 0; /* index at East */
    int idxS = 0; /* index at South */
    int idxN = 0; /* index at North */
    int idxF = 0; /* index at Front */
    int idxB = 0; /* index at Back */
    /*
     * Generally the viscous terms will only be discretized by central
     * difference scheme, and the calculation will be conducted on boundary
     * nodes and interior nodes. However, for interior ghost nodes the central
     * difference scheme can't be applied because of lacking stencil. Thus,
     * they need to be identified and modified.
     */
    idx = (k * space->jMax + j) * space->iMax + i;
    idxW = (k * space->jMax + j) * space->iMax + i - 1;
    idxE = (k * space->jMax + j) * space->iMax + i + 1;
    idxS = (k * space->jMax + j - 1) * space->iMax + i;
    idxN = (k * space->jMax + j + 1) * space->iMax + i;
    idxF = ((k - 1) * space->jMax + j) * space->iMax + i;
    idxB = ((k + 1) * space->jMax + j) * space->iMax + i;
    if (space->ghostFlag[idx] == 1) { /* interior ghost used Forward or Backward scheme */
        if (space->ghostFlag[idxW] == -1) { 
            idxW = idx;
        }
        if (space->ghostFlag[idxE] == -1) {
            idxE = idx;
        }
        if (space->ghostFlag[idxS] == -1) {
            idxS = idx;
        }
        if (space->ghostFlag[idxN] == -1) {
            idxN = idx;
        }
        if (space->ghostFlag[idxF] == -1) {
            idxF = idx;
        }
        if (space->ghostFlag[idxB] == -1) {
            idxB = idx;
        }
    }
    /*
     * Now transform indices to refer field variables
     */
    idx = idx * 5;
    idxW = idxW * 5;
    idxE = idxE * 5;
    idxS = idxS * 5;
    idxN = idxN * 5;
    idxF = idxF * 5;
    idxB = idxB * 5;

    /* calculate derivatives in z direction */
    rho_h = U[idxB+0];
    u_h = U[idxB+1] / rho_h;
    v_h = U[idxB+2] / rho_h;
    w_h = U[idxB+3] / rho_h;
    eT_h = U[idxB+4] / rho_h;
    T_h = (eT_h - 0.5 * (u_h * u_h + v_h * v_h + w_h * w_h)) / flow->cv;

    rho = U[idxF+0];
    u = U[idxF+1] / rho;
    v = U[idxF+2] / rho;
    w = U[idxF+3] / rho;
    eT = U[idxF+4] / rho;
    T = (eT - 0.5 * (u * u + v * v + w * w)) / flow->cv;

    du_dz = (u_h - u) * (0.5 * space->ddz);
    dv_dz = (v_h - v) * (0.5 * space->ddz);
    dw_dz = (w_h - w) * (0.5 * space->ddz);
    dT_dz = (T_h - T) * (0.5 * space->ddz);

    /* calculate derivatives in y direction */
    rho_h = U[idxN+0];
    u_h = U[idxN+1] / rho_h;
    v_h = U[idxN+2] / rho_h;
    w_h = U[idxN+3] / rho_h;
    eT_h = U[idxN+4] / rho_h;
    T_h = (eT_h - 0.5 * (u_h * u_h + v_h * v_h + w_h * w_h)) / flow->cv;

    rho = U[idxS+0];
    u = U[idxS+1] / rho;
    v = U[idxS+2] / rho;
    w = U[idxS+3] / rho;
    eT = U[idxS+4] / rho;
    T = (eT - 0.5 * (u * u + v * v + w * w)) / flow->cv;

    du_dy = (u_h - u) * (0.5 * space->ddy);
    dv_dy = (v_h - v) * (0.5 * space->ddy);
    dw_dy = (w_h - w) * (0.5 * space->ddy);
    dT_dy = (T_h - T) * (0.5 * space->ddy);

    /* calculate derivatives in x direction */
    rho_h = U[idxE+0];
    u_h = U[idxE+1] / rho_h;
    v_h = U[idxE+2] / rho_h;
    w_h = U[idxE+3] / rho_h;
    eT_h = U[idxE+4] / rho_h;
    T_h = (eT_h - 0.5 * (u_h * u_h + v_h * v_h + w_h * w_h)) / flow->cv;

    rho = U[idxW+0];
    u = U[idxW+1] / rho;
    v = U[idxW+2] / rho;
    w = U[idxW+3] / rho;
    eT = U[idxW+4] / rho;
    T = (eT - 0.5 * (u * u + v * v + w * w)) / flow->cv;

    du_dx = (u_h - u) * (0.5 * space->ddx);
    dv_dx = (v_h - v) * (0.5 * space->ddx);
    dw_dx = (w_h - w) * (0.5 * space->ddx);
    dT_dx = (T_h - T) * (0.5 * space->ddx);

    /* regain the primitive variables in current point */
    rho = U[idx+0];
    u = U[idx+1] / rho;
    v = U[idx+2] / rho;
    w = U[idx+3] / rho;
    eT = U[idx+4] / rho;
    T = (eT - 0.5 * (u * u + v * v + w * w)) / flow->cv;

    /*
     * Calculate dynamic viscosity and heat conductivity
     */
    mu = 1.45e-6 * (pow(T * flow->refTemperature, 1.5) / (T * flow->refTemperature + 110)) / flow->refMu ;
    heatK = flow->gamma * flow->cv * mu / flow->refPr;

    divV = du_dx + dv_dy + dw_dz;

    if (Gx != NULL) {
        Gx[0] = 0;
        Gx[1] = mu * (2 * du_dx - (2/3) * divV);
        Gx[2] = mu * (du_dy + dv_dx);
        Gx[3] = mu * (du_dz + dw_dx);
        Gx[4] = heatK * dT_dx + 
            u * Gx[1] + v * Gx[2] + w * Gx[3];
    }

    if (Gy != NULL) {
        Gy[0] = 0;
        Gy[1] = mu * (dv_dx + du_dy);
        Gy[2] = mu * (2 * dv_dy - (2/3) * divV);
        Gy[3] = mu * (dv_dz + dw_dy);
        Gy[4] = heatK * dT_dy + 
            u * Gy[1] + v * Gy[2] + w * Gy[3];
    }

    if (Gz != NULL) {
        Gz[0] = 0;
        Gz[1] = mu * (dw_dx + du_dz);
        Gz[2] = mu * (dw_dy + dv_dz);
        Gz[3] = mu * (2 * dw_dz - (2/3) * divV);
        Gz[4] = heatK * dT_dz + 
            u * Gz[1] + v * Gz[2] + w * Gz[3];
    }
    return 0;
}
/* a good practice: end file with a newline */

