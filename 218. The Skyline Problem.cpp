#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>
#include <algorithm>
#include <set>
using namespace std;

class MaxHeap {
public:
    MaxHeap(): heap_(), id_to_i_() {}
    
    void add(int h, int id) {
        heap_.push_back({h, id});
        heapifyUp_(heap_.size() - 1);
        
        // print_("after add");
        // print_id_to_i_("id_to_i_ after add");
    }
    
    void remove(int h, int id) {
        int i = id_to_i_[id];
        swap(i, heap_.size() - 1);
        id_to_i_.erase(id);
        heap_.pop_back();
        heapifyDown_(i);
        
        // print_("after remove");
        // print_id_to_i_("id_to_i_ after remove");
    }
    
    bool empty() {
        return heap_.empty();
    }
    
    int max() {
        return heap_[0].first;
    }

private:
    // <height, id>
    vector<std::pair<int, int>> heap_;
    unordered_map<int, int> id_to_i_;
    
    // T: O(logn)
    void heapifyUp_(int i) {
        id_to_i_[heap_[i].second] = i;
        while (i != 0) {
            int p = (i - 1) / 2;
            if (heap_[p].first >= heap_[i].first) return;
            
            swap(p, i);
            i = p;
        }
    }
    
    // T: O(logn)
    void heapifyDown_(int i) {
        int max_index = i;
        int max = heap_[i].first;
        // right child
        if (2 * i + 2 < heap_.size() && max < heap_[2 * i + 2].first) {
            max_index = 2 * i + 2;
            max = heap_[2 * i + 2].first;
        }
         
        // left child
        if (2 * i + 1 < heap_.size() && max < heap_[2 * i + 1].first) {
            max_index = 2 * i + 1;
            max = heap_[2 * i + 1].first;
        }
        
        if (max_index != i) {
            swap(i, max_index);
            heapifyDown_(max_index);
        }
    }
    
    void swap(int i, int j) {
        int h = heap_[i].first;
        int id = heap_[i].second;
        heap_[i].first = heap_[j].first;
        heap_[i].second = heap_[j].second;
        heap_[j].first = h;
        heap_[j].second = id;
        
        id_to_i_[heap_[i].second] = i;
        id_to_i_[heap_[j].second] = j;
    }
    
    void print_(string status) {
        std::cout << status << ": ";
        for (auto e: heap_) {
            std::cout << e.first << "," << e.second << " ";
        }
        
        std::cout << std::endl;
    }
    
    void print_id_to_i_(string status) {
        std::cout << status << ": ";
        for (auto e: id_to_i_) {
            std::cout << e.first << "," << e.second << " ";
        }

        std::cout << std::endl;
    }
};

// uses a line to sweep the events(entering or leaving) and keeps tracking the heights
// for a entering event, adds the height
// for a leaving event, removes the height
// every time sweeps at x, all the events(same x), entering or leaving, the height changing will line up in a proper order
// after a event happening at x finished, if the max height changes, adds it to the answer
// key point 1: sorts the events in a delicate way
// key point 2: uses an efficient data structure to add/remove events.
// uses a max heap with a vector to keep track of the node positions, T(add): O(logn), T(remove): O(logn), T(max): O(1)
// T: O(nlogn)
// S: O(n)
class Solution {
public:
    vector<vector<int>> getSkyline(vector<vector<int>>& buildings) {
        vector<Event> events;
        int id = 0;
        for (auto building: buildings) {
            events.push_back({id, building[0], building[2], 1});
            events.push_back({id, building[1], building[2], -1});
            id++;
        }
        
        // T: O(nlogn)
        std::sort(events.begin(), events.end());
        
        MaxHeap max_heap;
        int max_height = 0;
        vector<vector<int>> ans;
        for (auto event: events) {
            if (event.type == 1) max_heap.add(event.h, event.id);
            else max_heap.remove(event.h, event.id);
            
            if (max_heap.empty() && max_height != 0) {
                ans.push_back({event.x, 0});
                max_height = 0;
            } else if (max_heap.max() != max_height) {
                ans.push_back({event.x, max_heap.max()});
                max_height = max_heap.max();
            }
        }
        
        return ans;
    }

private:
    struct Event {
        int id;
        int x;
        int h;
        int type; // enter(1) or leave(-1)
        
        Event(int id, int x, int h, int type):id(id), x(x), h(h), type(type) {}
        
        // sorts by x, type, h, this is the sweeping order
        // if xs are the same, when both types are entering, sweeps the higher first
        // when both types are leaving, sweeps the lower first
        // when one type is entering and the other one is leaving, sweeps the enter event first
        bool operator<(const Event& e) const {
            if (x == e.x) {
                return type * h > e.type * e.h;
            }
            
            return x < e.x;
        }
    };
};

// uses a line to sweep the events(entering or leaving) and keeps tracking the heights
// for a entering event, adds the height
// for a leaving event, removes the height
// every time sweeps at x, all the events(same x), entering or leaving, the height changing will line up in a proper order
// after a event happening at x finished, if the max height changes, adds it to the answer
// key point 1: sorts the events in a delicate way
// key point 2: uses an efficient data structure to add/remove events.
// uses a AVL/BST(multiset which allows duplicated keys), T(add): O(logn), T(remove): O(logn), T(max): O(1)(the right most)
// T: O(nlogn)
// S: O(n)
class Solution2 {
public:
    vector<vector<int>> getSkyline(vector<vector<int>>& buildings) {
        // x, h
        typedef std::pair<int, int> Event;
        vector<Event> events;
        for (auto building: buildings) {
            events.push_back({building[0], building[2]});
            // negative height means leaving
            events.push_back({building[1], -building[2]});
        }
        
        hs_.clear();
        
        // T: O(nlogn)
        std::sort(events.begin(), events.end(), [](const Event& e1, const Event& e2){
            if (e1.first == e2.first) return e1.second > e2.second;
            return e1.first < e2.first;
        });
        
        int max_height = 0;
        vector<vector<int>> ans;
        for (auto event: events) {
            if (event.second > 0) hs_.insert(event.second);
            // just removes one of them if there are multiple hs
            else hs_.erase(hs_.find(abs(event.second)));
            
            if (hs_.empty() && max_height != 0) {
                // just removed all the events
                ans.push_back({event.first, 0});
                max_height = 0;
            } else if (max() != max_height) {
                ans.push_back({event.first, max()});
                max_height = max();
            }
        }
        
        return ans;
    }
    

private:
    multiset<int> hs_;
    
    int max() {
        if (hs_.empty()) return 0;
        return *hs_.rbegin();
    }
};

class SegmentTree {
public:
    SegmentTree():root_(new TreeNode()) {}
    
    vector<vector<int>> update(vector<vector<int>>& buildings) {
        // T: O(nlogn) ~ O(n^2), each update needs to recursively build the tree
        for (auto building: buildings)
            update_(root_, building[0], building[1], building[2]);

        vector<vector<int>> ans;
        int prev_height = 0;
        // T: O(n)
        dfs_(root_, ans, prev_height);
        
        // a special case that last segment is extended to INT_MAX
        if (ans.back()[1] != 0) ans.push_back({INT_MAX, 0});
        
        return ans;
    }

private:
    struct TreeNode {
        int start;
        int mid; // split the range to two parts [start, mid) and [mid, end)
        int end;
        int height; // the height of full coverage on [start, end)
        unique_ptr<TreeNode> left; // dynamicly creates nodes using smart pointer to avoid memory leak
        unique_ptr<TreeNode> right;
        
        TreeNode():start(INT_MIN), mid(-1), end(INT_MAX), height(0), left(nullptr), right(nullptr) {};
        TreeNode(int start, int end, int height):start(start), mid(-1), end(end), height(height), left(nullptr), right(nullptr) {};
    };
    
    TreeNode* root_;
    
    // T: O(logn) ~ O(n), each update needs to recursively build the tree
    // S: O(n)
    void update_(TreeNode* cur, int start, int end, int height) {
        // current node has been splitted
        if (cur->mid != -1) {
            if (end <= cur->mid) update_(cur->left.get(), start, end, height);
            else if (start >= cur->mid) update_(cur->right.get(), start, end, height);
            else {
                if (cur->start == start && cur->end == end) {
                    cur->height = max(cur->height, height);
                }
                update_(cur->left.get(), start, cur->mid, height);
                update_(cur->right.get(), cur->mid, end, height);
            }
            
            return;
        }
        
        if (cur->start == start && cur->end == end) {
            cur->height = max(cur->height, height);

            return;
        }
        
        // prefers to use start to split the range
        if (start > cur->start) {
            cur->mid = start;
            cur->left.reset(new TreeNode(cur->start, start, cur->height));
            cur->right.reset(new TreeNode(start, cur->end, cur->height));
            update_(cur->right.get(), start, end, height);
            
            return;
        }
        
        // end < cur->end && start == cur->start
        cur->mid = end;
        cur->left.reset(new TreeNode(cur->start, end, cur->height));
        cur->right.reset(new TreeNode(end, cur->end, cur->height));
        update_(cur->left.get(), start, end, height);
        
        return;
    }
    
    void dfs_(TreeNode* curr, vector<vector<int>>& ans, int& prev_height) {
        if (!curr) return;
        if (!curr->left.get() && !curr->right.get() && curr->height != prev_height) {
            ans.push_back({curr->start, curr->height});
            prev_height = curr->height;
        }
        
        dfs_(curr->left.get(), ans, prev_height);
        dfs_(curr->right.get(), ans, prev_height);
    }
};

// uses segment tree to save the merged ranges and outputs the ranges left point(excluding the -inf)
// T: O(nlogn) ~ O(n^2)
// S: O(n)
class Solution3 {
public:
    vector<vector<int>> getSkyline(vector<vector<int>>& buildings) {
        SegmentTree tree_;
        return tree_.update(buildings);
    }
};