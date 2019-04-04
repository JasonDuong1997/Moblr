import numpy as np
import cv2

#	INPUT: 	Image
#	OUTPUT:	Resized Image
#	DESCRIPTION: Currently set to reduce the size of the input image to 30% of both width and height dimensions.
def resize(src_image):
	return cv2.resize(src_image, (0, 0), fx=0.3, fy=0.3)


#	INPUT: 	Image
#	OUTPUT:	Edge-Detected Image
#	DESCRIPTION: Converts image to grey-scale, blur the image so that edges will be smoother to find,
#		and then finally apply edge detection.
def detect_edges(src_image):
	image = cv2.cvtColor(src_image, cv2.COLOR_BGR2GRAY)
	image = cv2.GaussianBlur(image, (7,7), 0)
	image = cv2.Canny(image, threshold1=0, threshold2=75)
	return image


# 	INPUT: 	Original Image, Edge-Detected Image
#	OUTPUT:	Line-Detected Image
#	DESCRIPTION: Applies line detection on edge-detected image and then overlays the 2 strongest lines onto the original image.
def detect_lane_lines(src_image, edge_image, line_list, line_coords, frame_count, frames_per_activation):
	line_min = 140
	gap_max = 2


	# if not the activation frame, add new lines to line_list
	if (frame_count%frames_per_activation != 0):
		try:
			for line in cv2.HoughLinesP(edge_image, 1, np.pi/180, 100, np.array([]), line_min, gap_max):
				line_list.append(line)
		except TypeError:
			pass
		finally:
			for line_coord in line_coords:
				cv2.line(src_image, (line_coord[0], line_coord[1]), (line_coord[2], line_coord[3]), (255, 0, 0), thickness=4)
	else:
		line_coords_new, filtered_lines = filter_lines(src_image, line_list)
		if (line_coords_new):
			line_coords = line_coords_new
		for line_coord in line_coords:
			cv2.line(src_image, (line_coord[0], line_coord[1]), (line_coord[2], line_coord[3]), (255, 0, 0), thickness=4)
		line_list.clear()

	return line_list, line_coords


# 	INPUT: 	Original Image, Edge-Detected Image
#	OUTPUT:	Line-Detected Image
#	DESCRIPTION: Applies line detection on edge-detected image and then overlays those lines onto the original image.
def detect_lines(src_image, edge_image):
	line_min = 140
	gap_max = 2

	try:
		for line_coord in cv2.HoughLinesP(edge_image, 1, np.pi / 180, 100, np.array([]), line_min, gap_max):
			cv2.line(src_image, (line_coord[0][0], line_coord[0][1]), (line_coord[0][2], line_coord[0][3]), (255, 100, 100), thickness=2)
	except TypeError:
		pass

	return src_image


#	INPUT: List of All Lines from detect_lines()
#	OUTPUT: Only important lines related to the lanes
#	DESCRIPTION: Currently a simple filter that removes the lines detected from the car door in the frame
def filter_lines(src_image, lines):
	height, width, channels = src_image.shape

	# filter out lines detected from edges of car door in frame
	filtered_lines = []
	for line in lines:
		if (line[0][1] < (6/8)*height or line[0][3] < (6/8)*height):
			filtered_lines.append(line)

	# compile line information to list
	line_info_list = []
	for line in filtered_lines:
		x1 = line[0][0]
		y1 = line[0][1]
		x2 = line[0][2]
		y2 = line[0][3]

		if (int(x2 - x1) != 0):
			slope = int((y2 - y1)/(x2 - x1))
			y_int = int(y2 - slope*x2)
			line_info_list.append([slope,y_int])

	print("Number of lines found: {}" .format(len(line_info_list)))
	print("Line List: {}" .format(line_info_list))

	threshold = .05
	line_aggregator = []
	for i in range(0, len(line_info_list)):
		key_slope = line_info_list[i][0]
		key_y_int = line_info_list[i][1]

		# initial condition
		if (len(line_aggregator) == 0):
			line_aggregator.append([key_slope, key_y_int, 1])
		else:
			# checking key line with lines in aggregator
			for line_item in line_aggregator:
				slope_match = False
				y_int_match = False

				true_slope = line_item[0]
				true_y_int = line_item[1]

				if (abs(true_slope) < 0.01):
					if (abs(key_slope) < 0.01):
						slope_match = True
					else:
						slope_match = False
				elif (abs(key_slope - true_slope)/true_slope < threshold):
					slope_match = True

				if (abs(key_y_int - true_y_int)/true_y_int < threshold):
					y_int_match = True
				else:
					y_int_match = False

				if (slope_match and y_int_match):
					line_item[0] = int((4*line_item[0] + key_slope)/5)
					line_item[1] = int((4*line_item[1] + key_y_int)/5)
					line_item[2] = line_item[2] + 1
					break
			if (not (slope_match and y_int_match)):
				line_aggregator.append([int(key_slope), int(key_y_int), 0])

	print("Line_Aggregator Count: {}" .format(len(line_aggregator)))
	print("Line Aggregator: {}" .format(line_aggregator))

	# sorting the list by highest line votes
	line_aggregator.sort(reverse=True, key=line_count)
	print("Line Aggregator Sorted: {}" .format(line_aggregator))

	# converting each line to (x,y) coordinates on each side of the screen
	line_coords = []
	count = 0
	for line in line_aggregator:
		if (count < 2):
			line_coords.append([0, int(line[1]), width, int(line[0]*width + line[1])])
			count = count + 1
		else:
			break

	return line_coords, filtered_lines


def line_count(line):
	return line[2]