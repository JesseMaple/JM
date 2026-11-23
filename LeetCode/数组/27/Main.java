package com.body;

import java.util.List;
import java.util.ArrayList;
import java.util.Arrays;

public class Main {

}

class Solution {
    public int removeElement(int[] nums, int val) {
        int i = 0, n = nums.length;
        while(i < n && nums[i] != val) i++;

        for(int j = i + 1; j < n; j++) {
        	if(nums[j] != val) {
        		nums[i++] = nums[j];
        	}
        }
        return i;
    }
}