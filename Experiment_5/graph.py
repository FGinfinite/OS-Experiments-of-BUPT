import matplotlib.pyplot as plt
import numpy as np

# 存储数据的字典
data = {}

# 循环遍历十个文件
for i in range(10):
    filename = f"./result/result_{i}.txt"

    # 打开文件并解析数据
    with open(filename, "r") as file:
        lines = file.readlines()

    for line in lines:
        line = line.strip().split()
        name = line[0]
        page_size = int(line[1])
        hit_rate = float(line[2][:-1])  # 去除百分号
        replacement_rate = float(line[3][:-1])  # 去除百分号

        if name not in data:
            data[name] = {"page_size": [], "hit_rate": [], "replacement_rate": []}

        data[name]["page_size"].append(page_size)
        data[name]["hit_rate"].append(hit_rate)
        data[name]["replacement_rate"].append(replacement_rate)

# 计算平均值并插值处理
page_sizes = np.arange(4, 33)
for name, values in data.items():
    hit_rates = np.array(values["hit_rate"])
    replacement_rates = np.array(values["replacement_rate"])

    hit_rates_interp = np.interp(page_sizes, values["page_size"], hit_rates)
    replacement_rates_interp = np.interp(page_sizes, values["page_size"], replacement_rates)

    data[name]["hit_rate"] = hit_rates_interp
    data[name]["replacement_rate"] = replacement_rates_interp

# 创建第一个图表：命中率
plt.figure(1)
plt.title("Average Page Hit Rate")
plt.xlabel("Page Size")
plt.ylabel("Hit Rate")

for name, values in data.items():
    hit_rates = values["hit_rate"]
    plt.plot(page_sizes, hit_rates, label=name)

plt.legend()

# 创建第二个图表：置换率
plt.figure(2)
plt.title("Average Page Replacement Rate")
plt.xlabel("Page Size")
plt.ylabel("Replacement Rate")

for name, values in data.items():
    replacement_rates = values["replacement_rate"]
    plt.plot(page_sizes, replacement_rates, label=name)

plt.legend()

# 展示图表
plt.show()
``