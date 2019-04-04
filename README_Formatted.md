# Moblr
## How to Run Program
1. must have the road test footage named "GP010020.mp4" in this directory
2. run command "python main.py"
## Files Involved
*video_object.py*      just makes it easier to load in the .mp4 video and read each frame in the video sequentially<br>
*image_processing.py*  is where the magic happens with edge_detection and line_detection<br>
*GP010020.mp4*         is the test footage<br>
*main.py*              is program driver that displays 4 windows:<br>
<ol>
<li>Original Video: original, raw input video</li>
<li>Edge-Detected Video: intermediary video that takes in the Original Video and runs detect_edges()</li>
<li>Line-Detected Video: intermediary video that takes in Edge-Detected Video and runs detect_lines()</li>
<li>Final Video: final video that takes in Edge-Detected Video and runs detect_lane_lines()</li>
</ol>

## How the Program Works
To explain how the program works, we will track what happens to an individual frame of the test footage.
### I. The test footage is loaded into the program"
The first frame is read.
*The original frame is displayed.*
### II. The original frame is fed into **detect_edges()**:
    frame converted to grey-scale
    grey-scale image is blurred slightly to improve edge detection
    grey-scale image is fed into Canny Edge detection
    edge-detected image returned
    *The edge-detected image is displayed.*
### III. The edge-detected image is fed into detect_lines():
    edge-detected image is fed into Hough Line detection
    all of the lines returned from Hough Line Detection is overlayed onto the original frame
    *The line-detected image is displayed*
### IV. The edge-detected image is fed into detect_lane_lines():
    the lines detected across multiple frames is added to a line_list aggregator
    after every X amount of frames, the line_list is fed into filter_lines():
    the lines detected from the body of the car are ignored since we don't care about those
    similar lines are grouped together based on slope & y-intercept
    the line are sorted based on number of duplicates
    up to two of the top two lines with the highest number of duplicates are returned
    *The lane_line-detected image is displayed.*
