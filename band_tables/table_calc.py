centers = [50,150,250,350,450,570,700,840,1000,1170,1370,1600,1850,2150,2500,2900,3400,4000,4800,5800,7000,8500,10500,13500]
bandwidths = [100,100,100,100,110,120,140,150,160,190,210,240,280,320,380,480,550,700,900,1100,1300,1800,2500,3500]
LINES = 576
def generate(samplerates):

	print("// samplerates:", samplerates)
	print()
	print("{")
	for s in samplerates:
		print("\t{")
		print("\t\t",end="")
		for i in range(22):
			low_f = centers[i] - (bandwidths[i] / 2)
			l = low_f * LINES / (s / 2)
			print(f"{int(l)}{', ' if i < 21 else ''}", end="")
		print("\n\t}")
	print("}")

generate([32000,44100,48000])