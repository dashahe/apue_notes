#include <pthread.h>
#include <vector>
#include <iostream>
#include <algorithm>
using namespace std;

constexpr int kPartLength = 1000000;
constexpr int kThreadCount = 4;

vector<int> nums;
pthread_barrier_t barrier;

bool IsSorted(const vector<int> &nums) {
    for (int i = 1; i < nums.size(); ++i) {
        if (nums[i] < nums[i-1]) {
            cout << i << " " << nums[i-1] << " " << nums[i] << endl;
            return false;
        }
    }
    return true;
}

void *thread_sort(void *arg) {
    long idx = (long)arg;
    sort(nums.begin() + idx * kPartLength, nums.begin() + (idx + 1) * kPartLength);
    pthread_barrier_wait(&barrier);
}

void MergeParts() {
    vector<int> snums(nums);
    vector<int> idx;
    for (int i = 0; i < kThreadCount; ++i) {
        idx.push_back(i * kPartLength);
    }

    for (int sidx = 0; sidx < nums.size(); ++sidx) {
        int num = INT32_MAX;
        int minidx = 0;
        for (int i = 0; i < kThreadCount; ++i) {
            if (idx[i] < (i+1)*kPartLength && nums[idx[i]] < num) {
                num = nums[idx[i]];
                minidx = i;
            }
        }
        snums[sidx] = nums[idx[minidx]];
        idx[minidx]++;
    }
    nums = snums;
}

int main() {
    for (int i = 0; i < kPartLength * kThreadCount; ++i) {
        nums.push_back(rand() % kPartLength * 10);
    }

    clock_t begin = clock();
    //single thread
    // sort(nums.begin(), nums.end());
// sort ok. time used: 4.73934 seconds

    // use 4 threads
    pthread_t tid;
    pthread_barrier_init(&barrier, NULL, kThreadCount+1);
    for (int i = 0; i < kThreadCount; ++i) { // use 4 threads to sort
        if (pthread_create(&tid, NULL, thread_sort, (void*)(i))) {
            exit(0);
        }
    }
    pthread_barrier_wait(&barrier);
    MergeParts();
    clock_t end = clock();

    if (IsSorted(nums)) {
        cout << "sort ok. time used: " << static_cast<double>(end - begin) / CLOCKS_PER_SEC  << " seconds" << endl;
    } else {
        cout << "gg";
    }
}