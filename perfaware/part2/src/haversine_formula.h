#include <math.h>

/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 65
   ======================================================================== */

static double Square(double A);

static double RadiansFromDegrees(double Degrees);

/* NOTE: EarthRadius is generally expected to be 6372.8 */
static double ReferenceHaversine(double X0, double Y0, double X1, double Y1, double EarthRadius);