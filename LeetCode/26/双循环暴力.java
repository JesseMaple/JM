package com.body;

public class Main {

}

/*
 * 目标：
 * 删除相同元素
 * 方法：
 * 对每一个元素进行判断，
 * 首层循环每个元素
 * 二层循环该元素后面的所有元素
 * 每个元素如果与该元素相同则记录，然后删除
 */

class Solution{
	public int removeDuplicates(int[] nums) {
		int k = 0; // k 表示当前已经去重好的数组长度，同时也是下一个可以放新元素的下标
        
        // 第一层循环：遍历每一个原始元素
        for (int i = 0; i < nums.length; i++) {
            boolean isDuplicate = false; // 标记当前元素是否重复
            
            // 第二层循环：去已经去重好的部分（0 到 k-1）里面找，看看有没有一样的
            for (int j = 0; j < k; j++) {
                if (nums[i] == nums[j]) {
                    isDuplicate = true; // 发现重复了！
                    break; // 找到了就赶紧跳出内层循环，别找了
                }
            }
            
            // 如果在前面没找到一样的，说明它是新元素
            if (!isDuplicate) {
                nums[k] = nums[i]; // 把它直接放到 k 的位置（原地覆盖）
                k++; // 去重长度 +1，k 往前走一步
            }
        }
        
        return k;
	}
}