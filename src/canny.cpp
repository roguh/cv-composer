#include "canny.hpp"

const float WEAK_GRAD   = 0.5;
const float STRONG_GRAD = 1.0;

/* scale the min and max threshholds according to max value of image */
void calc_dynamic_thresh(const Mat mag, const Mat dir,
                         float& min_thresh, float& max_thresh)
{
    /* find highest value */
    double _max;
    minMaxLoc(mag, NULL, &_max);

    max_thresh = min(_max, _max * max_thresh);
    min_thresh = min(_max, _max * min_thresh);
}

/* find magnitude and direction of gradient */
void polarGradient(const Mat& input, Mat kernel, Mat& mag, Mat& dir)
{
    Mat dx (mag.size(), mag.type());
    Mat dy (mag.size(), mag.type());

    filter2D(input, dx, -1, kernel);

    /* rotate the kernel */
    transpose(kernel, kernel);
    flip(kernel, kernel, 1);

    filter2D(input, dy, -1, kernel);

    /* convert gradient to polar coordinates */
    cartToPolar(dx, dy, mag, dir, true);
}

/* used within the inner loop of non-maximal suppression algorithm
 * zero a pixel when its neighbor (N4 or N8) has a greater gradient magnitude
 */
inline bool zero_if_non_max(const Mat& mag, const Mat& dir, int r, int c,
                            Mat& output, bool n8=false)
{
#define wrap(x) (fmod(x, 360))
    float m = mag.at<float>(r, c);
    float d = dir.at<float>(r, c);
    float up, dw;
    /* degrees and corresponding row/column offsets */
    float offsets8[][3] = {{0, 0, 1},
                           {45, 1, 1},
                           {90, 1, 0},
                           {135, 1, -1}};

    float offsets[][3] = {{0, 0, 1},
                          {90, 1, 0}};

    const float ex = n8 ? 22.5 : 45;

    for (int i=0; i < (n8 ? 4 : 2); i++) {
        float* offset = n8 ? offsets8[i] : offsets[i];
        if ((d >= wrap(offset[0] - ex)       && wrap(d < offset[0] + ex))
        ||  (d >= wrap(offset[0] - ex + 180) && wrap(d < offset[0] + ex + 180)))
            {
                up = mag.at<float>(r + offset[1], c + offset[2]);
                dw = mag.at<float>(r - offset[1], c - offset[2]);
                break;
            }
    }
    if (up > m || dw > m) {
        output.at<float>(r, c) = 0;
        return true;
    }
    return false;
}

/* non-maximal suppression
 * pixels are set to 0.5 if between min_thresh and max_thresh,
 * 1.0 if above max_thresh, 0.0 otherwise
 * also, set to 0.0 if not a local maximum in N4 or N8 neighborhood */
void non_max_suppresion(const Mat& mag, const Mat& dir, Mat& output,
                        std::list< Point_<int> >& strong,
                        std::list< Point_<int> >& weak,
                        float min_thresh, float max_thresh, bool n8=false)
{
    for (int r = 0; r < mag.rows; r++) {
        for (int c = 0; c < mag.cols; c++) {
            /* check neighbors of each pixel */
            float m = mag.at<float>(r, c);
            float d = dir.at<float>(r, c);
            if (m >= min_thresh
            && r > 0 && r < mag.rows - 1
            && c > 0 && c < mag.cols - 1)
            {
                output.at<float>(r, c) = WEAK_GRAD;
                if (m >= max_thresh) {
                    output.at<float>(r, c) = STRONG_GRAD;
                    m = STRONG_GRAD;
                } else {
                    m = WEAK_GRAD;
                }

                /* if there's a pixel with a higher direction in the same
                 * or opposite direction as the pixel, set to zero */
                if (!zero_if_non_max(mag, dir, r, c, output, n8)) {
                    if (m == WEAK_GRAD) {
                        weak.push_back(Point_<int>(r, c));
                    } else {
                        strong.push_back(Point_<int>(r, c));
                    }
                }
            } else {
                output.at<float>(r, c) = 0;
            }
        }
    }
}

/* all pixels with strong gradients are in the vector
 * marks all weak pixels as strong if they're connected to a strong pixel */
void link_edges(const std::list< Point_<int> > weak,
                std::list< Point_<int> >& strong,
                Mat& output, bool n8=true)
{
    while (!strong.empty()) {
        /* mark all weak N8 neighbors as strong and push to the strong list */
        auto p = strong.front();
        int r = p.x;
        int c = p.y;
        strong.erase(strong.begin());

        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                if (i != 0 && j != 0) {
                    if (output.at<float>(r + i, c + j) == WEAK_GRAD) {
                        output.at<float>(r + i, c + j) = STRONG_GRAD;
                        strong.push_back(Point_<int>(r + i, c + j));
                    }
                }
            }
        }
    }

    /* this list was originally populated by pixels with weak gradient */
    for (auto p: weak) {
        if (output.at<float>(p.x, p.y) == WEAK_GRAD) {
            output.at<float>(p.x, p.y) = 0;
        }
    }
}

void canny_edges(const Mat& bgr_input, Mat& output,
                 bool save, bool gui,
                 float min_thresh, float max_thresh, bool n8,
                 bool useSobel, std::string interm_name_prefix,
                 bool dynamic_thresh)
{
    /* convert to grayscale and smooth the result */
    Mat input;

    cvtColor(bgr_input, input, CV_BGR2GRAY);
    input.convertTo(input, CV_32FC1, 1.0f/256.0f);

    save_mat(interm_name_prefix + "gray", input, save, gui);

    /* find gradient magnitude and direction using Sobel filters */
    Mat kernel;
    if (useSobel) {
        kernel = Mat ((Mat_<float>(3, 3) <<
                       -1, 0, 1,
                       -2, 0, 2,
                       -1, 0, 1));
    } else {
        kernel = Mat ((Mat_<float>(3, 3) <<
                       -3,  0,  3,
                       -10, 0, 10,
                       -3,  0,  3));
    }
    Mat mag (input.size(), input.type());
    Mat dir (input.size(), input.type());
    polarGradient(input, kernel, mag, dir);

    /* try to dynamically calculate thresholds */
    if (dynamic_thresh) {
        calc_dynamic_thresh(mag, dir, min_thresh, max_thresh);
    }

    /* vectors of all weak and strong pixels */
    std::list<Point_<int> > strong, weak;
    non_max_suppresion(mag, dir, // inputs
                       output, strong, weak, // output
                       min_thresh, max_thresh, n8 /* parameters */);

    dir /= 360;
    // save_mat("gradient_direction", dir, save, gui);
    // save_mat("gradient_magnitude", mag, save, gui);
    save_mat(interm_name_prefix + "non_maximal_suppression"
                      + (n8 ? std::string("8") : std::string("4")),
                      output, save, gui);

    /* edge tracking */
    link_edges(weak, strong, output);

    save_mat(interm_name_prefix + "edge_linking"
                      + (n8 ? std::string("8") : std::string("4")),
                      output, save, gui);
}
