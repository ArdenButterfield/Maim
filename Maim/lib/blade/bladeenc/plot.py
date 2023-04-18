import matplotlib.pyplot as plt 

with open('test.txt') as f:
	lines = f.readlines()
lines = [float(l) for l in lines]
plt.plot(lines)
plt.show()