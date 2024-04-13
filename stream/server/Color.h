#include <cmath>
#include <iostream>

constexpr int SYMBOL_WIDTH = 2;
constexpr int SYMBOL_HEIGHT = 3;
constexpr int idx(const int x, const int y, const int width) {
    return y * width + x;
}

// Computercraft character ids for 0-9 and a-f respectively
const unsigned char HEX_COLOR_IDS[] = { 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 97, 98, 99, 100, 101, 102 };

struct Symbol {
    int id;
    bool invert;
};

struct Processor {
    /**
     * @brief Simple RGB distance, gives worse results
     * 
     * @param palette RGB palette
     * @param i1 Index 1
     * @param i2 Index 2
     * @return double 
     */
    double color_distance_simple(const unsigned char * palette, const int i1, const int i2) {
        const unsigned char r1 = palette[i1 * 3 + 0];
        const unsigned char g1 = palette[i1 * 3 + 1];
        const unsigned char b1 = palette[i1 * 3 + 2];
        const unsigned char r2 = palette[i1 * 3 + 0];
        const unsigned char g2 = palette[i1 * 3 + 1];
        const unsigned char b2 = palette[i1 * 3 + 2];
        return 2 * (r1 - r2) * (r1 - r2) + 4 * (g1 - g2) * (g1 - g2) + 3 * (b1 - b2) * (b1 - b2);
    }

    /**
     * @brief Return distance between two LAB colors using CIE 1994 recommendation.
     * @see https://www.colour-science.org/api/0.3.3/html/_modules/colour/difference/delta_e.html
     * @param lab_palette Lab palette, array as L,a,b,L,a,b
     * @param i1 Index into lab palette (indexed as offset * 3, so 1 = palette[3], 4 and 5)
     * @param i2 Second color index
     * @return double Distance metric
     */
    double color_distance_cie1994(const double * lab_palette, const int i1, const int i2) {
        if (i1 == i2) return 0.0;

        constexpr double k1 = 0.048;
        constexpr double k2 = 0.014;
        constexpr double kL = 2;
        constexpr double kC = 1;
        constexpr double kH = 1;

        double L1 = lab_palette[i1 * 3 + 0]; double a1 = lab_palette[i1 * 3 + 1]; double b1 = lab_palette[i1 * 3 + 2];
        double L2 = lab_palette[i2 * 3 + 0]; double a2 = lab_palette[i2 * 3 + 1]; double b2 = lab_palette[i2 * 3 + 2];

        double C1 = std::sqrt(a1 * a1 + b1 * b1);
        double C2 = std::sqrt(a2 * a2 + b2 * b2);

        double sL = 1;
        double sC = 1 + k1 * C1;
        double sH = 1 + k2 * C1;

        double delta_L = L1 - L2; double delta_C = C1 - C2;
        double delta_A = a1 - a2; double delta_B = b1 - b2;
        double inside = delta_A * delta_A + delta_B * delta_B - delta_C * delta_C;
        double delta_H = inside <= 0.0 ? 0.0 : std::sqrt(inside);

        double L = std::pow(delta_L / (kL * sL), 2);
        double C = std::pow(delta_C / (kC * sC), 2);
        double H = std::pow(delta_H / (kH * sH), 2); 

        return std::sqrt(L + C + H);
    }

    double _RGB_XYZ_helper(double val) {
        val = val / 255.0;
        val = (val > 0.04045) ?
            std::pow((val + 0.055) / 1.055, 2.4) :
            val = val / 12.92;
        return val * 100.0;
    }

    double _XYZ_LAB_helper(double val) {
        return val > 0.008856 ? std::pow(val, 1.0 / 3.0) :  (7.787 * val) + (16.0 / 116.0);
    }

    /**
     * @brief Generate a new palette from an RGB palette
     * 
     * @param rgb_palette Array of length palette_size, organized as R,G,B,R,G,B,...
     * @param lab_palette New palette to store lab data of length palette_size, organized as L,a,b,L,a,b,...
     * @param palette_size Length of the two arrays
     */
    void rgb_palette_to_lab(const unsigned char * rgb_palette, double * lab_palette, const int palette_size) {
        for (int i = 0; i < palette_size; i += 3) {
            double R2 = _RGB_XYZ_helper(rgb_palette[i + 0]);
            double G2 = _RGB_XYZ_helper(rgb_palette[i + 1]);
            double B2 = _RGB_XYZ_helper(rgb_palette[i + 2]);

            double X = (R2 * 0.4124 + G2 * 0.3576 + B2 * 0.1805) / 95.047;
            double Y = (R2 * 0.2126 + G2 * 0.7152 + B2 * 0.0722) / 100.0;
            double Z = (R2 * 0.0193 + G2 * 0.1192 + B2 * 0.9505) / 108.883; 

            X = _XYZ_LAB_helper(X);
            Y = _XYZ_LAB_helper(Y);
            Z = _XYZ_LAB_helper(Z);

            lab_palette[i + 0] = (116 * Y) - 16; // L
            lab_palette[i + 1] = 500 * (X - Y);  // a
            lab_palette[i + 2] = 200 * (Y - Z);  // b
        }
    }

    Symbol get_symbol(bool chars[SYMBOL_WIDTH * SYMBOL_HEIGHT]) {
        auto sidx = [](int x, int y) { return y * SYMBOL_WIDTH + x; };
        bool invert = false;
        int delta = 0;

        if (!chars[sidx(1, 2)]) {
            delta += chars[sidx(0, 0)];
            delta += chars[sidx(1, 0)] * 2;
            delta += chars[sidx(0, 1)] * 4;
            delta += chars[sidx(1, 0)] * 8;
            delta += chars[sidx(0, 2)] * 16;
        } else {
            // Invert fg/bg required
            delta += 1 - chars[sidx(0, 0)];
            delta += (1 - chars[sidx(1, 0)]) * 2;
            delta += (1 - chars[sidx(0, 1)]) * 4;
            delta += (1 - chars[sidx(1, 1)]) * 8;
            delta += (1 - chars[sidx(0, 2)]) * 16;
            invert = true;
        }
        return Symbol { delta + 128, invert };
    }

    /**
     * @brief Process image. Processed image obeys the following rules:
     *  1. Looks good
     *  2. Every 2x3 block (computercraft block character resolution) contains only two colors
     * 
     * @param np_img       2D row-major palette mode image as numpy array
     * @param width        Width of image (px)
     * @param height       Height of image (px)
     * @param palette      Palette for image as R,G,B,R,G,B,...
     * @param palette_size Number of numbers (number of colors / 3) in the palette
     * @param text_out     Text buffer of size (width * height) / (symbol width * symbol height) to write output characters
     * @param fg_color_out Text buffer of same size as text_out to write fg color
     * @param bg_color_out Text buffer of same size as text_out to write bg color
     */
    void process_image(
            unsigned char * np_img,
            const int width,
            const int height,
            const unsigned char * palette,
            const int palette_size,
            unsigned char * text_out,
            unsigned char * fg_color_out,
            unsigned char * bg_color_out
    ) {
        double lab_palette[32]; // Assert 32 >= palette_size
        rgb_palette_to_lab(palette, lab_palette, palette_size);
        int out_idx = 0;

        for (int y = 0; y < height; y += SYMBOL_HEIGHT) {
            for (int x = 0; x < width; x += SYMBOL_WIDTH) {
                int most_common_idx = idx(x, y, width);
                int count = 1;

                int colors_in_block_idx = 0;
                int colors_in_block[SYMBOL_HEIGHT * SYMBOL_WIDTH];

                for (int dy = 0; dy < SYMBOL_HEIGHT; dy++)
                for (int dx = 0; dx < SYMBOL_WIDTH; dx++) {
                    int curr_idx = idx(x + dx, y + dy, width);
                    colors_in_block[colors_in_block_idx++] = np_img[curr_idx];

                    // This code gets most common color
                    if (np_img[curr_idx] == np_img[most_common_idx])
                        count++;
                    else
                        count--;

                    if (count == 0) {
                        most_common_idx = curr_idx;
                        count = 1;
                    }
                }

                unsigned char most_common_color = np_img[most_common_idx];
                unsigned char most_contrasting_color = most_common_color;
                int max_dis = 0.0;

                for (int i = 0; i < colors_in_block_idx; i++) {
                    const int color_i = colors_in_block[i];
                    const double dis = color_distance_cie1994(lab_palette, color_i, most_common_color);
                    // const double dis = color_distance_simple(palette, color_i, most_common_color);

                    if (dis > max_dis) {
                        max_dis = dis;
                        most_contrasting_color = color_i;
                    }
                }

                bool chars[SYMBOL_WIDTH * SYMBOL_HEIGHT];
                int symbol_idx = 0;

                for (int dy = 0; dy < SYMBOL_HEIGHT; dy++)
                for (int dx = 0; dx < SYMBOL_WIDTH; dx++) {
                    const int color_i = np_img[idx(x + dx, y + dy, width)];
                    const double dis1 = color_distance_cie1994(lab_palette, color_i, most_common_color);
                    const double dis2 = color_distance_cie1994(lab_palette, color_i, most_contrasting_color);

                    // const double dis1 = color_distance_simple(palette, color_i, most_common_color);
                    // const double dis2 = color_distance_simple(palette, color_i, most_common_color);

                    // Pick the most common or most contrasting, whichever is closer
                    // Assume most_common_color is fg
                    bool is_common = dis1 < dis2;
                    chars[symbol_idx++] = is_common;
                    np_img[idx(x + dx, y + dy, width)] = is_common ? most_common_color : most_contrasting_color;
                }

                Symbol sym = get_symbol(chars);
                if (sym.invert) std::swap(most_common_color, most_contrasting_color);

                text_out[out_idx] = sym.id;
                fg_color_out[out_idx] = HEX_COLOR_IDS[most_common_color];
                bg_color_out[out_idx] = HEX_COLOR_IDS[most_contrasting_color];
                out_idx++;
            }
        }
    }
};
