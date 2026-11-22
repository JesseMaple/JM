class Solution {
public:
    vector<int> twoSum(vector<int>& nums, int target) {
        vector<pair<int, int>> numsWithIndex;
        for (int i = 0; i < nums.size(); ++i) {
            numsWithIndex.push_back({nums[i], i});
        }
        
        sort(numsWithIndex.begin(), numsWithIndex.end());
        
        int left = 0, right = numsWithIndex.size() - 1;
        while (left < right) {
            int sum = numsWithIndex[left].first + numsWithIndex[right].first;
            if (sum == target) {
                return {numsWithIndex[left].second, numsWithIndex[right].second};
            } else if (sum > target) {
                right--;
            } else {
                left++;
            }
        }
        return {};
    }
};
