#ifndef __APPLE__
#  include "HalconCpp.h"
#  include "HDevThread.h"
#else
#  ifndef HC_LARGE_IMAGES
#    include <HALCONCpp/HalconCpp.h>
#    include <HALCONCpp/HDevThread.h>
#    include <HALCON/HpThread.h>
#  else
#    include <HALCONCppxl/HalconCpp.h>
#    include <HALCONCppxl/HDevThread.h>
#    include <HALCONxl/HpThread.h>
#  endif
#  include <stdio.h>
#  include <CoreFoundation/CFRunLoop.h>
#endif

using namespace HalconCpp;
