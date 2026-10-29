class Solution {
public:
    vector<int> twoSum(vector<int>& nums, int target) {
        for(int i=0;i<nums.size();i++){
			for(int j=i+1;j<nums.size();j++){
				if(nums[i]+nums[j]==target){
					vector<int> n;
					n.push_back(i);n.push_back(j);
					return n;
				}
			}
		}
        vector<int> m;
        return m;
    }
};
