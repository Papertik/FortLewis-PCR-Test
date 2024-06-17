import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
from matplotlib.ticker import MultipleLocator

# Load data from CSV into a pandas DataFrame
csv_file = 'test_data/high(35,0,1400)_clamp(225).csv'
df = pd.read_csv(csv_file)

# Convert time from milliseconds to seconds
df['Time_s'] = df['Time'] / 1000
df['High_point'] = df['Setpoint'] + 0.3
df['Low_point'] = df['Setpoint'] - 0.3

# Calculate the average temperature
average_temp = df['Temperature'].mean()

# Function to calculate rise time
def calculate_rise_time(df, setpoint):
    setpoint_times = df.loc[df['Setpoint'] == setpoint, 'Time_s']
    if setpoint_times.empty:
        return None
    start_time = setpoint_times.iloc[0]
    target_temp = setpoint  # 90% of the setpoint
    rise_time_data = df[(df['Setpoint'] == setpoint) & (df['Temperature'] >= target_temp)]
    if rise_time_data.empty:
        return None
    rise_time = rise_time_data['Time_s'].iloc[0] - start_time
    return rise_time, start_time + rise_time

# Calculate rise times for each setpoint
setpoints = df['Setpoint'].unique()
rise_times = {setpoint: calculate_rise_time(df, setpoint) for setpoint in setpoints}

# Set the style and context for Seaborn
sns.set(style="whitegrid")

# Plotting
plt.figure(figsize=(10, 6))
sns.lineplot(x='Time_s', y='Temperature', data=df, label=f'Temperature (°C) (Avg: {average_temp:.2f} °C)')
sns.lineplot(x='Time_s', y='Setpoint', data=df, label='Setpoint (°C)', linestyle='--', color='red')
sns.lineplot(x='Time_s', y='High_point', data=df, label='High', linestyle='--', color='green')
sns.lineplot(x='Time_s', y='Low_point', data=df, label='Low', linestyle='--', color='green')

# Annotate rise times on the plot
for setpoint, rise_time_data in rise_times.items():
    if rise_time_data:
        rise_time, end_time = rise_time_data
        plt.annotate(f'Rise time: {rise_time:.2f}s',
                     xy=(end_time, setpoint * 0.9),
                     xytext=(end_time + 10, setpoint * 0.9 + 1),
                     arrowprops=dict(facecolor='black', shrink=0.05),
                     fontsize=9, color='black')

# Customize the plot
plt.xlabel('Time (s)')
plt.ylabel('Temperature (°C)')
plt.title('high(30,0.015,1100)_clamp(225).csv')
plt.legend()

# Set y-axis to have ticks every 2 degrees
plt.gca().yaxis.set_major_locator(MultipleLocator(2))

# Set x-axis to have ticks every 30 seconds
plt.gca().xaxis.set_major_locator(MultipleLocator(30))

# Enable gridlines
plt.grid(True, which='both', linestyle='--', linewidth=0.5)

plt.tight_layout()

# Show plot
plt.show()
