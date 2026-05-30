import numpy as np
import matplotlib.pyplot as plt

N = 50
beta = 25

# STEP1: データ生成
def g(x):
    return -100 * x * (x - 0.5)**2 * (x - 1)

# 入力データ
x = np.random.uniform(0, 1, N)

# ガウシアンのノイズ
eps = np.random.normal(0, np.sqrt(1 / beta), N)

# 出力データ
y = g(x) + eps

# STEP2: Prepare M basis functions
M = 5
mu = np.arange(M) / M
s = 0.05

# ファイ関数の計算
Phi = np.exp(-(x[:, None] - mu[None, :])**2 / s)

# 比較したい alpha の値
alphas = [0, 0.0001, 0.01, 0.1, 1, 10, 100, 1000]

# STEP3: 真の関数と予測関数をプロット
x_plot = np.linspace(0, 1, 500)
y_true = g(x_plot)
Phi_plot = np.exp(-(x_plot[:, None] - mu[None, :])**2 / s)

plt.figure(figsize=(10, 6))

# 真の関数
plt.plot(x_plot, y_true, label="true function g(x)", linewidth=3)

# 観測データ
plt.scatter(x, y, s=10, alpha=0.4, label="observed data")

# Ridge regression: alpha を変えながら回帰曲線を描画
I = np.eye(M)

for alpha in alphas:
    w_ridge = np.linalg.solve(
        Phi.T @ Phi + alpha * I,
        Phi.T @ y
    )
    y_pred_ridge = Phi_plot @ w_ridge
    plt.plot(x_plot, y_pred_ridge, label=f"alpha = {alpha}")

    # 重みの大きさも確認するために表示
    print(f"alpha = {alpha:8g}, ||w|| = {np.linalg.norm(w_ridge):.6f}")

plt.xlabel("x")
plt.ylabel("y")
plt.title(f"Ridge Regression with Various alpha values (N={N}, M={M})")
plt.legend()
plt.grid(True)

plt.savefig("N50_M5_multi_alpha.png")
plt.show()
