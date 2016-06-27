#ifndef _UTIL_HPP
#define _UTIL_HPP

#include <iostream>
#include <opencv2/opencv.hpp>

#include "../lib/docopt.cpp/docopt.h"

inline void save_mat(const std::string name, const cv::Mat mat,
                     bool save, bool gui)
{
    if (save) {
        if (gui) {
            cv::namedWindow(name, cv::WINDOW_NORMAL);
            cv::imshow(name, mat);
        } else {
            cv::Mat _mat(mat);
            if (mat.type() != CV_8UC3) {
                mat.convertTo(_mat, CV_8UC3, 256);
            }
            std::cout << "Writing to '_" << name << ".png'" << std::endl;
            cv::imwrite("_" + name + ".png", _mat);
        }
    }
}

inline float docopt_to_float(std::map<std::string, docopt::value> args,
                             std::string key, float default_v)
{
    if (!args[key].isEmpty()) {
        try {
            return std::stof(args[key].asString());
        } catch (std::invalid_argument) {
            std::cout << "Argument '" << key << "' expected number, got "
                      << args[key] << " using " << default_v << std::endl;
        }
    }
    return default_v;
}

/* this class controls the setup and activity of an algorithm that takes a
 * single input and has a main output
 *
 * docopt is used to control algorithm parameters
 *
 * by default,
 * run_on_images calls process_mat on each image
 * frames are read from every camera and
 * run_on_frames calls process_mat on each frame
 */
class FrameAlgorithm {
public:
    FrameAlgorithm(std::string _doc) : doc(_doc) {}

    std::map<std::string, docopt::value> args;
    std::map<std::string, docopt::value> main_args;

    std::string doc = "Usage: algorithm [<algorithm> [<args>...]]";

    virtual void process_frame(const cv::Mat& in, cv::Mat& out,
                               std::string prefix="") = 0;

    inline virtual std::map<std::string, docopt::value>
    parse_arguments(std::map<std::string, docopt::value> main_args,
                    std::vector<std::string> argv)
    {
        this->main_args = main_args;
        this->args = docopt::docopt(this->doc, argv, true, "", true);
        return {{"<algorithm>", docopt::value()}};
    }
};

#endif
