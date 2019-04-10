import time
import cv2
import csv
import numpy as np
import matplotlib.pyplot as plt

from video_object import Video
import image_processing as Image

# Must have this video file in the same directory"
file_name = "GP010020.MP4"

# Switches
graph = False
save_data = False
display = False

def main():
	factor = 0.3

	# Reading Video
	video = Video(file_name)
	global height, width, shape
	height, width, shape = video.get_frame().shape

	# Memory of previous lines
	line_list = []
	line_coords = []
	line_distances = []

	if (graph):
		# configuring graph for plotting
		plt.figure(figsize=(15,8))
		plt.axis([0, video.n_frames, 0, height])
		plt.grid(True)

	if (save_data):
		file = open("data.csv", 'w')
		writer = csv.writer(file)

	for frame_count in range(0, video.n_frames):
		# Get next frame in the video
		image = video.get_frame()
		image = Image.resize(image, factor)

		# Display Original Frame from the video
		if (display):
			cv2.imshow("1. Original Video", image)

		# Display the frame after edge detection has been applied to it
		edge_image = Image.detect_edges(image)
		if (display):
			cv2.imshow("2. Edge-Detected Video", edge_image)

		# Display the frame with all lines detected
		line_image = Image.detect_lines(np.copy(image), np.copy(edge_image))
		if (display):
			cv2.imshow("3. Line-Detected Image", line_image)

		# Display the frame with filtered lines detected
		line_list, line_coords = Image.detect_lane_lines(image, edge_image, line_list, line_coords, frame_count, 5)
		if (display):
			cv2.imshow("4. Final Image", image)

		# Calculating the distance from the car to the lane (in pixels)
		for line_coord in line_coords:
			line_distances.append(line_coord[1])	# y1
			line_distances.append(line_coord[3])	# y2
		try:
			lane_distance = (height - int(max(line_distances)/factor))
			print("Frame [{}]: Distance from car to lane line: {}" .format(frame_count, lane_distance))
			line_distances.clear()
			if (save_data):
				writer.writerow([frame_count, lane_distance])
			if (graph):
				# plotting lane distance of current frame
				plt.scatter(frame_count, lane_distance)
				plt.pause(0.05)
		except ValueError:
			print("No Lines Found!")

		time.sleep(0)

		# To end the process, click on any of the display windows and press "q" on the keyboard.
		if cv2.waitKey(25) & 0xFF == ord('q'):
				video.file.release()
				cv2.destroyAllWindows()
				break

	if (graph):
		plt.show()

if __name__ == '__main__':
	main()
