import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

sns.set(style="whitegrid", font_scale=1.2)

# ---------------------------------------------------------
# Load CSVs
# ---------------------------------------------------------
results = pd.read_csv("results_ks.csv")     # per-trial data
batch = pd.read_csv("batch_ks.csv")         # summary data

# ---------------------------------------------------------
# 1. Accuracy vs Graph Size (n)
# ---------------------------------------------------------
plt.figure(figsize=(8,5))
sns.lineplot(data=batch, x="n", y="erdos_k_acc", marker="o", label="Erdos - Karger")
sns.lineplot(data=batch, x="n", y="erdos_ks_acc", marker="o", label="Erdos - Karger–Stein")
plt.title("Accuracy vs Graph Size (Erdos)")
plt.ylabel("Accuracy")
plt.ylim(0, 1.05)
plt.savefig("acc_vs_n_erdos.png")
plt.close()

plt.figure(figsize=(8,5))
sns.lineplot(data=batch, x="n", y="clique_k_acc", marker="o", label="Clique - Karger")
sns.lineplot(data=batch, x="n", y="clique_ks_acc", marker="o", label="Clique - Karger–Stein")
plt.title("Accuracy vs Graph Size (Clique)")
plt.ylabel("Accuracy")
plt.ylim(0, 1.05)
plt.savefig("acc_vs_n_clique.png")
plt.close()

# ---------------------------------------------------------
# 2. Accuracy vs Trials
# ---------------------------------------------------------
plt.figure(figsize=(8,5))
sns.lineplot(data=batch, x="trials", y="combined_k_acc", marker="o", label="Karger")
sns.lineplot(data=batch, x="trials", y="combined_ks_acc", marker="o", label="Karger–Stein")
plt.title("Accuracy vs Number of Trials")
plt.xlabel("Trials")
plt.xscale("log")
plt.ylabel("Accuracy")
plt.ylim(0,1.05)
plt.savefig("acc_vs_trials.png")
plt.close()

# ---------------------------------------------------------
# 3. Runtime vs Graph Size
# ---------------------------------------------------------
plt.figure(figsize=(8,5))
sns.lineplot(data=batch, x="n", y="erdos_k_ms", marker="o", label="Karger (Erdos)")
sns.lineplot(data=batch, x="n", y="erdos_ks_ms", marker="o", label="Karger–Stein (Erdos)")
plt.title("Runtime vs Graph Size (Erdos)")
plt.ylabel("Runtime (ms)")
plt.savefig("runtime_vs_n_erdos.png")
plt.close()

plt.figure(figsize=(8,5))
sns.lineplot(data=batch, x="n", y="clique_k_ms", marker="o", label="Karger (Clique)")
sns.lineplot(data=batch, x="n", y="clique_ks_ms", marker="o", label="Karger–Stein (Clique)")
plt.title("Runtime vs Graph Size (Clique)")
plt.ylabel("Runtime (ms)")
plt.savefig("runtime_vs_n_clique.png")
plt.close()

# ---------------------------------------------------------
# 4. Runtime vs Accuracy (Pareto Frontier)
# ---------------------------------------------------------
plt.figure(figsize=(8,5))
plt.scatter(batch["erdos_k_ms"], batch["erdos_k_acc"], label="Karger", s=80)
plt.scatter(batch["erdos_ks_ms"], batch["erdos_ks_acc"], label="Karger–Stein", s=80)
plt.title("Runtime vs Accuracy (Erdos)")
plt.xlabel("Runtime (ms)")
plt.ylabel("Accuracy")
plt.legend()
plt.savefig("pareto_erdos.png")
plt.close()

# ---------------------------------------------------------
# 5. Scatter plot of per-trial cuts (Karger vs KS)
# ---------------------------------------------------------
plt.figure(figsize=(10,5))
plt.scatter(results["trial_id"], results["karger_cut"], s=5, label="Karger", alpha=0.4)
plt.scatter(results["trial_id"], results["ks_cut"], s=5, label="Karger–Stein", alpha=0.4)
plt.plot(results["trial_id"], results["true_mincut"], c="k", linewidth=1, label="True Cut")
plt.title("Per-Trial Mincut Values")
plt.xlabel("Trial ID")
plt.ylabel("Cut Value")
plt.legend()
plt.savefig("per_trial_scatter.png")
plt.close()

# ---------------------------------------------------------
# 6. Boxplots of runtime variability
# ---------------------------------------------------------
plt.figure(figsize=(6,5))
sns.boxplot(data=results[["karger_ms", "ks_ms"]])
plt.xticks([0,1], ["Karger", "Karger–Stein"])
plt.title("Runtime Variability")
plt.ylabel("Runtime (ms)")
plt.savefig("runtime_boxplot.png")
plt.close()

# ---------------------------------------------------------
# 7. Heatmap of accuracy (n × trials)
# ---------------------------------------------------------
# ---------------------------------------------------------
# 7. Heatmap of accuracy (n × trials)
# ---------------------------------------------------------
heat = batch.pivot(index="n", columns="trials", values="combined_k_acc")

plt.figure(figsize=(8,6))
sns.heatmap(heat, annot=True, cmap="viridis")
plt.title("Heatmap: Karger Accuracy (n × trials)")
plt.savefig("heatmap_karger.png")
plt.close()


# ---------------------------------------------------------
# 8. True mincut distribution
# ---------------------------------------------------------
plt.figure(figsize=(8,5))
sns.histplot(results[results["graph_type"]=="erdos"]["true_mincut"], kde=True, color="blue", label="Erdos")
sns.histplot(results[results["graph_type"]=="clique"]["true_mincut"], kde=True, color="red", label="Clique")
plt.title("True Mincut Distribution")
plt.legend()
plt.savefig("true_mincut_dist.png")
plt.close()

print("All visualizations generated!")
