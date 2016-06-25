#ifndef _TWO_PASS_HPP
#define _TWO_PASS_HPP

#include <functional>
#include "util.hpp"

using namespace cv;

inline float bgr_to_gray(Vec3b bgr)
{
    return (0.114 * bgr[0] + 0.587 * bgr[1] + 0.299 * bgr[2]) / 256.0;
}

float cat_by_value(Vec3b, void*);

void two_pass(const Mat& input, Mat& output,
              const std::function<float(Vec3b, void*)>& categorize, void* conf, bool north_bias=true);

class TwoPassInterface : public Interface {
public:
    float max_cats = 10;
    bool north_bias = true;

TwoPassInterface() : Interface(CV_32FC3,
R"(Usage: two_pass [--west-bias] [--max-categories=<cats>] [--help | -h] [<algorithm> [<args>...]]]

Options:
  -c <cats> --max-categories=<cats> Maximum number of categories.
  -w --west-bias    Instead of northern bias.
  -h --help Show this message.
)")
    { }

    inline virtual void parse_arguments(std::map<std::string, docopt::value> m,
                                        std::vector<std::string> a)
    {
        Interface::parse_arguments(m, a);
        max_cats = docopt_to_float(args, "--max-categories", max_cats);
        north_bias = !args["--west-bias"].asBool();
        max_cats = (float)((int) max_cats);
    }

    inline virtual void process_frame(const Mat& in, Mat& out, std::string prefix="") override
    {
        two_pass(in, out, cat_by_value, &max_cats, north_bias);
    }
};

#endif
