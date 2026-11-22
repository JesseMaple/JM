import java.util.Arrays;

public class 双指针逼近法优化 {
    
}

class Solution {
    public int threeSumClosest(int[] nums, int target) {
        Arrays.sort(nums);
        int n = nums.length;
        
        // 初始化一个不可能达到的最大差值
        int minDiff = Integer.MAX_VALUE; 
        // 初始化最接近的和（随便赋个初始值）
        int closestSum = 0; 
        
        for (int i = 0; i < n - 2; i++) {
            if (i > 0 && nums[i] == nums[i - 1]) continue;
            
            int j = i + 1, k = n - 1;
            while (j < k) {
                int sum = nums[i] + nums[j] + nums[k];
                int diff = Math.abs(sum - target);
                
                // 如果现在的差值更小，说明找到了更接近的
                if (diff < minDiff) {
                    minDiff = diff;
                    closestSum = sum; // 顺手把真正的结果存下来
                }
                
                // 如果完美命中，直接下班
                if (diff == 0) {
                    return sum; 
                }
                
                // 双指针移动
                if (sum < target) {
                    j++;
                } else {
                    k--;
                }
            }
        }
        return closestSum;
    }
}