#ifndef _CONVOLUTION_HPP
#define _CONVOLUTION_HPP
#include "util.hpp"
using namespace cv;
int convolution(const Mat& input, Mat& output, const Mat& kernel);

class ConvolutionAlgorithm : public FrameAlgorithm {
public:
    bool apply_gauss = false;
    bool apply_laplacian = false;
    bool apply_kernel = false;
    bool apply_polar = false;
    float gauss_stddev = 0.5;
    std::string kernel_key;
    std::string kernel_key_x;
    std::string kernel_key_y;

    std::map<std::string, Mat_<float>> kernels;

ConvolutionAlgorithm() : FrameAlgorithm(
R".(Usage: convolution [--kernel=<name>] [--gaussian=<stddev>] [--laplacian] [--polar-x=<kernel-x> --polar-y=<kernel-y>] [--help | -h]] [<algorithm> [<args>...]]

options:
  -k <name> --kernel=<name> Apply a given kernel.
  -g <stddev> --gaussian=<stddev> Gaussian.
  -l --laplacian Laplacian.
  -px <kernel-x> --polar-x=<kernel-x> Grayscale magnitude. Must also give --polar-y.
  -py <kernel-y> --polar-y=<kernel-y> Grayscale magnitude. Must also give --polar-x.

Kernels:
).")
    {
        kernels["sobel-west"] = (Mat_<float>(3,3) <<
             1, 0, -1,
             2, 0, -2,
             1, 0, -1);
        kernels["sobel-north"] = (Mat_<float>(3,3) <<
             1, 2, 1,
             0, 0, 0,
             -1, -2, -1);
        kernels["scharr-west"] = (Mat_<float>(3,3) <<
             3, 0, -3,
             10, 0, -10,
             3, 0, -3);
        kernels["scharr-north"] = (Mat_<float>(3,3) <<
             3, 5, 3,
             0, 0, 0,
             -3, -5, -3);
        kernels["sharpen"] = (Mat_<float>(3, 3) <<
             -1, -1, -1,
             -1, 9, -1,
             -1, -1, -1);
        kernels["laplacian"] = (Mat_<float>(5, 5) <<
             0, 0, 1, 0, 0,
             0, 1, 2, 1, 0,
             1, 2, -16, 2, 1,
             0, 1, 2, 1, 0,
             0, 0, 1, 0, 0);
        /* append kernel names to documentation */
        for (auto& k: kernels) {
            doc += "  " + k.first + "\n";
        }
    }

    inline bool is_kernel(docopt::value v)
    {
        if (v.isString()) {
            try {
                kernels.at(v.asString());
                return true;
            } catch (std::out_of_range) {
                std::cout << "convolution kernel '" << v.asString() << "' unknown" << std::endl;
            }
        }
        return false;
    }

    inline virtual std::map<std::string, docopt::value>
    parse_arguments(std::map<std::string, docopt::value> m,
                    std::vector<std::string> a)
    {
        FrameAlgorithm::parse_arguments(m, a);
        if (is_kernel(args["--kernel"])) {
            kernel_key = args["--kernel"].asString();
            apply_kernel = true;
        }
        if (!args["--laplacian"].isEmpty() && args["--laplacian"].asBool()) {
            apply_laplacian = true;
        }
        if (args["--gaussian"].isString()) {
            apply_gauss = true;
            gauss_stddev = docopt_to_float(args, "--gaussian", gauss_stddev);
        }
        if (is_kernel(args["--polar-x"]) && is_kernel(args["--polar-y"])) {
            apply_polar = true;
            kernel_key_x = args["--polar-x"].asString();
            kernel_key_y = args["--polar-y"].asString();
        }
        return args;
    }

    inline virtual void process_frame(const Mat& in, Mat& out, std::string prefix="") override
    {
        in.copyTo(out);
        if (apply_gauss) {
            GaussianBlur(out, out, Size(9, 9), gauss_stddev, gauss_stddev);
        }
        if (apply_laplacian) {
            Laplacian(out, out, -1, 9);
        }
        if (apply_kernel) {
            filter2D(out, out, -1, kernels[kernel_key]);
        }
        if (apply_polar) {
            cvtColor(in, out, CV_BGR2GRAY);
            out.convertTo(out, CV_32FC1, 1/256.0);

            Mat x, y, dir;
            filter2D(out, x, CV_32F, kernels[kernel_key_x]);
            filter2D(out, y, CV_32F, kernels[kernel_key_y]);
            cartToPolar(x, y, out, dir, true);
            dir /= 360;
        }
    }
};

#endif
