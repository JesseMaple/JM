public class ListNode{
    int val;
    ListNode next;
    ListNode(){}
    ListNode(int val){this.val=val;}
    ListNode(int val,ListNode next){this.val=val;this.next=next;}
}

class Solution {
    public ListNode addTwoNumbers(ListNode l1, ListNode l2) {
        ListNode dummy = new ListNode(0); // 哑结点
        ListNode curr = dummy;
        int p = 0; // 进位

        // 只要有一个没空，或者还有进位，就继续加
        while (l1 != null || l2 != null || p != 0) {
            // 谁空了就补 0
            int x = (l1 != null) ? l1.val : 0;
            int y = (l2 != null) ? l2.val : 0;

            int sum = x + y + p;
            p = sum / 10; // 更新进位

            // 无脑新建节点挂上去
            curr.next = new ListNode(sum % 10);
            curr = curr.next;

            // 只有没空的才往下走
            if (l1 != null) l1 = l1.next;
            if (l2 != null) l2 = l2.next;
        }

        return dummy.next;
    }
}