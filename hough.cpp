#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <vector>

#define NOGUI
#undef NOGUI

#define BSIZE 	200
#define ROWS 	100
#define COLS 	480
#define AVG	 	0.85

int main(const int argc, const char **argv)
{
	// Local var
	cv::VideoCapture cap;
#ifndef NOGUI
	cv::String window_name;
#endif
	cv::Mat frame, roi;
	cv::Vec3b pix;
	uint64_t fct;
	std::vector <cv::Vec4i> lines;
	cv::Vec4i l, lprev;
	int line_y;

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

	// Loop through 18000 frames
	for(fct = 0; fct < 18000;fct++, cap.read(frame))
	{
		// If the capture actually read a frame
		if(!frame.empty())
		{
			// Do some post processing
			cv::resize(frame, frame, cv::Size(frame.cols/2, frame.rows/2));
			roi = frame(cv::Rect(0,0,960,400));
			cv::blur(roi, roi, cv::Size(5,5));
			cvtColor(roi, roi, CV_BGR2GRAY);
			cv::Canny(roi, roi, 35, 45, 3);

			// Run the probabilistic hough transform
			HoughLinesP(roi, lines, 3, CV_PI/180, 60, 300, 10);
			cvtColor(roi, roi, CV_GRAY2BGR);

			// If we found any lines at all
			if(lines.size())
			{
				// The 0th line is the highest likely line, so
				// we'll assume that it's the one we want
				l = lines[0];

				/// And do the running average
				lprev[0] = AVG*lprev[0] + (1-AVG)*l[0];
				lprev[1] = AVG*lprev[1] + (1-AVG)*l[1];
				lprev[2] = AVG*lprev[2] + (1-AVG)*l[2];
				lprev[3] = AVG*lprev[3] + (1-AVG)*l[3];
			}
			else
			{
				// If we didn't find a line, don't update it
				l = lprev;
			}
			// Find the y coordinate
			line_y = (lprev[1] + lprev[3])>>1;
#ifndef NOGUI
			// Draw a line
			line(roi, cv::Point(lprev[0], lprev[1]), cv::Point(lprev[2], lprev[3]), cv::Scalar(0,0,255), 3, CV_AA);

			// Show the frame
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
