import subprocess
import os
import csv
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
import re

# --- CONFIGURATION ---
BIN_PATH = "../bin/primality_test"
DATA_DIR = "../data"
OUTPUT_DIR = "../plots"
RESULTS_FILE = "../results.csv"

# File Paths
CARMICHAEL_FILE = os.path.join(DATA_DIR, "carmichael.txt")
PRIME_FILE = os.path.join(DATA_DIR, "primes_2048.txt") 
COMPOSITE_FILE = os.path.join(DATA_DIR, "composites_2048.txt")

os.makedirs(OUTPUT_DIR, exist_ok=True)
sns.set_theme(style="whitegrid", context="talk", palette="deep")

def run_single_test(algo, filepath, k):
    """Helper to run C++ binary and parse output."""
    cmd = [BIN_PATH, "--algo", algo, "--file", filepath, "--k", str(k)]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, check=True)
        lines = result.stdout.strip().split("\n")
        data = []
        for row in lines[1:]: # Skip header
            parts = row.split(",")
            if len(parts) == 3:
                data.append(parts) # [Number, Result, TimeUS]
        return data
    except Exception as e:
        print(f"Error running {algo} on {filepath}: {e}")
        return []

def plot_runtime_scalability():
    print("[1/4] Generating Scalability Plot (Runtime vs BitLength)...")
    if not os.path.exists(RESULTS_FILE):
        print("   [!] results.csv not found. Please run benchmark_runner.py first for this specific plot.")
        return

    df = pd.read_csv(RESULTS_FILE)

    df = df[
        (~df['File'].str.contains("carmichael")) & 
        (df['BitLength'] != 'Unknown')
    ].copy()
    df['BitLength'] = pd.to_numeric(df['BitLength'])
    
    plt.figure(figsize=(10, 6))
    sns.lineplot(data=df, x="BitLength", y="TimeUS", hue="Algorithm", style="Algorithm", markers=True, dashes=False, linewidth=2.5)
    plt.title("Runtime Scalability ($O(k \log^3 n)$)")
    plt.xlabel("Bit Length")
    plt.ylabel("Time ($\mu s$)")
    plt.yscale("log")
    plt.tight_layout()
    plt.savefig(f"{OUTPUT_DIR}/runtime_comparison.png")
    plt.close()

def plot_carmichael_failure():
    print("[2/4] Generating Carmichael Failure Plot...")
    # We compare Fermat (k=1) vs Miller (k=5)
    # This shows the "Trap" vs the "Solution"
    
    results = []
    
    # Fermat Run (k=1) - The Trap
    data_f = run_single_test("fermat", CARMICHAEL_FILE, k=1)
    fail_f = sum(1 for row in data_f if row[1] == "1") # 1 = Prime (False Positive)
    rate_f = (fail_f / len(data_f)) * 100 if data_f else 0
    results.append({"Algorithm": "Fermat (k=1)", "FailureRate": rate_f})
    
    # Miller Run (k=5) - The Solution
    data_m = run_single_test("miller", CARMICHAEL_FILE, k=5)
    fail_m = sum(1 for row in data_m if row[1] == "1")
    rate_m = (fail_m / len(data_m)) * 100 if data_m else 0
    results.append({"Algorithm": "Miller-Rabin (k=5)", "FailureRate": rate_m})
    
    df = pd.DataFrame(results)
    
    plt.figure(figsize=(8, 6))
    ax = sns.barplot(x="Algorithm", y="FailureRate", data=df, palette=["#e74c3c", "#2ecc71"])
    plt.title("Failure Rate on Carmichael Numbers")
    plt.ylabel("False Positive Rate (%)")
    plt.ylim(0, 105)
    for i, v in enumerate(df["FailureRate"]):
        ax.text(i, v + 2, f"{v:.1f}%", ha='center', fontweight='bold')
    plt.tight_layout()
    plt.savefig(f"{OUTPUT_DIR}/carmichael_failure.png")
    plt.close()

def plot_convergence():
    print("[3/4] Generating Convergence Plot (Error vs k)...")
    results = []
    # Run Miller-Rabin on Carmichael numbers for k=1 to 10
    for k in range(1, 11):
        data = run_single_test("miller", CARMICHAEL_FILE, k)
        total = len(data)
        if total == 0: continue
        errors = sum(1 for row in data if row[1] == "1") # 1 = Probable Prime (Error)
        error_rate = (errors / total) * 100
        results.append({"k": k, "ErrorRate": error_rate})

    df = pd.DataFrame(results)
    
    plt.figure(figsize=(10, 6))
    sns.lineplot(data=df, x="k", y="ErrorRate", marker="o", linewidth=3, color="#e74c3c", label="Empirical Error")
    
    # Theoretical Bound (1/4^k)
    x = np.linspace(1, 10, 100)
    y = (1 / (4**x)) * 100
    plt.plot(x, y, '--', color='gray', alpha=0.7, label="Theoretical Max ($4^{-k}$)")
    
    plt.title("Miller-Rabin Error Convergence")
    plt.xlabel("Iterations ($k$)")
    plt.ylabel("Error Probability (%)")
    plt.legend()
    plt.tight_layout()
    plt.savefig(f"{OUTPUT_DIR}/convergence_analysis.png")
    plt.close()

def plot_variance():
    print("[4/4] Generating Variance Plot (Prime vs Composite)...")
    
    # Create temp files for single number testing
    with open(PRIME_FILE, 'r') as f: prime_num = f.readline().strip()
    with open(COMPOSITE_FILE, 'r') as f: comp_num = f.readline().strip()
    
    temp_p_file = "temp_prime_var.txt"
    temp_c_file = "temp_comp_var.txt"
    with open(temp_p_file, 'w') as f: f.write(prime_num)
    with open(temp_c_file, 'w') as f: f.write(comp_num)
    
    results = []
    TRIALS = 200
    
    # Run Prime (k=5)
    for _ in range(TRIALS):
        d = run_single_test("miller", temp_p_file, k=5)
        if d: results.append({"Type": "Prime (2048-bit)", "Time": int(d[0][2])})
        
    # Run Composite (k=5)
    for _ in range(TRIALS):
        d = run_single_test("miller", temp_c_file, k=5)
        if d: results.append({"Type": "Composite (2048-bit)", "Time": int(d[0][2])})
        
    df = pd.DataFrame(results)
    
    plt.figure(figsize=(10, 6))
    sns.boxplot(x="Type", y="Time", data=df, palette="Set2", width=0.5)
    plt.title("Runtime Variance Distribution")
    plt.ylabel("Time ($\mu s$)")
    plt.tight_layout()
    plt.savefig(f"{OUTPUT_DIR}/variance_analysis.png")
    plt.close()
    
    # Cleanup
    if os.path.exists(temp_p_file): os.remove(temp_p_file)
    if os.path.exists(temp_c_file): os.remove(temp_c_file)

if __name__ == "__main__":
    plot_runtime_scalability()
    plot_carmichael_failure()
    plot_convergence()
    plot_variance()
    print(f"[*] Done! All plots saved to {OUTPUT_DIR}")