class Solution {
public:
    int lengthOfLongestSubstring(string s) {
        int lastPos[128];
        for (int i = 0; i < 128; i++) lastPos[i] = -1;

        int maxLen = 0;
        int left = 0;

        for (int right = 0; right < s.size(); right++) {
            char c = s[right];
            
            if (lastPos[c] != -1 && lastPos[c] >= left) {
                left = lastPos[c] + 1;
            }
            
            lastPos[c] = right;
            
            maxLen = max(maxLen, right - left + 1);
        }
        return maxLen;
    }
};
