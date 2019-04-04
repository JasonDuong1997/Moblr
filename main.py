import time
import cv2
import numpy as np

from video_object import Video
import image_processing as Image

# Must have this video file in the same directory"
file_name = "GP010020.MP4"

def main():
	# Reading Video
	video = Video(file_name)

	# Memory of previous lines
	line_list = []
	line_coords = []

	frame_count = 1

	while(True):
		# Get next frame in the video
		image = video.get_frame()
		image = Image.resize(image)

		# Display Original Frame from the video
		cv2.imshow("1. Original Video", image)

		# Display the frame after edge detection has been applied to it
		edge_image = Image.detect_edges(image)
		cv2.imshow("2. Edge-Detected Video", edge_image)

		# Display the frame with all lines detected
		line_image = Image.detect_lines(np.copy(image), np.copy(edge_image))
		cv2.imshow("3. Line-Detected Image", line_image)

		# Display the frame with filtered lines detected
		line_list, line_coords = Image.detect_lane_lines(image, edge_image, line_list, line_coords, frame_count, 5)
		cv2.imshow("4. Final Image", image)

		frame_count = frame_count + 1
		time.sleep(0)

		# To end the process, click on any of the display windows and press "q" on the keyboard.
		if cv2.waitKey(25) & 0xFF == ord('q'):
				video.file.release()
				cv2.destroyAllWindows()
				break


if __name__ == '__main__':
	main()
