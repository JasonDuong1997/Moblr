import matplotlib
import matplotlib.pyplot as plt


def extract_data(filename):
	file = open(filename, "r")

	data = []

	# getting rid of first line
	line = file.readline()

	# extracting rest of the data
	while(True):
		line = file.readline()
		if (line == ""):
			break
		else:
			line = line.split(",")
			data.append([float(line[0]), float(line[1])])

	file.close()
	return data


def clean(data):
	print(data)
	for item in data:
		if item[1] == -1:
			del item
	return data


def normalize(data, y_max):
	length = len(data)
	x_max = data[length-1][0]

	x_list = [item[0]/x_max for item in data]
	y_list = [item[1]/y_max for item in data]

	normalized_data = []
	for i in range(length):
		normalized_data.append([x_list[i], y_list[i]])

	return normalized_data


def graph(data, color, name):
	x_data = [item[0] for item in data]
	y_data = [item[1] for item in data]

	plt.plot(x_data, y_data, color, label=name)



if __name__ == '__main__':

	print("\n"
		  "This program will graph both the truth data and algorithm results"
		  "onto the same graph with a normalized x and y axis.\n"
		  "To run the program:\n"
		  "1. Must have the truth data file named \"truth_data.csv\" in same directory.\n"
		  "2. Must have algorithm results file (.csv) in same directory\n"
		  "3. Must know the maximum y-value for your algorithm results (i.e. height of your cv2 window)"
		  "\n")

	# truth data
	filename1 = "truth_data.csv"
	data1 = extract_data(filename1)
	normalized_data1 = normalize(data1, 10)
	graph(normalized_data1, "r", "Hand-Measured Result")

	# algorithm results
	filename2 = input("Input \".csv\" file here: ")
	y_max = input("Input maximum y-value here: ")
	data2 = extract_data(filename2)
	normalized_data2 = normalize(data2, int(y_max))
	graph(normalized_data2, "g", "Histogram-Calculated Result")

	# configuring graph
	plt.title("Lane Detection Results")
	plt.xlabel("Time (Normalized)")
	plt.ylabel("Distance to Lane (Normalized)")
	fig = matplotlib.pyplot.gcf()
	fig.set_size_inches(16, 8)
	plt.legend(loc='upper right')
	plt.grid(True)
	plt.show()