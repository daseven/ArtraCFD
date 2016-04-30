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
#include "stl.h"
#include <stdio.h> /* standard library for input and output */
#include <string.h> /* manipulating strings */
#include "commons.h"
/****************************************************************************
 * Static Function Declarations
 ****************************************************************************/
/****************************************************************************
 * Function definitions
 ****************************************************************************/
int ReadStlFile(const char *fileName, Polyhedron *poly)
{
    StlString header = {'\0'};
    StlLongInt facetN = 0;
    StlInt attributeCount = 0;
    StlReal facetData = 0.0;
    FILE *filePointer = fopen(fileName, "rb");
    if (NULL == filePointer) {
        FatalError("failed to open STL file...");
    }
    fread(header, sizeof(char), sizeof(StlString), filePointer);
    fread(&facetN, sizeof(StlLongInt), 1, filePointer);
    poly->facetN = facetN;
    poly->facet = AssignStorage(poly->facetN, "Facet");
    for (int n = 0; n < facetN; ++n) {
        fread(&facetData, sizeof(StlReal), 1, filePointer);
        poly->facet[n].N[X] = facetData;
        fread(&facetData, sizeof(StlReal), 1, filePointer);
        poly->facet[n].N[Y] = facetData;
        fread(&facetData, sizeof(StlReal), 1, filePointer);
        poly->facet[n].N[Z] = facetData;
        fread(&facetData, sizeof(StlReal), 1, filePointer);
        poly->facet[n].P1[X] = facetData;
        fread(&facetData, sizeof(StlReal), 1, filePointer);
        poly->facet[n].P1[Y] = facetData;
        fread(&facetData, sizeof(StlReal), 1, filePointer);
        poly->facet[n].P1[Z] = facetData;
        fread(&facetData, sizeof(StlReal), 1, filePointer);
        poly->facet[n].P2[X] = facetData;
        fread(&facetData, sizeof(StlReal), 1, filePointer);
        poly->facet[n].P2[Y] = facetData;
        fread(&facetData, sizeof(StlReal), 1, filePointer);
        poly->facet[n].P2[Z] = facetData;
        fread(&facetData, sizeof(StlReal), 1, filePointer);
        poly->facet[n].P3[X] = facetData;
        fread(&facetData, sizeof(StlReal), 1, filePointer);
        poly->facet[n].P3[Y] = facetData;
        fread(&facetData, sizeof(StlReal), 1, filePointer);
        poly->facet[n].P3[Z] = facetData;
        fread(&attributeCount, sizeof(StlInt), 1, filePointer);
    }
    fclose(filePointer); /* close current opened file */
    return 0;
}
int WriteStlFile(const char *fileName, const Polyhedron *poly)
{
    StlString header = {'\0'};
    StlLongInt facetN = 0;
    StlInt attributeCount = 0;
    StlReal facetData = 0.0;
    FILE *filePointer = fopen(fileName, "wb");
    if (NULL == filePointer) {
        FatalError("failed to open STL file...");
    }
    strncpy(header, "binary stl", sizeof(StlString));
    fwrite(header, sizeof(char), sizeof(StlString), filePointer);
    facetN = poly->facetN;
    fwrite(&facetN, sizeof(StlLongInt), 1, filePointer);
    for (int n = 0; n < facetN; ++n) {
        facetData = poly->facet[n].N[X];
        fwrite(&facetData, sizeof(StlReal), 1, filePointer);
        facetData = poly->facet[n].N[Y];
        fwrite(&facetData, sizeof(StlReal), 1, filePointer);
        facetData = poly->facet[n].N[Z];
        fwrite(&facetData, sizeof(StlReal), 1, filePointer);
        facetData = poly->facet[n].P1[X];
        fwrite(&facetData, sizeof(StlReal), 1, filePointer);
        facetData = poly->facet[n].P1[Y];
        fwrite(&facetData, sizeof(StlReal), 1, filePointer);
        facetData = poly->facet[n].P1[Z];
        fwrite(&facetData, sizeof(StlReal), 1, filePointer);
        facetData = poly->facet[n].P2[X];
        fwrite(&facetData, sizeof(StlReal), 1, filePointer);
        facetData = poly->facet[n].P2[Y];
        fwrite(&facetData, sizeof(StlReal), 1, filePointer);
        facetData = poly->facet[n].P2[Z];
        fwrite(&facetData, sizeof(StlReal), 1, filePointer);
        facetData = poly->facet[n].P3[X];
        fwrite(&facetData, sizeof(StlReal), 1, filePointer);
        facetData = poly->facet[n].P3[Y];
        fwrite(&facetData, sizeof(StlReal), 1, filePointer);
        facetData = poly->facet[n].P3[Z];
        fwrite(&facetData, sizeof(StlReal), 1, filePointer);
        fwrite(&attributeCount, sizeof(StlInt), 1, filePointer);
    }
    fclose(filePointer); /* close current opened file */
    return 0;
}
/* a good practice: end file with a newline */

