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
		if (nums.length == 0) return 0;
        int slow = 0;
        for (int fast = 1; fast < nums.length; fast++) {
            if (nums[fast] != nums[slow]) {
                slow++;
                nums[slow] = nums[fast];
            }
        }
        return slow + 1;
	}
}