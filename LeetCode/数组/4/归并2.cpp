class Solution {
public:
    double findMedianSortedArrays(vector<int>& nums1, vector<int>& nums2) {
        size_t size1 = nums1.size(), size2 = nums2.size();
        
        int* p = size1 > 0 ? nums1.data() : nullptr;
        int* q = size2 > 0 ? nums2.data() : nullptr;
        
        vector<int> nums;
        nums.reserve(size1 + size2);
        
        size_t count1 = 0, count2 = 0;
        
        while (count1 < size1 || count2 < size2) {
            if (count1 < size1 && count2 < size2) {
                if (*p > *q) {
                    nums.push_back(*q++);
                    count2++;
                } else {
                    nums.push_back(*p++);
                    count1++;
                }
            }
            else if (count1 < size1) {
                nums.push_back(*p++);
                count1++;
            }
            else if (count2 < size2) {
                nums.push_back(*q++);
                count2++;
            }
        }

        size_t n = nums.size();
        if (n % 2 != 0) {
            return (double)nums[n / 2];
        } else {
            return (double)(nums[n / 2 - 1] + nums[n / 2]) / 2.0;
        }
    }
};
