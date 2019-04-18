#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <vector>
#include <unistd.h>

#define NOGUI
#undef NOGUI

#define BSIZE 	200
#define ROWS 	100
#define COLS 	480
#define AVG	 	0.85
#define sqr(x)	((x)*(x))

// See if a point is w/in a radius of another point in 2d
int in_range2d(int x1, int y1, int x2, int y2, int rad);

// See if a point is w/in range of another point in 1d
int in_range1d(int x1, int x2, int rad);

int main(const int argc, const char **argv)
{
	// Local var
	cv::VideoCapture cap;
#ifndef NOGUI
	cv::String window_name;
#endif
	cv::Mat frame, roi, element;
	cv::Vec3b pix;
	uint64_t fct;
	std::vector <cv::Vec4i> lines;
	cv::Vec4i l, lprev;
	cv::Ptr <cv::CLAHE> my_clahe;
	cv::Point mid;
	int cx, cy, cxp, cyp, x, y, bxy, thi;
	double ccy, ccx;

	// Get the file to process
	if(argc < 2)
	{
		printf("You must provide a video file to load\n");
		return 0;
	}

	// Init capture
	cap = cv::VideoCapture(argv[1]);
	if(!cap.isOpened())
	{
		std::cout << "Couldn't open video capture device...\n";
		return 1;
	}

	// Start displaying
#ifndef NOGUI
	window_name = cv::String("Capture Device Stream");
	namedWindow(window_name, CV_WINDOW_AUTOSIZE);
#endif

	// We're doing a running avg algorithm for the line
	// detection, so we'll init the line to just some value
	lprev[0] = 0;
	lprev[1] = 0;
	lprev[2] = 959;
	lprev[3] = 0;

	my_clahe = cv::createCLAHE();
	my_clahe->setClipLimit(4);
	my_clahe->setTilesGridSize(cv::Size(8,8));
	element = getStructuringElement(cv::MORPH_RECT, 
			cv::Size(15, 15));
	mid.x = 100;

	// Loop through 18000 frames
	for(fct = 0; fct < 18000;fct++, cap.read(frame))
	{
		// If the capture actually read a frame
		if(!frame.empty())
		{
			// Do some post processing
			cv::resize(frame, frame, cv::Size(frame.cols/2, frame.rows/2));
			roi = frame(cv::Rect(0,0,960,400));
			cvtColor(roi, roi, CV_BGR2GRAY);
			cv::blur(roi, roi, cv::Size(5,5));

			my_clahe->apply(roi, roi);
			cv::threshold(roi, roi, 220, 255, cv::THRESH_BINARY);
			cv::erode(roi, roi, element);
			//cv::Canny(roi, roi, 35, 45, 3);
			cxp = cx;
			cyp = cy;
			bxy = cx = cy = 0;
			for(x = 0; x < roi.cols; x++)
			{
				for(y = 0; y < roi.rows; y++)
				{
					if(roi.at<uchar>(y,x))	
					{
						bxy++;
						cx += x;
						cy += y;
					}
				}
			}
			if(bxy)
			{
				ccx = (double)cx/(double)bxy;
				ccy = (double)cy/(double)bxy;
				cx /= bxy;
				cy /= bxy;
				mid.y = cy;
			}
			printf("%lf\n", ccy);

			// Run the probabilistic hough transform
			//HoughLinesP(roi, lines, 3, CV_PI/180, 60, 10, 30);
			cvtColor(roi, roi, CV_GRAY2BGR);

#ifndef NOGUI
#if 0
			for(thi = 0; thi < lines.size(); thi++)
			{
				lprev = lines[thi];
				// Draw a line
				line(roi, cv::Point(lprev[0], lprev[1]), 
						cv::Point(lprev[2], lprev[3]), cv::Scalar(0,0,255), 3, CV_AA);
			}
#endif
			// Show the frame
			cv::circle(roi, mid, 5, cv::Scalar(255, 0, 0), -1);
			imshow(window_name, roi);
#endif
		}
#ifndef NOGUI
		// If they hit escape, then exit
		if(cv::waitKey(10) == 27)
		{
			break;
		}
#endif
	}

	return 0;
}

int in_range2d(int x1, int y1, int x2, int y2, int rad)
{
	// Just the equation for a 2d sphere
	return sqr((double)x2 - (double)x1) +
		sqr((double)y2 - (double)y1) <= sqr((double)rad);
}

int in_range1d(int x1, int x2, int rad)
{
	// Just the equation for a 1d sphere
	return sqr((double)x1 - (double)x2) <= sqr((double)rad);
}
