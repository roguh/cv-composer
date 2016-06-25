#include "convolution.hpp"

/* finds the convolution of a matrix and kernel
 * the input should be an 1-channel floating point image 
 * written as an illustration only */
int naive_convolution(const Mat& input, Mat& output, const Mat& kernel)
{
    int krows = kernel.rows;
    int kcols = kernel.cols;

    /* region of interest */
    Mat roi = input(cv::Rect(0, 0, krows, kcols));

    for (int r = 0; r < output.rows; r++) {
        for (int c = 0; c < output.cols; c++) {
            float& pixel = output.at<float>(r, c);

            if ((r < krows / 2) || (r > output.rows - krows / 2 - 1)
             || (c < kcols / 2) || (c > output.cols - kcols / 2 - 1)) {
                /* set out of bounds pixels to black */
                pixel = 0;
            } else {
                /* move the ROI */
                roi = input(cv::Rect(c - kcols / 2, r - krows / 2,
                                     kcols, krows));

                /* perform element-wise multiplication and take the sum
                 * to get the value of the current pixel */
                pixel = sum(kernel.mul(roi))[0];
            }
        }
    }
    return 0;
}

