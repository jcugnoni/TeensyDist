/*
 * Distortion modeling using Teensy 3.5/3.6 and i2s audio codec
 * 
 * Copyright, J.Cugnoni, 2017
 *  
 * Licence GNU GPLv3
 * 
 */

// General include file for TeensyDist distortion modeling library.
// Use this include file directly if you want to use all features, at the expense of a larger memory usage
// If you only use some objects, it is more efficient to include directly the correponding headers instead of this general one.

#include <TD_filterf.h>     
#include <TD_compressor.h>
#include <TD_filterIIR.h>
#include <TD_waveshaperf.h>
#include <TD_multistagedist.h>
#include <TD_tubetable.h>

//#include <TD_filter.h>
//#include <TD_waveshaper.h>
