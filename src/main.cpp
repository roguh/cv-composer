#include <memory>
#include <iostream>
#include <map>

#include "../lib/docopt.cpp/docopt.h"
#include "util.hpp"
#include "convolution.hpp"
#include "canny.hpp"
#include "two_pass.hpp"

using namespace cv;

std::string doc =
R"(Usage: composer [-Sng] [--image=<path>...] [--camera]
                [--quiet] [--help]
                <algorithm> [<args>...]
       composer --version

Options:
  -i <path> --image=<path> Process an image.
  -c --camera              Process stream from default camera.
  -S --show-intermediates  Show intermediate steps for all algorithms.
  -n --no-gui              Save output to .png instead of showing using
                           OpenCV's highgui. Does not save any video.
  -g --no-interm-gui       Same as above, applies only to intermediate images.
  -h --help                Show this message.
     --version             Print version.
  -q --quiet               Suppress all printing.

For help on an algorithm:
  composer <algorithm> --help

Available algorithms:
  convolution
  morphology, erode, dilate
  histogram_equalization

  selective_sharpen

  distance_transform

  canny

  two_pass
  snakes
  level_sets

  track

  skel_voronoi
  skel_distance
  skel_thinning

  blue_screen
  difference_keying

Actually available algorithms:
)";

bool open_camera(int id, VideoCapture& cap)
{
    std::cout << "Reading from default camera " << id << std::endl;
    cap = VideoCapture(id);
    if (!cap.isOpened()) {
        std::cout << "Failed to open camera " << id << std::endl;
        return false;
    }
    return true;
}

int main(int argc, char** argv )
{
    std::vector<Mat> frames, images;
    std::vector<VideoCapture> cameras;
    std::vector<std::string> image_names;

    /* define available algorithms */
    std::map<std::string, std::function<std::shared_ptr<FrameAlgorithm>()>> algs;

    algs["canny"] = []() {
        return std::make_shared<CannyAlgorithm>(); };
    algs["convolution"] = []() {
        return std::make_shared<ConvolutionAlgorithm>(); };
    algs["two_pass"] = []() {
        return std::make_shared<TwoPassAlgorithm>(); };

    std::vector<std::string> v_argv(argv + 1, argv + argc);

    /* add algorithm names to docstring */
    for (auto& a: algs) {
        doc += "  " + a.first + "\n";
    }

    auto args = docopt::docopt(doc, v_argv,
                         true, // print help and exit
                         "composer 0.0.0", // version
                         true); // leave options at end alone
    auto main_args = args;

    /* open camera */
    if (main_args["--camera"].asBool()) {
        VideoCapture cap;
        if (open_camera(0, cap)) {
            cameras.push_back(cap);
            frames.push_back(Mat());
            cameras.back() >> frames.back();
        }
    }

    /* open images */
    for (auto& fname : main_args["--image"].asStringList()) {
        std::cout << "Reading from '" << fname << "'" << std::endl;
        Mat input = imread(fname, 1);

        if (!input.data) {
            std::cout << "Failed to open '" << fname << "'" << std::endl;
        } else {
            images.push_back(input);
            image_names.push_back(fname);
        }
    }

    /* parse parameters for each algorithm */
    std::vector<std::shared_ptr<FrameAlgorithm>> active_algorithms;
    std::vector<std::string> active_names;
    while (true) {
        try {
            if (args["<algorithm>"].isEmpty()) {
                break;
            }
            std::string name = args["<algorithm>"].asString();
            active_names.push_back(name);
            active_algorithms.push_back(algs.at(name)());
        } catch (std::out_of_range e) {
            std::cout << "Algorithm " << args["<algorithm>"] << " not found: "
                      << std::string(e.what()) << std::endl;
        }
        args = active_algorithms.back()->
            parse_arguments(main_args, args["<args>"].asStringList());
    }

    std::cout << "Press q or ESC, or type Ctrl-C to exit." << std::endl;

    /* run each algorithm in sequence on every image */
    std::vector<cv::Mat> outputs(images.size());
    for (int i = 0; i < active_algorithms.size(); i++) {
            for (int j = 0; j < images.size(); j++) {
                auto prefix = image_names[j] + active_names[i];
                active_algorithms[i]->
                    process_frame(images[j], outputs[j], prefix);
                save_mat(prefix, outputs[j], true, true);
            }
        images = outputs;
    }

    /* main loop: read from available cameras and show OpenCV windows */
    if (!main_args["--no-gui"].asBool()) {
        std::vector<cv::Mat> outputs(frames.size());
        long frame_number = 0;
        while (true) {
            /* run each algorithm on each frame */
            for (int i = 0; i < frames.size(); i++) {
                save_mat("input_" + std::to_string(i), frames[i], true, true);
                for (int j = 0; j < active_algorithms.size(); j++) {
                    auto prefix = active_names[j] + "_" + std::to_string(i);
                    active_algorithms[j]->
                        process_frame(frames[i], outputs[i], prefix);
                    save_mat(prefix, outputs[i], true, true);

                    if (j < active_algorithms.size() - 1) {
                        frames[i] = outputs[i];
                    } else {
                        outputs[i] = Mat();
                    }
                }
            }

            /* get new frames */
            for (int i = 0; i < cameras.size(); i++) {
                cameras[i] >> frames[i];
            }

            auto key = cv::waitKey(10);
            if (key == 'q' || key == 'Q' || key == 27) {
                break;
            }
            frame_number++;
        }
    }


    return 0;
}
