import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
import os

# Configuration
INPUT_FILE = "results.csv"
OUTPUT_DIR = "plots"
os.makedirs(OUTPUT_DIR, exist_ok=True)
REAL_PI = 3.1415926535

def main():
    print(f"Loading data from {INPUT_FILE}...")
    try:
        df = pd.read_csv(INPUT_FILE)
    except FileNotFoundError:
        print("Error: results.csv not found.")
        return

    # Set style
    sns.set_theme(style="whitegrid")
    
    # We need to transform the data for Seaborn
    # We want a format like: Size | Run_ID | Algorithm | Time | Estimate | Error
    
    # 1. Melt each metric separately
    df_time = df.melt(id_vars=['Size', 'Run_ID'], 
                      value_vars=['Time_Basic', 'Time_Strat', 'Time_Anti'],
                      var_name='Algorithm', value_name='Time')
    df_time['Algorithm'] = df_time['Algorithm'].str.replace('Time_', '')

    df_est = df.melt(id_vars=['Size', 'Run_ID'], 
                     value_vars=['Est_Basic', 'Est_Strat', 'Est_Anti'],
                     var_name='Algorithm', value_name='Estimate')
    df_est['Algorithm'] = df_est['Algorithm'].str.replace('Est_', '')

    df_err = df.melt(id_vars=['Size', 'Run_ID'], 
                     value_vars=['Err_Basic', 'Err_Strat', 'Err_Anti'],
                     var_name='Algorithm', value_name='Error')
    df_err['Algorithm'] = df_err['Algorithm'].str.replace('Err_', '')

    # Combine them (Assuming row order is preserved, which melt does)
    df_long = df_time.copy()
    df_long['Estimate'] = df_est['Estimate']
    df_long['Error'] = df_err['Error']

    # Rename for readability
    algo_map = {'Basic': 'Basic Random', 'Strat': 'Stratified (10x10)', 'Anti': 'Antithetic'}
    df_long['Algorithm'] = df_long['Algorithm'].map(algo_map)
    
    # Custom colors
    palette = {"Basic Random": "#e74c3c", "Stratified (10x10)": "#2ecc71", "Antithetic": "#3498db"}

    # ==========================================
    # PLOT 1: ERROR CONVERGENCE (Log-Log)
    # ==========================================
    print("Generating Error Convergence Plot...")
    plt.figure(figsize=(10, 6))
    
    sns.lineplot(
        data=df_long, 
        x="Size", 
        y="Error", 
        hue="Algorithm", 
        style="Algorithm",
        markers=True, 
        dashes=False,
        palette=palette,
        err_style="band"
    )

    plt.xscale('log')
    plt.yscale('log')
    plt.title("Convergence of Error vs Sample Size (Lower is Better)")
    plt.xlabel("Number of Samples (Log Scale)")
    plt.ylabel("Absolute Error |Est - Pi| (Log Scale)")
    plt.grid(True, which="both", ls="-", alpha=0.2)
    
    plt.savefig(f"{OUTPUT_DIR}/error_convergence_log.png")
    plt.close()

    # ==========================================
    # PLOT 2: ESTIMATION FUNNEL
    # ==========================================
    print("Generating Estimation Funnel Plot...")
    plt.figure(figsize=(10, 6))
    
    # Draw the target line
    plt.axhline(REAL_PI, color='black', linestyle='--', linewidth=1.5, label='True Pi')

    sns.lineplot(
        data=df_long, 
        x="Size", 
        y="Estimate", 
        hue="Algorithm", 
        palette=palette,
        errorbar=('ci', 95), # Show 95% confidence interval
        linewidth=1
    )
    
    plt.xscale('log')
    plt.title("Estimation Stability (The 'Funnel')")
    plt.xlabel("Number of Samples (Log Scale)")
    plt.ylabel("Estimated Value of Pi")
    
    # Zoom in to see the differences clearly at high N
    # We limit Y axis to meaningful Pi range roughly
    plt.ylim(3.10, 3.18) 
    
    plt.savefig(f"{OUTPUT_DIR}/estimation_funnel.png")
    plt.close()

    # ==========================================
    # PLOT 3: EXECUTION TIME
    # ==========================================
    print("Generating Time Performance Plot...")
    plt.figure(figsize=(10, 6))
    
    sns.lineplot(
        data=df_long, 
        x="Size", 
        y="Time", 
        hue="Algorithm", 
        style="Algorithm",
        markers=True,
        palette=palette
    )
    
    plt.xscale('log')
    plt.yscale('log') # Log scale for time too, since N spans orders of magnitude
    plt.title("Computational Cost vs Sample Size")
    plt.xlabel("Number of Samples (Log Scale)")
    plt.ylabel("Execution Time (ms) - Log Scale")
    
    plt.savefig(f"{OUTPUT_DIR}/execution_time.png")
    plt.close()

    print(f"Done! Check the '{OUTPUT_DIR}' folder.")

if __name__ == "__main__":
    main()