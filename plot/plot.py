import matplotlib.pyplot as plt

# Data
n = [100, 200, 300, 400]
time_V92_F6 = [167, 308, 481, 616]
time_V382_F6 = [655, 1263, 1843, 2484]
time_V382_F24 = [1484, 2990, 4326, 5840]

# Create a new figure with a specified size
plt.figure(figsize=(10, 6))

# Plot the data for each case
plt.plot(n, time_V92_F6, marker='o', linestyle='-', label='V=92, F=6')
plt.plot(n, time_V382_F6, marker='s', linestyle='--', label='V=382, F=6')
plt.plot(n, time_V382_F24, marker='^', linestyle='-.', label='V=382, F=24')

# Add title and labels
plt.title('Execution Time vs. Number of Walks (n)')
plt.xlabel('Number of Walks (n)')
plt.ylabel('Execution Time (ms)')

# Add grid for better readability
plt.grid(True, linestyle='--', linewidth=0.5)

# Add legend to distinguish different lines
plt.legend()

# Display the plot
plt.show()