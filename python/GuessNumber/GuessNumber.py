import random
from FileOperator import *

def main():
    count = load_victory_times() # 记录游戏胜利次数
    max_time = load_max() # 记录最大步数
    min_time = load_min() # 记录最小步数
    times = 0 # 步数记录器
    generated = random.randint(1, 100) # 初始化随机数
    while True:
        # 尝试输入，如果非数字则提示
        try:
            guess = int(input(
                "Guess a number:\n(between 1 and 100, out-of-range will be modded into 1~100, input 0 to exit, -1 to show history, -2 to clean history, -3 to show max step)\n"
            ))
        except ValueError:
            print("Please enter a number\n")
            continue

        # 退出游戏时提示并退出
        if guess == 0:
            print("Game Over\n")
            save_victory_times(count)
            save_max(max_time)
            save_min(min_time)
            break
        # 展示猜中次数时输出游戏次数并继续游戏
        elif guess == -1:
            print(f"History: {count}\n")
            continue
        # 清理游戏记录时将所有历史记录均清除
        elif guess == -2:
            print(f"History {count} has been cleared\n")
            count = 0
            max_time = None
            min_time = None
            clear_victory_times()
            clear_max()
            clear_min()
            continue
        # 展示最值步数时输出步数，游戏中途不改变
        elif guess == -3:
            print(f"Max step: {max_time}")
            print(f"Min step: {min_time}")
            continue
        elif guess <=0:
            print(f"Number {guess} is invalid\n")
            continue

        # 已经获取输入并处理完特殊数，步数+1
        times += 1
        # 对次数取模
        guess = (guess - 1) % 100 + 1

        # 如果猜中了，count++，如果步数超出最值步数则更新最值步数并重置步数记录器，并且重新生成随机数
        if guess == generated:
            print("You guessed the number\n")
            count += 1
            if max_time is None or times >= max_time:
                max_time = times
                save_max(max_time)
            if min_time is None or times <= min_time:
                min_time = times
                save_min(min_time)
            times = 0
            generated = random.randint(1, 100)
        else:
            if guess > generated:
                print("Too high\n")
            else:
                print("Too low\n")


if __name__ == "__main__":
    main()
