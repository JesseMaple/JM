class Solution {
public:
    vector<int> twoSum(vector<int>& nums, int target) {
        // 1. 准备一个小本子（哈希表）
        // key 存数字的值，value 存该数字在数组里的下标
        unordered_map<int, int> map;
        
        // 2. 遍历整个数组
        for (int i = 0; i < nums.size(); ++i) {
            // 3. 计算当前数字需要的“搭档”是谁
            int need = target - nums[i];
            
            // 4. 翻开小本子，看看有没有记过这个“搭档”
            // find() 返回一个迭代器，如果找到了，就不等于 map.end()
            auto it = map.find(need);
            if (it != map.end()) {
                // 找到了！it->second 是搭档的下标，i 是当前数字的下标
                // 直接返回这两个下标，程序结束
                return {it->second, i};
            }
            
            // 5. 没找到说明目前还没遇到能凑对的，就把当前的数字和下标记进小本子
            // 留给后面的数字来匹配我
            map[nums[i]] = i;
        }
        
        // 题目保证一定有解，所以理论上走不到这。为了代码规范随便返回个空数组
        return {};
    }
};