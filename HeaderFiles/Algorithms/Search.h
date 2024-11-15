#include "/opt/homebrew/opt/libomp/include/omp.h" //include OpenMP header file path (Mac Silicon)
#include <iostream>
#include <vector>
#include <string>

int sequentialLinearSearch(const std::vector<std::vector<std::string>> &data, int column, const std::string &target)
{
    for (int i = 0; i < data.size(); ++i)
    {
        if (data[i][column] == target)
        {
            return i; // Return the index of the row where the value is found
        }
    }
    return -1; // Return -1 if the value is not found
}

int parallelLinearSearch(const std::vector<std::vector<std::string>> &data, int column, const std::string &target)
{
    int index = -1; // Variable to store the index of the target if found

#pragma omp parallel for shared(index) // Start parallel region and share the index variable
    for (int i = 0; i < data.size(); ++i)
    {
        if (data[i][column] == target) // Check if the value is found
        {
#pragma omp critical
            {
                if (index == -1 || i < index) 
                {
                    index = i; // Update index to the first occurrence found
                }
            }
        }
    }

    return index; // Return the index of the row where the value is found or -1 if not found
}
