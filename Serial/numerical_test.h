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
 * Header File Guards to Avoid Interdependence
 ****************************************************************************/
#ifndef ARTRACFD_NUMERICAL_TEST_H_ /* if this is the first definition */
#define ARTRACFD_NUMERICAL_TEST_H_ /* a unique marker for this header file */
/****************************************************************************
 * Required Header Files
 ****************************************************************************/
#include "commons.h"
/****************************************************************************
 * Data Structure Declarations
 ****************************************************************************/
/****************************************************************************
 * Public Functions Declaration
 ****************************************************************************/
/*
 * Case Field Settings
 */
extern int SetField(const int tn, Space *, const Model *);
/*
 * Compute Solution Error and Functional
 */
extern int ComputeSolutionError(Space *, const Model *);
extern int ComputeSolutionFunctional(const Time *, Space *, const Model *);
#endif
/* a good practice: end file with a newline */

