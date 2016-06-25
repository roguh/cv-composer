#ifndef _UTIL_HPP
#define _UTIL_HPP

#include <iostream>
#include <opencv2/opencv.hpp>

#include "../lib/docopt.cpp/docopt.h"

inline void save_mat(const std::string name, const cv::Mat mat,
                     bool save, bool gui) {
    if (save) {
        if (gui) {
            cv::namedWindow(name, cv::WINDOW_NORMAL);
            cv::imshow(name, mat);
        } else {
            cv::Mat _mat;
            if (mat.type() != CV_8UC3) {
                mat.convertTo(_mat, CV_8UC3, 256);
            }
            std::string fname = "_" + name + ".png";
            std::cout << "writing to '" << fname << "'" << std::endl;
            cv::imwrite(fname, _mat);
        }
    };
}

inline float docopt_to_float(std::map<std::string, docopt::value> args,
                             std::string key, float default_v,
                             bool print_message=true)
{
    if (!args[key].isEmpty()) {
        try {
            return std::stof(args[key].asString());
        } catch (std::invalid_argument) {
            if (print_message) {
                std::cout << "Argument '" << key << "' expected number, got "
                          << args[key] << " defaulting to " << default_v << std::endl;
            }
        }
    }
    return default_v;
}

class Interface {
public:
    Interface() {}
    Interface(int _output_format, std::string _doc) :
        output_format(_output_format), doc(_doc) {}
    std::map<std::string, docopt::value> args;
    std::map<std::string, docopt::value> main_args;
    std::string doc = "";
    int output_format = 0;

    virtual bool check_inputs(int n_images, int n_cameras)
    {
        if (n_images + n_cameras == 0) {
            std::cout << "one or more inputs expected" << std::endl;
            return false;
        }
        return true;
    }
    virtual void process_frame(const cv::Mat& in, cv::Mat& out,
                               std::string prefix="") = 0;

    inline virtual void parse_arguments(std::map<std::string, docopt::value>
                                        main_args, std::vector<std::string>
                                        argv)
    {
        this->main_args = main_args;
        this->args = docopt::docopt(this->doc, argv, true);
    }

    /* by default, calls process_frame on each image
     * and reads new frames from each camera at least every 10ms
     * then runs process_frame on each
     */
    inline virtual void run_algorithm(std::vector<cv::Mat>& images,
                       std::vector<std::string>& image_names,
                       std::vector<cv::Mat>& frames,
                       std::vector<cv::VideoCapture>& cameras)
    {
        if (!check_inputs(images.size(), cameras.size())) {
            return;
        }
        for (int i = 0; i < images.size(); i++) {
            cv::Mat output (images[i].size(), this->output_format);
            this->process_frame(images[i], output, image_names[i]);
            save_mat("in_" + image_names[i], images[i], true,
                     !main_args["--no-gui"].asBool());
            save_mat("output_" + image_names[i], output, true,
                     !main_args["--no-gui"].asBool());
        }
        std::vector<cv::Mat> outputs (frames.size());
        bool first = true;
        while (true) {
            if (cameras.size() > 0 && !main_args["--no-gui"].asBool()) {
                for (int i = 0; i < cameras.size(); i++) {
                    cameras[i] >> frames[i];
                    if (first) {
                        outputs[i] = cv::Mat(frames[i].size(),
                                             this->output_format);
                        std::cout << "Press Q or ESC or type Ctrl-C to exit."
                                  << std::endl;
                    }
                    this->process_frame(frames[i], outputs[i], "camera_");
                    save_mat("camera_" + i, outputs[i], true,
                             !main_args["--no-gui"].asBool());
                    save_mat("out_camera_" + i, frames[i], true,
                             !main_args["--no-gui"].asBool());
                }
            }
            auto key = cv::waitKey(10);
            if (key == 'q' || key == 'Q' || key == 27) {
                break;
            }
            first = false;
        }
    }
};

#endif
