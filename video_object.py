import cv2

class Video:
	def __init__(self, file_name):
		self.file = cv2.VideoCapture(file_name)		# video object
		self.frame = None								# individual frame of the video selected

	#	INPUT: 	Video
	#	OUTPUT: Frame of Video
	#	DESCRIPTION: Attempts to grab the next frame in the video. If successful, it returns the frame. If not, it attempts the next frame.
	def get_frame(self):
		success, frame = self.file.read() # initial read
		if (success):
			return frame
		else:
			while(not success):
				success, frame = self.file.read()
			return frame

