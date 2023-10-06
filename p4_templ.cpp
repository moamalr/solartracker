
#include "image_pr4.h"
#include <math.h>
#include <thread>
#include <chrono>
#include <iostream> // For logging

// Constants
const double OMEGA = 0.1;
const int MIN_POINTS = 15;
const double QUALITY_THRESHOLD = 0.7;
const double CIRCULARITY_THRESHOLD = 0.8;

// Global variables to store sun's historical positions
std::vector<int> sunXs;
std::vector<int> sunYs;

bool isDaytime(int ySun, int imageHeight) {
    return ySun < imageHeight;
}

double circularity(int count, int perimeter) {
    return 4 * M_PI * count / (perimeter * perimeter);
}

double detectSunAndLog(int& sunX, int& sunY, ImagePPM image) {
    int sumX = 0, sumY = 0, count = 0, perimeter = 0;
    for (int y = 0; y < image.height; y++) {
        for (int x = 0; x < image.width; x++) {
            unsigned char r = get_pixel(image, y, x, 0);
            unsigned char g = get_pixel(image, y, x, 1);
            unsigned char b = get_pixel(image, y, x, 2);
            if (r > 200 && g < 100 && b < 100) {
                sumX += x;
                sumY += y;
                count++;
                bool isBoundary = false;
                for (int dy = -1; dy <= 1 && !isBoundary; dy++) {
                    for (int dx = -1; dx <= 1 && !isBoundary; dx++) {
                        if (dx == 0 && dy == 0) continue;
                        unsigned char nr = get_pixel(image, y + dy, x + dx, 0);
                        unsigned char ng = get_pixel(image, y + dy, x + dx, 1);
                        unsigned char nb = get_pixel(image, y + dy, x + dx, 2);
                        if (!(nr > 200 && ng < 100 && nb < 100)) {
                            isBoundary = true;
                            perimeter++;
                        }
                    }
                }
            }
        }
    }

    if (count > 0 && circularity(count, perimeter) > CIRCULARITY_THRESHOLD) {
        sunX = sumX / count;
        sunY = sumY / count;
        sunXs.push_back(sunX);
        sunYs.push_back(sunY);
    }

    return (double)count / (image.width * image.height);
}

void moveToSunrisePosition() {
    if (sunXs.size() >= MIN_POINTS) {
        int avgSunriseX = 0, avgSunriseY = 0;
        for (int i = 0; i < sunXs.size(); i++) {
            avgSunriseX += sunXs[i];
            avgSunriseY += sunYs[i];
        }
        avgSunriseX /= sunXs.size();
        avgSunriseY /= sunYs.size();

        int panelX, panelY;
        get_aim(panelX, panelY);
        double angle = atan2(avgSunriseY - panelY, avgSunriseX - panelX);
        move_aim(angle);
    }
}

int main() {
    ImagePPM image;
    make_image(900, 900);  // Assuming default image size as 900x900 based on the content of image_pr4.h

    while (true) {
        std::cout << "Main loop iteration..." << std::endl;

        int sunX, sunY;
        double quality = detectSunAndLog(sunX, sunY, image);

        if (quality > QUALITY_THRESHOLD) {
            if (isDaytime(sunY, image.height)) {
                std::cout << "Detected sun at (" << sunX << ", " << sunY << "). Adjusting panel..." << std::endl;
                int panelX, panelY;
                get_aim(panelX, panelY);
                double angle = atan2(sunY - panelY, sunX - panelX);
                move_aim(angle);
            } else {
                std::cout << "It's nighttime. Moving panel to sunrise position..." << std::endl;
                moveToSunrisePosition();
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    return 0;
}
