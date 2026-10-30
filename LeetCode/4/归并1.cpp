class Solution {
public:
    double findMedianSortedArrays(vector<int>& nums1, vector<int>& nums2) {
        vector<int> whole;
		int i=0,j=0;
		int n=nums1.size(),m=nums2.size(); 
		while(i<n&&j<m){
			if(nums1[i]<=nums2[j])whole.push_back(nums1[i++]);
			else whole.push_back(nums2[j++]);
		}
		while (i < n) whole.push_back(nums1[i++]);
        while (j < m) whole.push_back(nums2[j++]);
        int total=whole.size();
		if(total%2==0)return ((double)whole[total/2-1]+(double)whole[total/2])/2.0;
		else return (double)whole[total/2];
    }
};
