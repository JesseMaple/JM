#include <queue>
#include <unordered_set>
#include <algorithm>

using namespace std;

class Solution{
	public:
		int lengthOfLongestSubstring(string s){
			queue<char> q;
			unordered_set<char> seen;
			int maxLen=0;
			
			for(char c:s){
				while(seen.count(c)){
					char frontChar=q.front();
					q.pop();
					seen.erase(frontChar);
				}
				
				q.push(c);
				seen.insert(c);
				maxLen=max(maxLen,(int)q.size());
			}
			
			return maxLen;
		}
}; 
