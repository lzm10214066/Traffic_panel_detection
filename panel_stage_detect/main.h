#include <stdio.h>
#include <stdlib.h>
#include <io.h>

#include <iostream>
#include <fstream>

#include "opencv2\core\core.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\imgproc\imgproc.hpp"
#include "opencv2\ml\ml.hpp"

#include "../src/imageRead.h"
#include "../src/Lines/lsd.h"
#include "../src/Lines/lines.h"
#include "../src/LBP/featureLBP.h"
#include "../src/LBP/classifierrLBP.h"
#include "../src/utility.h"
#include "../src/HOG/classifierHOG.h"
#include "../src/Bow/classifierBow.h"

using namespace cv;
using namespace std;
using namespace lbp;