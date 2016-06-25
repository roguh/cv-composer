#ifndef _CANNY_HPP
#define _CANNY_HPP

#include <list>
#include <iostream>
#include "util.hpp"

using namespace cv;

/* run the Canny edge detection algorithm on the given 32-bit float input
 * can save intermediate results, use N7 neighbors during non-max suppression,
 * use Sobel or Scharr kernels to find the gradient
 */
void canny_edges(const Mat& bgr_input, Mat& output,
                 bool save=false, bool gui=false,
                 float min_thresh=0.6, float max_thresh=0.8, bool n8=false,
                 bool useSobel=true, std::string interm_name_prefix="",
                 bool dynamic_thresh=false);

class CannyInterface : public Interface {
public:
    const float default_thi = 0.5;
    const float default_tlo = 0.25;

CannyInterface() : Interface(CV_32FC1,
R"(Usage: canny [--sobel | --scharr]
             [--thresh-hi=<thi>] [--thresh-lo=<tlo>]
             [--n8 | --n4] [--help] [<algorithm> [<args>...]]]

Options:
  --thresh-hi=<thi> Threshold [default: 0.5].
  --thresh-lo=<tlo> Threshold [default: 0.25].
  -h --help Show this message.
)")
    { }

    inline virtual void process_frame(const Mat& in, Mat& out, std::string prefix="") override
    {
        canny_edges(in, out,
                    main_args["--show-intermediates"].asBool(), // save interm.
                    !main_args["--no-interm-gui"].asBool(), // show interm.
                    docopt_to_float(args, "--thresh-lo", default_thi), // min thresh
                    docopt_to_float(args, "--thresh-hi", default_tlo), // max thresh
                    args["--n8"].asBool() && !args["--n4"].asBool(), // n8 or n4
                    args["--sobel"].asBool() && !args["--scharr"].asBool(), // sobel or scharr
                    prefix);
    }
};

#endif
