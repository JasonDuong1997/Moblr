/* * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* res.cpp * * * * * * * * * * * * * * * * * * * * * * */
/* Created by: Jordan Bonecutter * * * * * * * * * * * */
/* 28 April 2019 * * * * * * * * * * * * * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Includes
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <assert.h>
#include <stdio.h>
#include <vector>
#include <math.h>

// Defn's
#define ERR_ARGS	1
#define ERR_MOV		2
#define SUCCESS		0
#define P_GUI
#undef  P_GUI
#define ROWS		243
#define DECAY 		1.03
#define RNG			70
#define RNGDLT		15
#define THRESH		0.9
#define BINT		240
#define INIT_TIME	100
#define CSIZE		6
#define DLTRFSH		2
#define cub(x)		((x)*(x)*(x)*(x))

// Functions
int main         (const int argc, const char **argv);
int process_movie(const char *fname);

// Main function
int main(const int argc, const char **argv)
{
	// Check if arguments are provided
	if(argc != 2)
	{
		// Tell 'em
		printf("Usage error: %s <filename>\n", argv[0]);
		return ERR_ARGS;
	}
	// Try to process the movie
	else if(process_movie(argv[1]) != SUCCESS)
	{
		printf("Movie file %s not found\n", argv[1]);
		return ERR_MOV;
	}

	return SUCCESS;
}

static void on_mouse(int event, int x, int y, int, void *)
{
	printf("%d,%d\n", x, y);
	return;
}

int process_movie(const char *fname)
{
	// Assert
	assert(fname);

	// Local var
	cv::Mat frame, heq, vis, kernel, orig, hough, el;
	cv::VideoCapture cap;
	#ifdef P_GUI
	cv::String window_name[2];
	#endif
	cv::Ptr <cv::CLAHE> hist_equalizer;
	float rows[ROWS], maxr;
	int x, y, max_y, max_yp, bxy, cy, cyp, fct, li, rng, ci, csum;
	int conv[CSIZE];
	float max;
	std::vector <cv::Vec4i> lines;

	FILE *fp = fopen("out_new.csv", "w");

	// Init cap
	cap = cv::VideoCapture(fname);
	if(!cap.isOpened())
	{
		return ERR_MOV;
	}

	// Set the kernel
	kernel = cv::Mat(90, 90, CV_32FC1);
	for(y = 0; y < 15; y++)
	{
		for(x = 0; x < 90; x++)
		{
			kernel.at<float>(y,x)    = -0.75;
			kernel.at<float>(89-y,x) = -0.75;
		}
	}
	for(y = 15; y < 30; y++)
	{
		for(x = 0; x < 90; x++)
		{
			kernel.at<float>(y,x)    = -0.25;
			kernel.at<float>(89-y,x) = -0.25;
		}
	}
	for(y = 30; y < 60; y++)
	{
		for(x = 0; x < 90; x++)
		{
			kernel.at<float>(y,x) = 1.0;
		}
	}

	#ifdef P_GUI
	// Open a window for the GUI
	window_name[0] = cv::String("Graph");
	window_name[1] = cv::String("Detect");
	namedWindow(window_name[0], CV_WINDOW_AUTOSIZE);
	namedWindow(window_name[1], CV_WINDOW_AUTOSIZE);
	#endif

	// Initialize loop var
	hist_equalizer = cv::createCLAHE();
    hist_equalizer->setClipLimit(15);
    hist_equalizer->setTilesGridSize(cv::Size(5,5));
	memset(rows, 0, sizeof(rows));
	cyp = fct = max_yp = ci = csum = 0;
	rng = RNG + RNGDLT;
	el = getStructuringElement(cv::MORPH_RECT, cv::Size(10, 10));

	// Loop thru the movie
	for(fct = 0; cap.isOpened();)
	{
		// Read the frame
		cap.read(orig);
		if(orig.empty())
		{
			continue;
		}
		if(fct++%500 == 0)
		{
			printf("%d\n", fct);
		}
		rng -= rng > RNG;
		
		// Let's do some processing!

		// Resize
		cv::resize(orig, orig, cv::Size(orig.cols/2, orig.rows/2));
		orig = orig(cv::Rect(0, 0, orig.cols, orig.rows-54));
		cvtColor(orig, frame, CV_BGR2GRAY);
		hist_equalizer->apply(frame, frame);
		frame.convertTo(frame, CV_32FC1, 1/255.0);

		// Do the specialized 
		cv::filter2D(frame, heq, -1, kernel, cv::Point(-1,-1), 0, cv::BORDER_REPLICATE);

		// Get avg row brightness
		maxr = -100000;
		for(y = 0; y < ROWS; y++)
		{
			// Time average the frames
			rows[y] /= DECAY;
		}
		for(y = 0; y < heq.rows; y++)
		{
			for(x = 0; x < heq.cols; x++)
			{
				// Calculate the average row value
				rows[(y*(ROWS-1))/(heq.rows-1)] += cub(fabs(heq.at<float>(y,x)));
			}
		}
		// Remember the previous y value
		max_yp = max_y;
		for(y = 0; y < ROWS; y++)
		{
			if(rows[y] > maxr)
			{
				// Find the row of maximum brightness
				maxr  = rows[y];
				max_y = (y*(heq.rows-1))/(ROWS-1);
			}
		}
		// If we changed the value significantly, 
		// increase the search range 
		if(abs(max_y-max_yp) > DLTRFSH)
		{
			rng = RNG + RNGDLT;
		}

		// If we think there might have been an error,
		// try using Jason's hough transform method and
		// see if it finds something good
		if(abs(max_y - max_yp) > rng && fct > INIT_TIME)
		{
			// Clear the convultion buffer
			ci = 0;

			// Run Jason's algorithm
			cvtColor(orig, hough, CV_BGR2GRAY);
			cv::medianBlur(hough, hough, 25);
			cv::Canny(hough, hough, 40, 120, 3);
			lines.clear();
			HoughLinesP(hough, lines, 3, CV_PI/180, 60, 300, 10);
			
			// Go through the lines and see if there's one that's 
			// closer
			for(li = 0; li < lines.size(); li++)
			{
				if(abs(lines[li][1]-max_yp) < rng)
				{
					max_y = lines[li][1];
					rng = RNG + RNGDLT;
					break;
				}
			}
		}

		for(y = max_y < 5 ? 0 : max_y - 5; y < (max_y > ROWS-5 ? ROWS : max_y - 5); y++)
		{
			rows[y] += maxr*3/heq.cols;
		}
		vis = cv::Mat(heq.rows/2, heq.cols/2, CV_8UC1);
		memset(vis.data, 0, sizeof(unsigned char)*(vis.rows*vis.cols));
		for(x = 0; x < vis.cols; x++)
		{
			y = vis.rows-1-(rows[(x*(ROWS-1))/(vis.cols-1)]*(vis.rows-1))/maxr;
			vis.at<uchar>(y,x) = 0xFF;
		}

		// Fix convolution so it's viewable
		max = -100000;
		for(x = 0; x < heq.cols; x++)
		{
			for(y = 0; y < heq.rows; y++)
			{
				if(fabs(heq.at<float>(y,x)) > max)
				{
					max = fabs(heq.at<float>(y,x));
				}
			}
		}
		cyp = cy;
		bxy = cy = 0;
		frame.convertTo(frame, CV_8UC1, 255.0);
		cv::threshold(frame, frame, 240, 255, cv::THRESH_BINARY);
		cv::erode(frame, frame, el);
		cv::dilate(frame, frame, el);
		for(x = 0; x < heq.cols; x++)
		{
			for(y = max_y < rng ? 0 : max_y - rng; 
					y < (max_y > heq.rows-rng ? heq.rows : max_y + rng); y++)
			{
				if(frame.at<float>(y,x))
				{
					bxy++;
					cy+=y;
				}
			}
		}
		if(bxy)
		{
			cy /= bxy;
			if(ci < CSIZE-1)
			{
				csum = cy;
				conv[ci++] = cy;
			}
			else if(ci == CSIZE-1)
			{
				csum = 0;
				for(x = 0; x < CSIZE-1; x++)
				{
					csum += conv[x];
				}
				csum += cy;
				conv[ci%CSIZE] = cy;
				ci++;
			}
			else
			{
				csum -= conv[ci%CSIZE];
				csum += cy;
				conv[ci%CSIZE] = cy;
				ci++;
			}
		}
		else
		{
			cy = cyp;
		}
		fprintf(fp, "%d, %d\n", fct, heq.rows-1-csum/CSIZE);
		if(cy < max_y - RNG/2 || cy > max_y + RNG/2)
		{
			rng = RNG + RNGDLT;
		}

		// Show the frame
		#ifdef P_GUI
		cv::line(orig, cv::Point(0,(csum/CSIZE)), 
				cv::Point(orig.cols-1,(csum/CSIZE)),
				cv::Scalar(255,0,0),cv::LINE_8,0);
		cv::line(orig, cv::Point(0,(max_y > rng ? max_y - rng : 0)), 
				cv::Point(orig.cols-1,(max_y > rng ? max_y - rng : 0)),
				cv::Scalar(0,0,255),cv::LINE_8,0);
		cv::line(orig, cv::Point(0,(max_y < orig.rows - 1 - rng ? max_y + rng : orig.rows-1)), 
				cv::Point(orig.cols-1,(max_y < orig.rows - 1 - rng ? max_y + rng : orig.rows-1)),
				cv::Scalar(0,0,255),cv::LINE_8,0);
		imshow(window_name[0], vis);
		imshow(window_name[1], orig);
		#endif

		// If they hit escape, then exit
        if(cv::waitKey(10) == 27)
        {
            break;
        }
	}
	fclose(fp);

	return SUCCESS;
}
