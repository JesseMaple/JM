import java.util.Arrays;

class Solution {
    public int threeSumClosest(int[] nums, int target) {
        int n = nums.length; // 获取数组长度
        int result = Integer.MAX_VALUE; // 设定结果为最大int
        int temp = result;
        Arrays.sort(nums); // 对数组排序
        int sum = 0;
        int truely = 0;
        for(int i = 0; i < n - 2; i++) {
            // 去重
            if(i > 0 && nums[i] == nums[i-1]) continue;
            
            // 双指针收敛
            int j = i + 1, k = n - 1;
            while(j < k && result != 0) {
                sum = nums[i] + nums[j] + nums[k];
                temp = result;
                result = Math.min(Math.abs(sum - target), result);
                if(temp != result)truely = sum;
                if(sum < target) j++;
                else if(sum > target) k--;
                else result = 0;
            }
        }
        return truely;
    }
}