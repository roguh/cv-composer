#include "two_pass.hpp"

float cat_by_value(Vec3b bgr, void* conf) 
{
    //CLARIFY
    float max_cats = conf == nullptr ? 2 : *((float*)conf);
    float gray = bgr_to_gray(bgr);
    return (float)((int)(gray * max_cats)) / max_cats;
}


void two_pass(const Mat& input, Mat& output,
              const std::function<float(Vec3b, void*)>& categorize, void* conf, bool north_bias)
{
    /* categorize pixels */
    Mat categories(input.size(), CV_32FC1);
    for (int r = 0; r < input.rows; r++) {
        for (int c = 0; c < input.cols; c++) {
            Vec3b val = input.at<Vec3b>(r, c);
            categories.at<float>(r, c) = categorize(val, conf); 
        }
    }

    /* pass 1:
     * find contiguous runs of pixels */
    float label = 0;
    for (int r = 1; r < input.rows - 1; r++) {
        for (int c = 1; c < input.cols - 1; c++) {
            float north = categories.at<float>(r - 1, c); 
            float west  = categories.at<float>(r, c - 1); 
            float here  = categories.at<float>(r, c);
            if (north_bias) {
                if (c > 1 && here == north) {
                    output.at<Vec3f>(r, c) = output.at<Vec3f>(r - 1, c); 
                } else if (r > 1 && here == west) {
                    output.at<Vec3f>(r, c) = output.at<Vec3f>(r, c - 1); 
                } else {
                    output.at<Vec3f>(r, c) = Vec3f(label, 1.0, 1.0); 
                    label += 1; 
                }
            } else {
                if (r > 1 && here == west) {
                    output.at<Vec3f>(r, c) = output.at<Vec3f>(r, c - 1); 
                } else if (c > 1 && here == north) {
                    output.at<Vec3f>(r, c) = output.at<Vec3f>(r - 1, c); 
                } else {
                    output.at<Vec3f>(r, c) = Vec3f(label, 1.0, 1.0); 
                    label += 1; 
                }
            }
            if (here == west && west == north) {
                // equiv(west.label, north.label) 
            }

        }
    }

    /* visualization */
    auto hsv(std::vector<Mat>(3));
    split(output, hsv);
    hsv[0] /= label / 256;
    merge(hsv, output);
    cvtColor(output, output, CV_HSV2BGR);
    save_mat("categories", categories, true, true);

    /* pass 2: merge equivalent labels */
}
