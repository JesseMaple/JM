package com.body;

import java.util.List;
import java.util.ArrayList;
import java.util.Arrays;

public class Main {

}

class Solution {
    public List<List<Integer>> fourSum(int[] nums, int target) {
        List<List<Integer>> res = new ArrayList<>();
        Arrays.sort(nums);

        for (int i = 0; i < nums.length - 3; i++) {
            if (i > 0 && nums[i] == nums[i - 1]) continue;

            for (int j = i + 1; j < nums.length - 2; j++) {
                if (j > i + 1 && nums[j] == nums[j - 1]) continue;

                int k = j, l = nums.length - 1;
                long need = (long) target - nums[i] - nums[j];

                while (k + 1 < l) {
                    int left = nums[k + 1];
                    int right = nums[l];
                    long had = (long) left + right;

                    if (had < need) {
                        k++;
                    } else if (had > need) {
                        l--;
                    } else {
                        res.add(new ArrayList<>(Arrays.asList(nums[i], nums[j], left, right)));

                        // 跳过左边重复元素
                        while (k + 1 < l && nums[k + 1] == nums[k + 2]) k++;
                        // 跳过右边重复元素
                        while (k + 1 < l && nums[l] == nums[l - 1]) l--;

                        k++;
                        l--;
                    }
                }
            }
        }
        return res;
    }
}