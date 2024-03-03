import subprocess
import re
import random
from itertools import permutations
import matplotlib.pyplot as plt
import numpy as np

def permute(nums):
    nums = list(permutations(nums, len(nums)))
    return nums

def chiSquared(observation, expectation):
    return ((observation - expectation) ** 2) / expectation

# 產生測試數據
test_count = 1000000
input_data = "new\nit 1\nit 2\nit 3\nit 4\n"
for i in range(test_count):
    input_data += "shuffle\n"
input_data += "free\nquit\n"

# 執行子進程
command = './qtest -v 3'
command_list = command.split()
completed_process = subprocess.run(command_list, capture_output=True, text=True, input=input_data)
output_data = completed_process.stdout

# 提取數據
start_idx = output_data.find("l = [1 2 3 4]")
end_idx = output_data.find("l = NULL")
output_data = output_data[start_idx + 14: end_idx]
regex = re.compile(r'\d \d \d \d')
result = regex.findall(output_data)

# 整理數據
nums = [i.split() for i in result]

# 找出全部的排序可能
counter_set = {}
shuffle_array = ['1', '2', '3', '4']
s = permute(shuffle_array)

# 初始化 counter_set
for i in range(len(s)):
    w = ''.join(s[i])
    counter_set[w] = 0

# 計算每一種 shuffle 結果的數量
for num in nums:
    permutation = ''.join(num)
    counter_set[permutation] += 1

# 計算 chiSquare sum
expectation = test_count // len(s)
c = counter_set.values()
chi_squared_sum = 0
for i in c:
    chi_squared_sum += chiSquared(i, expectation)

# 顯示統計結果
print("Expectation: ", expectation)
print("Observation: ", counter_set)
print("chi square sum: ", chi_squared_sum)

# 繪製條形圖
counts_list = list(counter_set.values())

# 使用 1 到 24 作為 x 軸座標
x = np.arange(1, 25)

plt.bar(x, counts_list)
plt.xlabel('permutations')
plt.ylabel('counts')
plt.title('Shuffle result')
plt.show()
