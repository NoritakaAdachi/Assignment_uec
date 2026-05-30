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
eps = np.random.normal(0, np.sqrt(1/beta), N)

# 出力データ
y = g(x) + eps

# STEP2: Prepare 𝑀 basis functions
M = 6
mu = np.arange(M) / M
s = 0.05

# ファイ関数の計算
Phi = np.exp(-(x[:, None] - mu[None, :])**2 / s)

# 正規方程式を解いて，重みを求める
w = np.linalg.pinv(Phi) @ y

# STEP3: 真の関数と予測関数をプロット
x_plot = np.linspace(0, 1, 500)
y_true = g(x_plot)

Phi_plot = np.exp(-(x_plot[:, None] - mu[None, :])**2 / s)
y_pred = Phi_plot @ w

plt.plot(x_plot, y_true, color="blue", label="true function g(x)")
plt.plot(x_plot, y_pred, color="red", label="RBF regression f(x; w)")
plt.scatter(x, y, color="gray", label="observed data")

plt.xlabel("x")
plt.ylabel("y")
plt.legend()

plt.savefig("N50_M6.png")
plt.show()
