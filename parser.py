import sys
import numpy as np
import pylab as pl
import scipy.stats as stats

pl.rcParams.update({'font.size': 18})

# Specify cache partition size for this plot
part_size = '1.5 MB'
data_set = 'CIF'
bench_name = 'Localization'
plot_data = 'cif_1_5'

# Specify the name for saving the plot to disk as a PNG file
fig_name = bench_name + '_' + data_set + '_1_5_PDF.png'

# Create a dictionary to store parsed data
disparity_data = {}
disparity_data[plot_data] = []

# Open the file for reading data
try:
    fd = open('../perf_logs/localization_cif_1_5mb.log', 'r')
except:
    print "File Not Found!"
    sys.exit(-1)

lines = fd.readlines()

# Close the data file
fd.close()

# Look for the line in the log file which contains miss-rate
# information
for line in lines:
    if '%' in line:
        left_side = line.split('%')
        split_string = left_side[0].split(' ')
        miss_rate = split_string[-2]
        miss_rate = float(miss_rate)
        disparity_data[plot_data].append(miss_rate)
        
# Sort the data
mr_array = sorted(disparity_data[plot_data])

# Calculate mean and standard deviation
mr_mean = np.mean(mr_array)
mr_std = np.std(mr_array)

# Fit a normal curve on the miss-rate data
fit = stats.norm.pdf(mr_array, mr_mean, mr_std)

# Create a figure to contain the plot
fig = pl.figure(1, figsize = (25, 15))

# Plot the normal curve
pl.plot(mr_array, fit, '-o')

# Overlay data-histogram on top of normal curve
pl.hist(mr_array, normed = True, bins = np.arange(0, mr_array[-1], 0.5))

# Annotate the maximum miss-rate on the plot
pl.plot((mr_array[-1], mr_array[-1]), (0, 1), 'r', label = 'Max Miss-Rate')

# Specify x and y labels
pl.xlabel('Miss-Rate', fontsize = 20)
pl.ylabel('Probability', fontsize = 20)

# Specify limits for x and y axes
pl.xlim(0, (mr_array[-1]+2))
pl.ylim(0, 0.5)

# Create a title for the plot
pl.title('Benchmark : ' + bench_name + ' | Data-Set : ' + data_set + ' | Partition Size : ' + part_size + ' | Mean : ' + "{0:0.3f}".format(mr_mean) + ' | Std. : ' + "{0:0.3f}".format(mr_std) + ' | Max : ' + "{0:0.3f}".format(mr_array[-1]), fontsize = 20)

fig.savefig(fig_name)
