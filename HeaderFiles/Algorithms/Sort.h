#ifndef SORT_H
#define SORT_H
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>
#include <iostream>
#include "/opt/homebrew/opt/libomp/include/omp.h" //include OpenMP header file path (Mac Silicon)

// Merge sort adapted from https://www.geeksforgeeks.org/merge-sort/
// Quick sort adapted from https://www.geeksforgeeks.org/quick-sort/
// Parallel merge sort adapted from https://www.moreno.marzolla.name/teaching/HPC/handouts/omp-merge-sort.html
// Parallel quick sort adapted from (Sameer and Al-Dabbagh, 2016) (reference listed in the report)

// Sequential QuickSort
int partition(std::vector<std::vector<std::string>> &arr, int low, int high, int col)
{
    auto pivot = arr[high][col]; // Choose the last element as the pivot
    int i = low - 1;             // Index of the smaller element
    for (int j = low; j < high; ++j)
    {
        if (arr[j][col] <= pivot) // Compare the current element with the pivot
        {
            ++i;                       // Increment the index of the smaller element
            std::swap(arr[i], arr[j]); // Swap the elements at i and j
        }
    }
    std::swap(arr[i + 1], arr[high]); // Swap the pivot with the element at i+1
    return i + 1;                     // Return the index of the pivot
}
// sequential quick sort
void quickSort(int low, int high, std::vector<std::vector<std::string>> &arr, int col)
{
    if (low < high) // Check if the array has more than one element
    {
        int q = partition(arr, low, high, col); // Partition the array and get the pivot index
        quickSort(low, q - 1, arr, col);        // sort the elements before the pivot
        quickSort(q + 1, high, arr, col);       // sort the elements after the pivot
    }
}

// Parallel QuickSort
void parQuickSort(int low, int high, std::vector<std::vector<std::string>> &arr, int col)
{
    if (low < high) // Check if the array has more than one element
    {
        int q = partition(arr, low, high, col); // Partition the array and get the pivot index
#pragma omp parallel sections
        {
            // sort the partitions in parallel
#pragma omp section
            parQuickSort(low, q - 1, arr, col); // Sort the elements before the pivot

#pragma omp section
            parQuickSort(q + 1, high, arr, col); // Sort the elements after the pivot
        }
    }
}

// merge partitions function
void merge(int left, int right, int mid, std::vector<std::vector<std::string>> &arr, int col)
{
    int leftSize = mid - left + 1;                                                  // size of the left partition
    int rightSize = right - mid;                                                    // size of the right partition
    std::vector<std::vector<std::string>> leftPart(leftSize), rightPart(rightSize); // Create temporary arrays to store the partitions
    for (int i = 0; i < leftSize; ++i)                                              // Copy the elements to the left partition
        leftPart[i] = arr[left + i];
    for (int i = 0; i < rightSize; ++i) // Copy the elements to the right partition
        rightPart[i] = arr[mid + 1 + i];

    int leftIndex = 0, rightIndex = 0, mergedIndex = left; // Initialize the indices for the left, right, and merged arrays
    while (leftIndex < leftSize && rightIndex < rightSize) // Merge the partitions
    {
        if (leftPart[leftIndex][col] <= rightPart[rightIndex][col]) // Compare the elements in the left and right partitions
        {
            arr[mergedIndex++] = leftPart[leftIndex++];
        }
        else
        {
            arr[mergedIndex++] = rightPart[rightIndex++]; // Copy the element from the right partition
        }
    }
    while (leftIndex < leftSize) // Copy the remaining elements from the left partition
        arr[mergedIndex++] = leftPart[leftIndex++];
    while (rightIndex < rightSize) // Copy the remaining elements from the right partition
        arr[mergedIndex++] = rightPart[rightIndex++];
}

void mergeSort(int left, int right, std::vector<std::vector<std::string>> &arr, int col)
{
    if (left < right)
    {
        int mid = left + (right - left) / 2;
        // recursively sort the left and right partitions
        mergeSort(left, mid, arr, col);
        mergeSort(mid + 1, right, arr, col);
        merge(left, right, mid, arr, col); // merge the sorted partitions
    }
}

// Parallel MergeSort
void parMergeSort(int left, int right, std::vector<std::vector<std::string>> &arr, int col)
{
    if (left < right)
    {
        int mid = left + (right - left) / 2;
#pragma omp parallel sections
        {
            // sort the partitions recursively in parallel
#pragma omp section
            parMergeSort(left, mid, arr, col); // sort the left partition

#pragma omp section
            parMergeSort(mid + 1, right, arr, col); // sort the right partition
        }

        merge(left, right, mid, arr, col); // merge the sorted partitions
    }
}

#endif // SORT_H
