import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os

# Configuration
INPUT_FILE = "results.csv"
OUTPUT_DIR = "plots"
os.makedirs(OUTPUT_DIR, exist_ok=True)

def main():
    # 1. Load Data
    print(f"Loading data from {INPUT_FILE}...")
    try:
        df = pd.read_csv(INPUT_FILE)
    except FileNotFoundError:
        print("Error: results.csv not found. Please run the C++ benchmark first.")
        return

    # 2. Preprocessing
    # Seaborn prefers "Long" format (one row per observation), but our CSV is "Wide" 
    # (separate columns for each algorithm). We use .melt() to fix this.
    print("Preprocessing data...")
    df_long = df.melt(
        id_vars=['Size', 'InputType', 'Run_ID'], 
        value_vars=['Time_Standard', 'Time_Randomized', 'Time_DualPivot'],
        var_name='Algorithm', 
        value_name='Time'
    )
    
    # Clean up the Algorithm names (remove "Time_" prefix for cleaner legends)
    df_long['Algorithm'] = df_long['Algorithm'].str.replace('Time_', '')

    # Set the style
    sns.set_theme(style="whitegrid")
    
    # Define consistent colors: Standard=Red, Randomized=Green, DualPivot=Blue
    custom_palette = {"Standard": "#e74c3c", "Randomized": "#2ecc71", "DualPivot": "#3498db"}

    # --- PLOT 1: RANDOM INPUTS (The Fair Fight) ---
    print("Generating Random Input Plot...")
    
    random_df = df_long[df_long['InputType'] == 'Random']
    
    if not random_df.empty:
        plt.figure(figsize=(10, 6))
        
        sns.lineplot(
            data=random_df, 
            x="Size", 
            y="Time", 
            hue="Algorithm", 
            style="Algorithm",
            markers=True, 
            dashes=False,
            palette=custom_palette,
            err_style="band", # Shows the variance/standard deviation as a shaded band
            errorbar='sd'     # Standard Deviation
        )
        
        plt.title("Runtime on RANDOM Arrays (Average Case)")
        plt.xlabel("Input Size (N)")
        plt.ylabel("Execution Time (ms)")
        plt.legend(title="Algorithm Logic")
        
        outfile = f"{OUTPUT_DIR}/random_performance.png"
        plt.savefig(outfile)
        plt.close()
        print(f"Saved: {outfile}")
    else:
        print("Warning: No Random data found.")

    # --- PLOT 2: SORTED INPUTS (The Stress Test) ---
    print("Generating Sorted Input Plot...")
    
    sorted_df = df_long[df_long['InputType'] == 'Sorted']
    
    if not sorted_df.empty:
        plt.figure(figsize=(10, 6))
        
        # We use a lineplot again to show the quadratic curve
        sns.lineplot(
            data=sorted_df, 
            x="Size", 
            y="Time", 
            hue="Algorithm", 
            style="Algorithm",
            markers=True, 
            dashes=False,
            palette=custom_palette
        )
        
        plt.title("Runtime on SORTED Arrays (Worst Case Vulnerability)")
        plt.xlabel("Input Size (N)")
        plt.ylabel("Execution Time (ms)")
        
        # Optional: Log scale helps see the fast algos, but Linear scale shows the failure better.
        # Let's keep Linear to demonstrate the O(N^2) explosion visually.
        # plt.yscale("log") 
        
        plt.legend(title="Algorithm Logic")
        
        outfile = f"{OUTPUT_DIR}/sorted_performance.png"
        plt.savefig(outfile)
        plt.close()
        print(f"Saved: {outfile}")
    else:
        print("Warning: No Sorted data found.")

    print(f"Done! Analysis complete. Check the '{OUTPUT_DIR}' folder.")

if __name__ == "__main__":
    main()