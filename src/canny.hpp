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

class CannyAlgorithm : public FrameAlgorithm {
public:
    const float default_thi = 0.5;
    const float default_tlo = 0.25;

    bool save_interm;
    bool show_interm;
    bool max_thresh;
    bool min_thresh;
    bool n8;
    bool scharr;

    CannyAlgorithm() : FrameAlgorithm(
R"(Usage: canny [--sobel | --scharr]
             [--max-thresh=<thi>] [--min-thresh=<tlo>]
             [--n8 | --n4] [--help] [<algorithm> [<args>...]]]

Options:
  --max-thresh=<thi> Threshold [default: 0.5].
  --min-thresh=<tlo> Threshold [default: 0.25].
  -h --help Show this message.
)")
    { }

    inline virtual std::map<std::string, docopt::value>
    parse_arguments(std::map<std::string, docopt::value> m,
                    std::vector<std::string> a)
    {
        FrameAlgorithm::parse_arguments(m, a);
        save_interm = main_args["--show-intermediates"].asBool();
        show_interm = main_args["--no-interm-gui"].asBool();
        max_thresh = docopt_to_float(args, "--min-thresh", default_thi);
        min_thresh = docopt_to_float(args, "--max-thresh", default_tlo);
        n8 = args["--n8"].asBool();
        scharr = args["--scharr"].asBool();
        return args;
    }

    inline virtual void process_frame(const Mat& in, Mat& out,
                                      std::string prefix="") override
    {
        canny_edges(in, out, save_interm, show_interm, min_thresh, max_thresh,
                    n8, !scharr, prefix);
    }
};

#endif
