public class ListNode{
    int val;
    ListNode next;
    ListNode(){}
    ListNode(int val){this.val=val;}
    ListNode(int val,ListNode next){this.val=val;this.next=next;}
}

class Solution {
    public ListNode addTwoNumbers(ListNode l1, ListNode l2) {
        if (l1 == null && l2 == null) return null;

        // 1. 直接使用第一个节点作为结果链表的头
        ListNode result = new ListNode(0);
        ListNode temp = result;
        int p = 0; // 进位

        // 2. 双方都不为空时，正常相加
        while (l1 != null && l2 != null) {
            int sum = l1.val + l2.val + p;
            temp.val = sum % 10;
            p = sum / 10;

            l1 = l1.next;
            l2 = l2.next;

            // 如果双方还有下一位，提前把下一个节点建好，并把 temp 挪过去
            if (l1 != null && l2 != null) {
                temp.next = new ListNode(0);
                temp = temp.next;
            }
        }

        // 3. 处理剩下的尾巴，必须挂载到 temp.next 上！
        if (l1 != null) {
            temp.next = copyList(l1, p);
        } else if (l2 != null) {
            temp.next = copyList(l2, p);
        } else if (p == 1) {
            // 双方都空了，但最后还有一个进位（比如 5 + 5 = 10）
            temp.next = new ListNode(1);
        }

        return result;
    }

    // 复制剩余链表并处理进位
    private ListNode copyList(ListNode node, int p) {
        ListNode dummy = new ListNode(0); // 这里用哑结点，方便拼接
        ListNode temp = dummy;
        
        while (node != null) {
            int sum = node.val + p;
            temp.next = new ListNode(sum % 10);
            p = sum / 10; // 更新进位，解决 9+1=10 的连环进位
            
            node = node.next;
            temp = temp.next;
        }
        
        // 如果复制完了，还有一个进位（比如 [9] 加 1，复制完变成了 [0]，还要补个 [1]）
        if (p == 1) {
            temp.next = new ListNode(1);
        }
        
        return dummy.next;
    }
}