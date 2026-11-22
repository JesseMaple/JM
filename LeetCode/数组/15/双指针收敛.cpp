class Solution {
public:
    vector<vector<int>> threeSum(vector<int>& nums) {
        int n = nums.size();
        vector<vector<int>> result;
        result.reserve(n / 2); // 预分配容量
        
        sort(nums.begin(), nums.end()); 
        
        for(int i = 0; i < n - 2; i++) {
            // 去重
            if(i > 0 && nums[i] == nums[i-1]) continue;
            
            // 最小三数之和 > 0，直接结束
            if(nums[i] + nums[i + 1] + nums[i + 2] > 0) break;
            
            // 剪枝：当前数加最大两数 < 0，跳过当前数
            if(nums[i] + nums[n - 1] + nums[n - 2] < 0) continue;
            
            // 双指针收敛
            int j = i + 1, k = n - 1, target = -nums[i];
            while(j < k) {
                int sum = nums[j] + nums[k];
                if(sum < target) j++;
                else if(sum > target) k--;
                else {
                    result.push_back({nums[i], nums[j], nums[k]}); // 安全写法
                    
                    while(j < k && nums[j] == nums[j + 1]) j++;
                    while(j < k && nums[k] == nums[k - 1]) k--;
                    
                    j++; k--;
                }
            }
        }
        return result;
    }
};
