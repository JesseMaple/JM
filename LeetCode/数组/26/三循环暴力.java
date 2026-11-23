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
		int count=nums.length;
		for(int i=0;i<count;i++) {
			for(int j=i+1;j<count;j++) {
				if(nums[j]==nums[i]) {
					for(int k=j+1;k<count;k++) {
						nums[k-1]=nums[k];
					}
					count--;
                    j--;
				}
			}
		}
		return count;
	}
}