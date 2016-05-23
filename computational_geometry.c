/****************************************************************************
 *                              ArtraCFD                                    *
 *                          <By Huangrui Mo>                                *
 * Copyright (C) Huangrui Mo <huangrui.mo@gmail.com>                        *
 * This file is part of ArtraCFD.                                           *
 * ArtraCFD is free software: you can redistribute it and/or modify it      *
 * under the terms of the GNU General Public License as published by        *
 * the Free Software Foundation, either version 3 of the License, or        *
 * (at your option) any later version.                                      *
 ****************************************************************************/
/****************************************************************************
 * Required Header Files
 ****************************************************************************/
#include "computational_geometry.h"
#include <stdio.h> /* standard library for input and output */
#include <stdlib.h> /* dynamic memory allocation and exit */
#include <math.h> /* common mathematical functions */
#include <float.h> /* size of floating point values */
#include "cfd_commons.h"
#include "commons.h"
/****************************************************************************
 * Static Function Declarations
 ****************************************************************************/
static int AddVertex(const Real [restrict], Polyhedron *);
static int FindEdge(const int, const int, const Polyhedron *);
static void ComputeParametersSphere(const int, Polyhedron *);
static void ComputeParametersPolyhedron(const int, Polyhedron *);
static void TransformVertex(const Real [restrict], const Real [restrict], 
        const Real [restrict][DIMS], const Real [restrict], Real [restrict]);
static void TransformNormal(const Real [restrict][DIMS], Real [restrict]);
static Real TransformInertia(Real [restrict][DIMS], const Real [restrict]);
/****************************************************************************
 * Function definitions
 ****************************************************************************/
void ConvertPolyhedron(Polyhedron *poly)
{
    /* allocate memory, assume vertN = faceN */
    AllocatePolyhedronMemory(poly->faceN, poly->faceN, poly);
    /* convert representation */
    for (int n = 0; n < poly->faceN; ++n) {
        poly->f[n][0] = AddVertex(poly->facet[n].v0, poly);
        poly->f[n][1] = AddVertex(poly->facet[n].v1, poly);
        poly->f[n][2] = AddVertex(poly->facet[n].v2, poly);
        AddEdge(poly->f[n][0], poly->f[n][1], n, poly); 
        AddEdge(poly->f[n][1], poly->f[n][2], n, poly); 
        AddEdge(poly->f[n][2], poly->f[n][0], n, poly); 
    }
    /* adjust the memory allocation */
    RetrieveStorage(poly->facet);
    poly->facet = NULL;
    poly->v = realloc(poly->v, poly->vertN * sizeof(*poly->v));
    poly->Nv = realloc(poly->Nv, poly->vertN * sizeof(*poly->Nv));
    return;
}
void AllocatePolyhedronMemory(const int vertN, const int faceN, Polyhedron *poly)
{
    const int edgeN = (int)(1.5 * faceN + 0.5); /* edgeN = faceN * 3 / 2 */
    poly->f = AssignStorage(faceN * sizeof(*poly->f));
    poly->Nf = AssignStorage(faceN * sizeof(*poly->Nf));
    poly->e = AssignStorage(edgeN * sizeof(*poly->e));
    poly->Ne = AssignStorage(edgeN * sizeof(*poly->Ne));
    poly->v = AssignStorage(vertN * sizeof(*poly->v));
    poly->Nv = AssignStorage(vertN * sizeof(*poly->Nv));
}
static int AddVertex(const Real v[restrict], Polyhedron *poly)
{
    /* search the vertex list, if already exist, return the index */
    for (int n = 0; n < poly->vertN; ++n) {
        if (EqualReal(v[X], poly->v[n][X]) && EqualReal(v[Y], poly->v[n][Y]) && 
                EqualReal(v[Z], poly->v[n][Z])) {
            return n;
        }
    }
    /* otherwise, add to the vertex list */
    poly->v[poly->vertN][X] = v[X];
    poly->v[poly->vertN][Y] = v[Y];
    poly->v[poly->vertN][Z] = v[Z];
    ++(poly->vertN); /* increase pointer */
    return (poly->vertN - 1); /* return index */
}
void AddEdge(const int v0, const int v1, const int f, Polyhedron *poly)
{
    /* search the edge list, if already exist, add the second face index */
    for (int n = 0; n < poly->edgeN; ++n) {
        if (((v0 == poly->e[n][0]) && (v1 == poly->e[n][1])) ||
                ((v1 == poly->e[n][0]) && (v0 == poly->e[n][1]))) {
            poly->e[n][3] = f;
            return;
        }
    }
    /* otherwise, add to the edge list */
    poly->e[poly->edgeN][0] = v0;
    poly->e[poly->edgeN][1] = v1;
    poly->e[poly->edgeN][2] = f;
    ++(poly->edgeN); /* increase pointer */
    return;
}
static int FindEdge(const int v0, const int v1, const Polyhedron *poly)
{
    /* search the edge list and return the edge index */
    for (int n = 0; n < poly->edgeN; ++n) {
        if (((v0 == poly->e[n][0]) && (v1 == poly->e[n][1])) ||
                ((v1 == poly->e[n][0]) && (v0 == poly->e[n][1]))) {
            return n;
        }
    }
    /* otherwise, something is wrong */
    return 0;
}
void Transformation(const Real scale[restrict], const Real angle[restrict],
        const Real offset[restrict], Polyhedron *poly)
{
    const RealVec Sin = {sin(angle[X]), sin(angle[Y]), sin(angle[Z])};
    const RealVec Cos = {cos(angle[X]), cos(angle[Y]), cos(angle[Z])};
    const Real rotate[DIMS][DIMS] = { /* rotation matrix */
        {Cos[Y]*Cos[Z], Cos[X]*Sin[Z]+Sin[X]*Sin[Y]*Cos[Z], Sin[X]*Sin[Z]-Cos[X]*Sin[Y]*Cos[Z]},
        {-Cos[Y]*Sin[Z], Cos[X]*Cos[Z]-Sin[X]*Sin[Y]*Sin[Z], Sin[X]*Cos[Z]+Cos[X]*Sin[Y]*Sin[Z]},
        {Sin[Y], -Sin[X]*Cos[Y], Cos[X]*Cos[Y]}};
    const Real invrot[DIMS][DIMS] = { /* inverse rotation matrix */
        {rotate[0][0], rotate[1][0], rotate[2][0]},
        {rotate[0][1], rotate[1][1], rotate[2][1]},
        {rotate[0][2], rotate[1][2], rotate[2][2]}};
    const Real num = 1.0 / sqrt(2.0);
    const Real axe[6][DIMS] = { /* direction vector of axis xx, yy, zz, xy, yz, zx */
        {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}, 
        {num, num, 0.0}, {0.0, num, num}, {num, 0.0, num}};
    RealVec axis = {0.0}; /* direction vector of axis in rotated frame */
    Real I[6] = {0.0}; /* inertia tensor after rotation */
    /* transforming vertex */
    for (int n = 0; n < poly->vertN; ++n) {
        TransformVertex(poly->O, scale, rotate, offset, poly->v[n]);
    }
    /* transforming bounding box */
    for (int n = 0; n < LIMIT; ++n) {
        RealVec p = {poly->box[X][n], poly->box[Y][n], poly->box[Z][n]};
        TransformVertex(poly->O, scale, rotate, offset, p);
        poly->box[X][n] = p[X];
        poly->box[Y][n] = p[Y];
        poly->box[Z][n] = p[Z];
    }
    /* transforming normal assuming pure rotation and translation */
    for (int n = 0; n < poly->faceN; ++n) {
        TransformNormal(rotate, poly->Nf[n]);
    }
    for (int n = 0; n < poly->edgeN; ++n) {
        TransformNormal(rotate, poly->Ne[n]);
    }
    for (int n = 0; n < poly->vertN; ++n) {
        TransformNormal(rotate, poly->Nv[n]);
    }
    /* transform inertial tensor */
    for (int n = 0; n < 6; ++n) {
        axis[X] = Dot(invrot[X], axe[n]);
        axis[Y] = Dot(invrot[Y], axe[n]);
        axis[Z] = Dot(invrot[Z], axe[n]);
        I[n] = TransformInertia(poly->I, axis);
    }
    poly->I[X][X] = I[0];  poly->I[X][Y] = -I[3]; poly->I[X][Z] = -I[5];
    poly->I[Y][X] = -I[3]; poly->I[Y][Y] = I[1];  poly->I[Y][Z] = -I[4];
    poly->I[Z][X] = -I[5]; poly->I[Z][Y] = -I[4]; poly->I[Z][Z] = I[2];
    /* centroid should be transformed at last */
    poly->O[X] = poly->O[X] + offset[X];
    poly->O[Y] = poly->O[Y] + offset[Y];
    poly->O[Z] = poly->O[Z] + offset[Z];
    return;
}
static void TransformVertex(const Real O[restrict], const Real scale[restrict], 
        const Real rotate[restrict][DIMS], const Real offset[restrict], Real v[restrict])
{
    /* translate reference frame to a parallel frame at centroid */
    v[X] = v[X] - O[X];
    v[Y] = v[Y] - O[Y];
    v[Z] = v[Z] - O[Z];
    /* scale */
    v[X] = v[X] * scale[X];
    v[Y] = v[Y] * scale[Y];
    v[Z] = v[Z] * scale[Z];
    /* rotate */
    const RealVec tmp = {v[X], v[Y], v[Z]};
    v[X] = Dot(rotate[X], tmp);
    v[Y] = Dot(rotate[Y], tmp);
    v[Z] = Dot(rotate[Z], tmp);
    /* translate with offset and then translate reference back to origin */
    v[X] = v[X] + offset[X] + O[X];
    v[Y] = v[Y] + offset[Y] + O[Y];
    v[Z] = v[Z] + offset[Z] + O[Z];
    return;
}
static void TransformNormal(const Real matrix[restrict][DIMS], Real N[restrict])
{
    const RealVec tmp = {N[X], N[Y], N[Z]};
    N[X] = Dot(matrix[X], tmp);
    N[Y] = Dot(matrix[Y], tmp);
    N[Z] = Dot(matrix[Z], tmp);
    /* normalization is needed if anisotropic transformation happens */
    return;
}
static Real TransformInertia(Real I[restrict][DIMS], const Real axis[restrict])
{
    return I[X][X] * axis[X] * axis[X] + I[Y][Y] * axis[Y] * axis[Y] + I[Z][Z] * axis[Z] * axis[Z] +
        2.0 * I[X][Y] * axis[X] * axis[Y] + 2.0 * I[Y][Z] * axis[Y] * axis[Z] + 2.0 * I[Z][X] * axis[Z] * axis[X];
}
void ComputeGeometryParameters(const int collapse, Geometry *geo)
{
    for (int n = 0; n < geo->sphereN; ++n) {
        ComputeParametersSphere(collapse, geo->poly + n);
    }
    for (int n = geo->sphereN; n < geo->totalN; ++n) {
        ComputeParametersPolyhedron(collapse, geo->poly + n);
    }
    return;
}
/*
 * A bounding box and a bounding sphere are both used as bounding containers
 * to enclose a finite geometric object. Meanwhile, triangulated polyhedrons
 * and analytical spheres are unified by the using of bounding container,
 * since an analytical sphere is the bounding sphere of itself. Moreover,
 * a polyhedron with a unit length thickness is used to represent a polygon
 * with the same cross-section shape.
 */
static void ComputeParametersSphere(const int collapse, Polyhedron *poly)
{
    const Real pi = 3.14159265359;
    /* bounding box */
    for (int s = 0; s < DIMS; ++s) {
        poly->box[s][MIN] = poly->O[s] - poly->r;
        poly->box[s][MAX] = poly->O[s] + poly->r;
    }
    /* geometric property */
    Real num = 0.4 * poly->r * poly->r;
    if (COLLAPSEN == collapse) { /* no space dimension collapsed */
        poly->area = 4.0 * pi * poly->r * poly->r; /* area of a sphere */
        poly->volume = 4.0 * pi * poly->r * poly->r * poly->r / 3.0; /* volume of a sphere */
    } else {
        poly->area = 2.0 * pi * poly->r; /* side area of a unit thickness cylinder */
        poly->volume = pi * poly->r * poly->r; /* volume of a unit thickness cylinder */
        num = 0.5 * poly->r * poly->r;
    }
    num = num * poly->volume;
    poly->I[X][X] = num;  poly->I[X][Y] = 0;    poly->I[X][Z] = 0;
    poly->I[Y][X] = 0;    poly->I[Y][Y] = num;  poly->I[Y][Z] = 0;
    poly->I[Z][X] = 0;    poly->I[Z][Y] = 0;    poly->I[Z][Z] = num;
    return;
}
static void ComputeParametersPolyhedron(const int collapse, Polyhedron *poly)
{
    /* initialize parameters */
    RealVec v0 = {0.0}; /* vertices */
    RealVec v1 = {0.0};
    RealVec v2 = {0.0};
    RealVec e01 = {0.0}; /* edges */
    RealVec e02 = {0.0};
    RealVec Nf = {0.0}; /* outward normal */
    RealVec Angle = {0.0}; /* internal angle */
    RealVec O = {0.0}; /* centroid */
    Real area = 0.0; /* area */
    Real volume = 0.0; /* volume */
    Real I[6] = {0.0}; /* inertia integration xx, yy, zz, xy, yz, zx */
    RealVec tmp = {0.0}; /* temporary */
    Real f[DIMS][DIMS] = {{0.0}}; /* temporary */
    Real g[DIMS][DIMS] = {{0.0}}; /* temporary */
    Real box[LIMIT][DIMS] = {{0.0}}; /* bounding box */
    for (int s = 0; s < DIMS; ++s) {
        box[MIN][s] = FLT_MAX;
        box[MAX][s] = FLT_MIN;
    }
    /* initialize vertices normal */
    for (int n = 0; n < poly->vertN; ++n) {
        for (int s = 0; s < DIMS; ++s) {
            poly->Nv[n][s] = 0.0;
        }
    }
    /* bounding box */
    for (int n = 0; n < poly->vertN; ++n) {
        for (int s = 0; s < DIMS; ++s) {
            box[MIN][s] = MinReal(box[MIN][s], poly->v[n][s]);
            box[MAX][s] = MaxReal(box[MAX][s], poly->v[n][s]);
        }
    }
    /*
     * Gelder, A. V. (1995). Efficient computation of polygon area and
     * polyhedron volume. Graphics Gems V.
     * Eberly, David. "Polyhedral mass properties (revisited)." Geometric
     * Tools, LLC, Tech. Rep (2002).
     */
    for (int n = 0; n < poly->faceN; ++n) {
        BuildTriangle(n, poly, v0, v1, v2, e01, e02);
        /* outward normal vector */
        Cross(e01, e02, Nf);
        /* temporary values */
        for (int s = 0; s < DIMS; ++s) {
            tmp[0] = v0[s] + v1[s];
            f[0][s] = tmp[0] + v2[s];
            tmp[1] = v0[s] * v0[s];
            tmp[2] = tmp[1] + v1[s] * tmp[0];
            f[1][s] = tmp[2] + v2[s] * f[0][s];
            f[2][s] = v0[s] * tmp[1] + v1[s] * tmp[2] + v2[s] * f[1][s];
            g[0][s] = f[1][s] + v0[s] * (f[0][s] + v0[s]);
            g[1][s] = f[1][s] + v1[s] * (f[0][s] + v1[s]);
            g[2][s] = f[1][s] + v2[s] * (f[0][s] + v2[s]);
        }
        /* integration */
        area = area + Norm(Nf);
        volume = volume + Nf[X] * f[0][X];
        O[X] = O[X] + Nf[X] * f[1][X];
        O[Y] = O[Y] + Nf[Y] * f[1][Y];
        O[Z] = O[Z] + Nf[Z] * f[1][Z];
        I[0] = I[0] + Nf[X] * f[2][X];
        I[1] = I[1] + Nf[Y] * f[2][Y];
        I[2] = I[2] + Nf[Z] * f[2][Z];
        I[3] = I[3] + Nf[X] * (v0[Y] * g[0][X] + v1[Y] * g[1][X] + v2[Y] * g[2][X]);
        I[4] = I[4] + Nf[Y] * (v0[Z] * g[0][Y] + v1[Z] * g[1][Y] + v2[Z] * g[2][Y]);
        I[5] = I[5] + Nf[Z] * (v0[X] * g[0][Z] + v1[X] * g[1][Z] + v2[X] * g[2][Z]);
        /* unit normal */
        Normalize(DIMS, Norm(Nf), Nf);
        /*
         * Refine vertices normal by corresponding angles
         * Baerentzen, J. A., & Aanaes, H. (2005). Signed distance computation
         * using the angle weighted pseudonormal. Visualization and Computer
         * Graphics, IEEE Transactions on, 11(3), 243-253.
         */
        /* calculate internal angles by the law of cosines */
        const RealVec e12 = {v2[X] - v1[X], v2[Y] - v1[Y], v2[Z] - v1[Z]};
        Angle[0] = acos((Dot(e01, e01) + Dot(e02, e02) - Dot(e12, e12)) / 
                (2.0 * sqrt(Dot(e01, e01) * Dot(e02, e02))));
        Angle[1] = acos((Dot(e01, e01) + Dot(e12, e12) - Dot(e02, e02)) / 
                (2.0 * sqrt(Dot(e01, e01) * Dot(e12, e12))));
        Angle[2] = 3.14159265359 - Angle[0] - Angle[1];
        for (int s = 0; s < DIMS; ++s) {
            poly->Nv[poly->f[n][0]][s] = poly->Nv[poly->f[n][0]][s] + Angle[0] * Nf[s];
            poly->Nv[poly->f[n][1]][s] = poly->Nv[poly->f[n][1]][s] + Angle[1] * Nf[s];
            poly->Nv[poly->f[n][2]][s] = poly->Nv[poly->f[n][2]][s] + Angle[2] * Nf[s];
        }
        /* assign face normal */
        for (int s = 0; s < DIMS; ++s) {
            poly->Nf[n][s] = Nf[s];
        }
    }
    /* rectify final integration */
    area = 0.5 * area;
    volume = volume / 6.0;
    O[X] = O[X] / 24.0;
    O[Y] = O[Y] / 24.0;
    O[Z] = O[Z] / 24.0;
    I[0] = I[0] / 60.0;
    I[1] = I[1] / 60.0;
    I[2] = I[2] / 60.0;
    I[3] = I[3] / 120.0;
    I[4] = I[4] / 120.0;
    I[5] = I[5] / 120.0;
    /* assign to polyhedron */
    if (COLLAPSEN == collapse) { /* no space dimension collapsed */
        poly->area = area;
    } else {
        poly->area = area - 2.0 * volume; /* change to side area of a unit thickness polygon */
    }
    poly->volume = volume;
    poly->O[X] = O[X];
    poly->O[Y] = O[Y];
    poly->O[Z] = O[Z];
    /* inertia relative to centroid */
    poly->I[X][X] = I[1] + I[2] - volume * (O[Y] * O[Y] + O[Z] * O[Z]);
    poly->I[X][Y] = -I[3] + volume * O[X] * O[Y];
    poly->I[X][Z] = -I[5] + volume * O[Z] * O[X];
    poly->I[Y][X] = poly->I[X][Y];
    poly->I[Y][Y] = I[0] + I[2] - volume * (O[Z] * O[Z] + O[X] * O[X]);
    poly->I[Y][Z] = -I[4] + volume * O[Y] * O[Z];
    poly->I[Z][X] = poly->I[X][Z];
    poly->I[Z][Y] = poly->I[Y][Z];
    poly->I[Z][Z] = I[0] + I[1] - volume * (O[X] * O[X] + O[Y] * O[Y]);
    for (int s = 0; s < DIMS; ++s) {
        poly->box[s][MIN] = box[MIN][s];
        poly->box[s][MAX] = box[MAX][s];
    }
    /* a radius for estimating maximum velocity */
    poly->r = Dist(box[MIN], box[MAX]);
    /* normalize vertices normal */
    for (int n = 0; n < poly->vertN; ++n) {
        Normalize(DIMS, Norm(poly->Nv[n]), poly->Nv[n]);
    }
    /* compute edge normal */
    for (int n = 0; n < poly->edgeN; ++n) {
        for (int s = 0; s < DIMS; ++s) {
            poly->Ne[n][s] = poly->Nf[poly->e[n][2]][s] + poly->Nf[poly->e[n][3]][s];
        }
        Normalize(DIMS, Norm(poly->Ne[n]), poly->Ne[n]);
    }
    return;
}
void BuildTriangle(const int faceID, const Polyhedron *poly, Real v0[restrict], 
        Real v1[restrict], Real v2[restrict], Real e01[restrict], Real e02[restrict])
{
    for (int s = 0; s < DIMS; ++s) {
        /* vertices */
        v0[s] = poly->v[poly->f[faceID][0]][s];
        v1[s] = poly->v[poly->f[faceID][1]][s];
        v2[s] = poly->v[poly->f[faceID][2]][s];
        /* edge vectors */
        e01[s] = v1[s] - v0[s];
        e02[s] = v2[s] - v0[s];
    }
    return;
}
int PointInPolyhedron(const Real p[restrict], const Polyhedron *poly, int faceID[restrict])
{
    RealVec v0 = {0.0}; /* vertices */
    RealVec v1 = {0.0};
    RealVec v2 = {0.0};
    RealVec e01 = {0.0}; /* edges */
    RealVec e02 = {0.0};
    RealVec pi = {0.0}; /* closest point */
    RealVec N = {0.0}; /* normal of the closest point */
    /*
     * Parametric equation of triangle defined plane 
     * T(s,t) = v0 + s(v1-v0) + t(v2-v0) = v0 + s*e01 + t*e02
     * s, t: real numbers. v0, v1, v2: vertices. e01, e02: edge vectors. 
     * A point pi = T(s,t) is in the triangle T when s>=0, t>=0, and s+t<=1.
     * Further, pi is on an edge of T if one of the conditions s=0; t=0; 
     * s+t=1 is true with each condition corresponds to one edge. Each 
     * s=0, t=0; s=1, t=0; s=0, t=1 corresponds to v0, v1, and v2.
     */
    RealVec para = {0.0}; /* parametric coordinates */
    Real distSquare = 0.0; /* store computed squared distance */
    Real distSquareMin = FLT_MAX; /* store minimum squared distance */
    int fid = 0; /* closest face identifier */
    for (int n = 0; n < poly->faceN; ++n) {
        BuildTriangle(n, poly, v0, v1, v2, e01, e02);
        distSquare = PointTriangleDistance(p, v0, e01, e02, para);
        if (distSquareMin > distSquare) {
            distSquareMin = distSquare;
            fid = n;
        }
    }
    *faceID = fid;
    ComputeIntersection(p, fid, poly, pi, N);
    pi[X] = p[X] - pi[X];
    pi[Y] = p[Y] - pi[Y];
    pi[Z] = p[Z] - pi[Z];
    if (0.0 < Dot(pi, N)) {
        /* outside polyhedron */
        return 0;
    } else {
        /* inside or on polyhedron */
        return 1;
    }
}
/*
 * Eberly, D. (1999). Distance between point and triangle in 3D.
 * http://www.geometrictools.com/Documentation/DistancePoint3Triangle3.pdf
 */
Real PointTriangleDistance(const Real p[restrict], const Real v0[restrict], const Real e01[restrict], 
        const Real e02[restrict], Real para[restrict])
{
    const RealVec D = {v0[X] - p[X], v0[Y] - p[Y], v0[Z] - p[Z]};
    const Real a = Dot(e01, e01);
    const Real b = Dot(e01, e02);
    const Real c = Dot(e02, e02);
    const Real d = Dot(e01, D);
    const Real e = Dot(e02, D);
    const Real f = Dot(D, D);
    const Real det = a * c - b * b;
    const Real zero = 0.0;
    const Real one = 1.0;
    Real s = b * e - c * d;
    Real t = b * d - a * e;
    Real distSquare = 0.0;
    if (s + t <= det) {
        if (s < zero) {
            if (t < zero) {
                /* region 4 */;
                if (d < zero) {
                    t = zero;
                    if (-d >= a) {
                        s = one;
                        distSquare = a + 2.0 * d + f;
                    } else {
                        s = -d / a;
                        distSquare = d * s + f;
                    }
                } else {
                    s = zero;
                    if (e >= zero) {
                        t = zero;
                        distSquare = f;
                    } else {
                        if (-e >= c) {
                            t = one;
                            distSquare = c + 2.0 * e + f;
                        } else {
                            t = -e / c;
                            distSquare = e * t + f;
                        }
                    }
                }
            } else {
                /* region 3 */
                s = zero;
                if (e >= zero) {
                    t = zero;
                    distSquare = f;
                } else {
                    if (-e >= c) {
                        t = one;
                        distSquare = c + 2.0 * e + f;
                    } else {
                        t = -e / c;
                        distSquare = e * t + f;
                    }
                }
            }        
        } else {
            if (t < zero) {
                /* region 5 */;
                t = zero;
                if (d >= zero) {
                    s = zero;
                    distSquare = f;
                } else {
                    if (-d >= a) {
                        s = one;
                        distSquare = a + 2.0 * d + f;
                    } else {
                        s = -d / a;
                        distSquare = d * s + f;
                    }
                }
            } else {
                /* region 0 */
                s = s / det;
                t = t / det;
                distSquare = s * (a * s + b * t + 2.0 * d) + t * (b * s + c * t + 2.0 * e) + f;
            }
        }
    } else {
        if (s < zero) {
            /* region 2 */
            const Real tmp0 = b + d;
            const Real tmp1 = c + e;
            if (tmp1 > tmp0) {
                const Real numer = tmp1 - tmp0;
                const Real denom = a - 2.0 * b + c;
                if (numer >= denom) {
                    s = one;
                    t = zero;
                    distSquare = a + 2.0 * d + f;
                } else {
                    s = numer / denom;
                    t = one - s;
                    distSquare = s * (a * s + b * t + 2.0 * d) + t * (b * s + c * t + 2.0 * e) + f;
                }
            } else {
                s = zero;
                if (tmp1 <= zero) {
                    t = one;
                    distSquare = c + 2.0 * e + f;
                } else {
                    if (e >= zero) {
                        t = zero;
                        distSquare = f;
                    } else {
                        t = -e / c;
                        distSquare = e * t + f;
                    }
                }
            }
        } else {
            if (t < zero) {
                /* region 6 */;
                const Real tmp0 = b + e;
                const Real tmp1 = a + d;
                if (tmp1 > tmp0) {
                    const Real numer = tmp1 - tmp0;
                    const Real denom = a - 2.0 * b + c;
                    if (numer >= denom) {
                        t = one;
                        s = zero;
                        distSquare = c + 2.0 * e + f;
                    } else {
                        t = numer / denom;
                        s = one - t;
                        distSquare = s * (a * s + b * t + 2.0 * d) + t * (b * s + c * t + 2.0 * e) + f;
                    }
                } else {
                    t = zero;
                    if (tmp1 <= zero) {
                        s = one;
                        distSquare = a + 2.0 * d + f;
                    } else {
                        if (d >= zero) {
                            s = zero;
                            distSquare = f;
                        } else {
                            s = -d / a;
                            distSquare = d * s + f;
                        }
                    }
                }
            } else {
                /* region 1 */
                const Real numer = c + e - b - d;
                if (numer <= zero) {
                    s = zero;
                    t = one;
                    distSquare = c + 2.0 * e + f;
                } else {
                    const Real denom = a - 2.0 * b + c;
                    if (numer >= denom) {
                        s = one;
                        t = zero;
                        distSquare = a + 2.0 * d + f;
                    } else {
                        s = numer / denom;
                        t = one - s;
                        distSquare = s * (a * s + b * t + 2.0 * d) + t * (b * s + c * t + 2.0 * e) + f;
                    }
                }
            }
        }
    }
    para[0] = one - s - t;
    para[1] = s;
    para[2] = t;
    /* account for numerical round-off error */
    if (zero > distSquare) {
        distSquare = zero;
    }
    return distSquare;
}
Real ComputeIntersection(const Real p[restrict], const int faceID, const Polyhedron *poly,
        Real pi[restrict], Real N[restrict])
{
    RealVec v0 = {0.0}; /* vertices */
    RealVec v1 = {0.0};
    RealVec v2 = {0.0};
    RealVec e01 = {0.0}; /* edges */
    RealVec e02 = {0.0};
    RealVec para = {0.0}; /* parametric coordinates */
    const Real zero = 0.0;
    const Real one = 1.0;
    const IntVec v = {poly->f[faceID][0], poly->f[faceID][1], poly->f[faceID][2]}; /* vertex index in vertex list */
    int e = 0; /* edge index in edge list */
    BuildTriangle(faceID, poly, v0, v1, v2, e01, e02);
    const Real distSquare = PointTriangleDistance(p, v0, e01, e02, para);
    if (EqualReal(para[1], zero)) {
        if (EqualReal(para[2], zero)) {
            /* vertex 0 */
            for (int s = 0; s < DIMS; ++s) {
                pi[s] = v0[s];
                N[s] = poly->Nv[v[0]][s];
            }
        } else {
            if (EqualReal(para[2], one)) {
                /* vertex 2 */
                for (int s = 0; s < DIMS; ++s) {
                    pi[s] = v2[s];
                    N[s] = poly->Nv[v[2]][s];
                }
            } else {
                /* edge e02 */
                e = FindEdge(v[0], v[2], poly);
                for (int s = 0; s < DIMS; ++s) {
                    pi[s] = v0[s] + para[2] * e02[s];
                    N[s] = poly->Ne[e][s];
                }
            }
        }
    } else {
        if (EqualReal(para[1], one)) {
            /* vertex 1 */
            for (int s = 0; s < DIMS; ++s) {
                pi[s] = v1[s];
                N[s] = poly->Nv[v[1]][s];
            }
        } else {
            if (EqualReal(para[2], zero)) {
                /* edge e01 */
                e = FindEdge(v[0], v[1], poly);
                for (int s = 0; s < DIMS; ++s) {
                    pi[s] = v0[s] + para[1] * e01[s];
                    N[s] = poly->Ne[e][s];
                }
            } else {
                if (EqualReal(para[0], zero)) {
                    /* edge e12 */
                    e = FindEdge(v[1], v[2], poly);
                    for (int s = 0; s < DIMS; ++s) {
                        pi[s] = v0[s] + para[1] * e01[s] + para[2] * e02[s];
                        N[s] = poly->Ne[e][s];
                    }
                } else {
                    /* complete in the triangle */
                    for (int s = 0; s < DIMS; ++s) {
                        pi[s] = v0[s] + para[1] * e01[s] + para[2] * e02[s];
                        N[s] = poly->Nf[faceID][s];
                    }
                }
            }
        }
    }
    return distSquare;
}
/* a good practice: end file with a newline */

