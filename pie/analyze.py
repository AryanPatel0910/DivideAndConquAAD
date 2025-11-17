import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os

# --- Helper Function for Log-Log Linear Fit ---
def fit_log_log(x, y):
    """Fits a line to log-log data and returns the slope."""
    # Remove any non-positive values that break log()
    mask = (x > 0) & (y > 0)
    if not np.any(mask):
        return np.nan
        
    log_x = np.log10(x[mask])
    log_y = np.log10(y[mask])
    
    # Fit a 1st degree polynomial (a line)
    # polyfit returns [slope, intercept]
    try:
        slope, intercept = np.polyfit(log_x, log_y, 1)
        return slope
    except np.linalg.LinAlgError:
        return np.nan

# --- Setup ---
plt.style.use('ggplot')
output_dir = "." # Save plots in the current directory

# Load the data
try:
    df = pd.read_csv("data.csv")
except FileNotFoundError:
    print("Error: data.csv not found.")
    print("Please run your C++ program first: ./b > data.csv")
    exit()

# Get the max 'm' and 'n' values for filtering
if df.empty:
    print("Error: data.csv is empty.")
    exit()
    
MAX_M = df['m_trials'].max()
MAX_N = df['n'].max()

print(f"Data loaded. Found max m_trials = {MAX_M} and max n = {MAX_N}")

# --- 1. Analysis: Convergence Rate (Error vs. n) ---
print("\n--- Analysis 1: Convergence Rate (Error vs. n) ---")
fig1, ax1 = plt.subplots(figsize=(10, 6))
data_m_max = df[df['m_trials'] == MAX_M]

ax1.scatter(data_m_max['n'], data_m_max['mean_error'])

# Set log scales
ax1.set_xscale('log')
ax1.set_yscale('log')

# Fit line and print slope
slope_1 = fit_log_log(data_m_max['n'], data_m_max['mean_error'])
print(f"Log-Log Slope (Error vs. n): {slope_1:.4f} (Expected ~ -0.5)")

ax1.set_title(f'Convergence Rate (Error vs. n) at m_trials = {MAX_M}')
ax1.set_xlabel('n (Number of Points)')
ax1.set_ylabel('Mean Error (log scale)')
ax1.grid(True, which="both", ls="--")

fig1.savefig(os.path.join(output_dir, "analysis_1_convergence.png"))
print("Saved analysis_1_convergence.png")


# --- 2. Analysis: Stability (Std Dev vs. n) ---
print("\n--- Analysis 2: Stability (Std Dev vs. n) ---")
fig2, ax2 = plt.subplots(figsize=(10, 6))
# Filter for max 'm' and also m > 1 (since std_dev is 0 for m=1)
data_m_stable = df[(df['m_trials'] == MAX_M) & (df['m_trials'] > 1)]

# Filter out std_dev = 0 to avoid log errors
data_m_stable = data_m_stable[data_m_stable['std_dev'] > 0]

ax2.scatter(data_m_stable['n'], data_m_stable['std_dev'])

# Set log scales
ax2.set_xscale('log')
ax2.set_yscale('log')

# Fit line and print slope
slope_2 = fit_log_log(data_m_stable['n'], data_m_stable['std_dev'])
print(f"Log-Log Slope (Std Dev vs. n): {slope_2:.4f} (Expected ~ -0.5)")

ax2.set_title(f'Stability (Std Dev vs. n) at m_trials = {MAX_M}')
ax2.set_xlabel('n (Number of Points) (log scale)')
ax2.set_ylabel('Standard Deviation (log scale)')
ax2.grid(True, which="both", ls="--")

fig2.savefig(os.path.join(output_dir, "analysis_2_stability.png"))
print("Saved analysis_2_stability.png")


# --- 3. Analysis: Performance (Runtime vs. Total Work) ---
print("\n--- Analysis 3: Performance (Runtime vs. Total Work) ---")
fig3, ax3 = plt.subplots(figsize=(10, 6))

# Create the TotalWork column
df['TotalWork'] = df['n'] * df['m_trials']
# Filter out n=1, m=1 as they are too fast to be meaningful
data_perf = df[df['TotalWork'] > 100] 

ax3.scatter(data_perf['TotalWork'], data_perf['runtime_sec'], alpha=0.5)

# Fit a line on the LINEAR data
slope_3, intercept_3 = np.polyfit(data_perf['TotalWork'], data_perf['runtime_sec'], 1)
print(f"Linear Slope (Runtime vs. TotalWork): {slope_3:.2e} sec/point")

# Plot the fitted line
x_vals = np.array(ax3.get_xlim())
y_vals = intercept_3 + slope_3 * x_vals
ax3.plot(x_vals, y_vals, color='red', linestyle='--')

ax3.set_title('Performance: Runtime vs. Total Work (n * m_trials)')
ax3.set_xlabel('Total Work (n * m_trials)')
ax3.set_ylabel('Runtime (seconds)')
ax3.grid(True)

fig3.savefig(os.path.join(output_dir, "analysis_3_performance.png"))
print("Saved analysis_3_performance.png")


# --- 4. Analysis: Value of More Trials (Error vs. m) ---
print("\n--- Analysis 4: Value of More Trials (Error vs. m) ---")
fig4, ax4 = plt.subplots(figsize=(10, 6))
data_n_max = df[df['n'] == MAX_N]

# Filter out mean_error = 0 to avoid log errors
data_n_max = data_n_max[data_n_max['mean_error'] > 0]

ax4.scatter(data_n_max['m_trials'], data_n_max['mean_error'])

# Set log scales
ax4.set_xscale('log')
ax4.set_yscale('log')

# Fit line and print slope
slope_4 = fit_log_log(data_n_max['m_trials'], data_n_max['mean_error'])
print(f"Log-Log Slope (Error vs. m_trials): {slope_4:.4f} (Expected ~ -0.5)")

ax4.set_title(f'Value of Trials (Error vs. m_trials) at n = {MAX_N}')
ax4.set_xlabel('m_trials (Number of Trials) (log scale)')
ax4.set_ylabel('Mean Error (log scale)')
ax4.grid(True, which="both", ls="--")

fig4.savefig(os.path.join(output_dir, "analysis_4_value_of_trials.png"))
print("Saved analysis_4_value_of_trials.png")

print("\n--- Analysis complete. All plots saved to disk. ---")
