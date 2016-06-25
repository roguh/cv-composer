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
    std::vector<VideoCapture> cameras;
    std::vector<Mat> video_frames;
    std::vector<std::string> image_names;
    std::vector<Mat> images;

    std::map<std::string, std::shared_ptr<Interface>> algorithms;
    algorithms["canny"] = std::make_shared<CannyInterface>();
    algorithms["convolution"] = std::make_shared<ConvolutionInterface>();
    algorithms["two_pass"] = std::make_shared<TwoPassInterface>();

    std::vector<std::string> v_argv(argc - 1);
    for (int i = 1; i < argc; i++) {
        v_argv[i - 1] = std::string(argv[i]) ;
    }

    for (auto& a: algorithms) {
        doc += "  " + a.first + "\n";
    }

    auto args = docopt::docopt(doc, v_argv,
                         true, // print help and exit
                         "composer 0.0.0", // version
                         true); // leave options at end alone

    /* open camera */
    if (args["--camera"].asBool()) {
        VideoCapture cap;
        if (open_camera(0, cap)) {
            Mat mat;
            cameras.push_back(cap);
            video_frames.push_back(mat);
            cameras.back() >> video_frames.back();
        }
    }

    /* open images */
    for (auto& fname : args["--image"].asStringList()) {
        std::cout << "Reading from '" << fname << "'" << std::endl;

        Mat input = imread(fname, 1);

        if (!input.data) {
            std::cout << "Failed to open '" << fname << "'" << std::endl;
        } else {
            images.push_back(input);
            image_names.push_back(fname);
        }
    }

    /* run the given algorithm */
    try {
        auto interface = algorithms.at(args["<algorithm>"].asString());
        interface->parse_arguments(args, args["<args>"].asStringList());
        /* pass input and arguments to each algorithm 
         * in case of camera or video input; do this indefinitely */ 
         // TODO
        interface->run_algorithm(images, image_names, video_frames, cameras);
    } catch (std::out_of_range e) {
        std::cout << "Algorithm " << args["<algorithm>"]
                  << " not found" << std::endl;
    }

    return 0;
}
