# 暴力算法<br>
直接使用双循环求出所需下标<br>
## 代码如下

以 *Java* 为例：
<details>
	<summary>点击展开</summary>


```JAVA
class Solution{
    public int[] twoSum(int[] nums,int target){
        for(int i=0;i<nums.length;i++){
            int needed=target-nums[i];
            for(int j=i+1;j<nums.length;j++){
                if(needed==nums[j]){
                    return new int[]{i,j};
                }
            }
        }
        return new int[]{-1,-1};
    }
}
```

</details>

# 哈希表<br>
创建一个哈希表实例，单循环遍历数组，对每个元素进行操作，如果该元素所需的数字未被哈希表实例记录则记录，如果已被记录则返回该pair<br>
## 代码如下

以 *Java*  为例
<details>
	<summary>点击展开</summary>


```JAVA
class Solution{
    public int[] twoSum(int[] nums,int target){
        HashMap<Integer,Integer> map=new HashMap<>();
        for(int i=0;i<nums.length;i++){
            int needed=target-nums[i];
            if(map.containsKey(needed)){
                return new int[]{map.get(needed),i};
            }
            map.put(nums[i],i);
        }
        return new int[]{-1,-1};
    }
}
```

</details>

# 排序+头尾双指针<br>
将数组有序化，然后通过头尾交替推进寻找所需下标<br>
## 代码如下

以 *Java* 为例
<details>
	<summary>点击展开</summary>


```JAVA
import java.util.Arrays;

class Solution {
    public int[] twoSum(int[] nums, int target) {
        int n = nums.length;
        int[][] numsWithIndex = new int[n][2];
        for (int i = 0; i < n; i++) {
            numsWithIndex[i][0] = nums[i];
            numsWithIndex[i][1] = i;
        }

        Arrays.sort(numsWithIndex, (a, b) -> {
            if (a[0] == b[0]) {
                return Integer.compare(a[1], b[1]);
            }
            return Integer.compare(a[0], b[0]);
        });

        int left = 0, right = n - 1;
        while (left < right) {
            int sum = numsWithIndex[left][0] + numsWithIndex[right][0];
            if (sum == target) {
                return new int[]{numsWithIndex[left][1], numsWithIndex[right][1]};
            } else if (sum > target) {
                right--;
            } else {
                left++;
            }
        }

        return new int[]{-1, -1};
    }
}
```

</details>