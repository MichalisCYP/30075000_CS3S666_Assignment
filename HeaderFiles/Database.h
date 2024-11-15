#ifndef DATABASE_H
#define DATABASE_H
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <mutex>
#include <string>
#include <condition_variable>
#include "Helpers.h"
#include <unordered_map>
#include <iomanip>
#include <chrono>
#include "Algorithms/Sort.h"
#include "Algorithms/Search.h"
using namespace std;

class Database
{
public:
    Database()
    {
        updateSchema(); // update tableNames - tableSchema,
    }

    string handleRequest(const char buffer[]) // takes the buffer and handles the request
    {
        int request = buffer[0] - '0';                                                  // convert char to int
        std::string tableName, columns, value, newRecord;                               // variables to store the received data
        int columnIndex;                                                                // used for search and sort
        if (request != 9)                                                               // 9 is the only request that does not require any data
            bufferVariables(buffer, tableName, columns, value, newRecord, columnIndex); // passed by reference

        // Server Buffer Codes
        //  0 - New Table 0NewTableName:Columns:
        //  1 - Add Record 1TableName:ColumnValues:
        //  2 - Read all Records of a Table 2TableName:
        //  3 - Sort and Read records - 3TableName:ColumnIndex:
        //  4 - Search Records 4TableName:ColumnName:Value:
        //  8 - Return Table Columns (Schema) 5TableIndex:
        //  9 - Return All Tables 6
        vector<vector<string>> limitedRecords; // used to limit the number of records returned for demonstration purposes
        string output = "";
        switch (request)
        {
        case 0:                                                                 // Create New Table
            if (newFile(tableName, columns) && addToTables(tableName, columns)) // add to tables is used to keep track of the tables and their columns
            {
                return "Table Created";
            }
            return "Table Creation Failed";
        case 1:                                // Add Record
            if (addRecord(tableName, columns)) // columns is the new record which is received as a csv
            {
                return "Record Added";
            }
            return "Record Addition Failed";
        case 2:                                         // Read Records
            limitedRecords = readAllRecords(tableName); // read all records
            if (limitedRecords.size() > 30)             // limit the number of records returned for demonstration purposes
            {
                limitedRecords.resize(30);
            }
            return vector2string(limitedRecords); // returns Limited 30 records for demonstration purposes
        case 3:                                   // Sort and read Records
            return sortRecords(tableName, columnIndex);
            // sorts the records, compares sorting algorithms, as well as parallel and sequential execution, and returns the time taken and a preview of the sorted records
        case 4:                                                  // Search Records
            return searchRecords(value, tableName, columnIndex); // compares sequential and parallel linear search and returns the time taken and the result
        case 8:                                                  // Return Table Columns
            return returnSchema(tableName);                      // returns the schema of the selected table
        case 9:                                                  // Return All Tables
            return returnTablesAsString();                       // returns all the table names
        default:
            return "Invalid Request";
        }
    }

private:
    bool newFile(const std::string &tableName, const std::string &columnNames) // creates a new file for a new table
    {
        std::ofstream file("Tables/" + tableName + ".csv");
        if (file.is_open())
        {
            file << columnNames << std::endl; // column names come as a csv from a server and match the headers
            {
                std::lock_guard<std::mutex> lock(mtx); // lock the mutex since we are writing to shared memory

                tableAdded.notify_all(); // condition variable to notify other threads that a new table has been added
            }
            return true;
        }

        return false;
    }

    bool addRecord(const std::string &table, const std::string &record)
    {
        std::lock_guard<std::mutex> lock(mtx); // lock the mutex since we are writing to shared memory

        std::ofstream file("Tables/" + table + ".csv", std::ios::app); // open file in append mode
        if (file.is_open())
        {
            file << record << std::endl; // add record to the file, it comes as a csv from the client
            return true;
        }
        return false;
    }

    std::vector<std::vector<std::string>> readAllRecords(const std::string &table) // reads all records from a table and returns them as a 2D vector
    {
        std::vector<std::vector<std::string>> records; // 2D vector to store the records
        std::ifstream file("Tables/" + table + ".csv");
        if (file.is_open())
        {
            std::string line;

            std::getline(file, line); // ignore the header
            while (std::getline(file, line))
            {
                std::vector<std::string> record;
                std::stringstream ss(line);          // use stringstream to split the csv
                std::string field;                   // field to store the split values
                while (std::getline(ss, field, ',')) // split the csv
                {
                    record.push_back(std::move(field));
                }
                records.push_back(std::move(record));
            }
        }
        else
        {
            std::cerr << "Unable to open file for reading." << std::endl;
        }
        return records;
    }

    std::string searchRecords(const std::string &target, const std::string &table, int &columnIndex) // compares sequential and parallel linear search and returns the time taken and the result
    {
        std::string output = "";
        bool found;
        string time;                                                           // time taken for the search
        std::vector<std::vector<std::string>> records = readAllRecords(table); // 2D vector of records

        // timing the search is conducted using the chrono library
        auto start = std::chrono::high_resolution_clock::now();
        int result_seq = sequentialLinearSearch(records, columnIndex, target); // sequential search
        auto end = std::chrono::high_resolution_clock::now();
        time = std::to_string(std::chrono::duration<double>(end - start).count());
        output += "Time taken for sequential LinearSearch: " + time + " seconds\n";
        recordTiming("Sequential LinearSearch", to_string(records.size()), time); // add the time taken to the searchTimes.csv file

        start = std::chrono::high_resolution_clock::now();
        int result_par = parallelLinearSearch(records, columnIndex, target); // parallel search
        end = std::chrono::high_resolution_clock::now();
        time = std::to_string(std::chrono::duration<double>(end - start).count());
        output += "Time taken for parallel LinearSearch: " + time + " seconds\n";
        recordTiming("Parallel LinearSearch", to_string(records.size()), time); // add the time taken to the searchTimes.csv file

        if (result_par != -1 || result_seq != -1) // if the target is found
        {
            output += "First target found at row " + to_string(result_par) + "\n";
            output += "Target's record: \n";
            output += vector2string({records[result_par]}); // return the record where the target is found
        }
        else
        {
            output += "Target not found.\n";
        }
        return output;
    }

    std::string sortRecords(const std::string &table, int &columnIndex)
    // sorts the records, compares sorting algorithms, as well as parallel and sequential execution, and returns the time taken and a preview of the sorted records
    {

        // get all records
        std::vector<std::vector<std::string>> records = readAllRecords(table); // 2D vector of records
        string output = "";
        string time; // time taken for the sort
        string vectorSize = to_string(records.size());
        // copies of records for each sorting algorithm, since functions use recursion and pass by reference
        auto qsRecords = records;
        auto parQsRecords = records;
        auto msRecords = records;
        auto parMsRecords = records;

        // report  sort and time each algorithm
        // timings are calculated using the chrono library
        auto start = std::chrono::high_resolution_clock::now();
        quickSort(0, qsRecords.size() - 1, qsRecords, columnIndex); // sequential quick sort
        auto end = std::chrono::high_resolution_clock::now();
        time = std::to_string(std::chrono::duration<double>(end - start).count());
        output += "Time taken for sequential QuickSort: " + time + " seconds\n";
        recordTiming("QuickSort", vectorSize, time); // add the time taken to the sortTimes.csv file

        start = std::chrono::high_resolution_clock::now();
        parQuickSort(0, parQsRecords.size() - 1, parQsRecords, columnIndex); // parallel quick sort
        end = std::chrono::high_resolution_clock::now();
        time = std::to_string(std::chrono::duration<double>(end - start).count());
        output += "Time taken for parallel QuickSort: " + time + " seconds\n";
        recordTiming("Parallel QuickSort", vectorSize, time);

        start = std::chrono::high_resolution_clock::now();
        mergeSort(0, msRecords.size() - 1, msRecords, columnIndex); // sequential merge sort
        end = std::chrono::high_resolution_clock::now();
        time = std::to_string(std::chrono::duration<double>(end - start).count());
        output += "Time taken for sequential MergeSort: " + time + " seconds\n";
        recordTiming("MergeSort", vectorSize, time);

        start = std::chrono::high_resolution_clock::now();
        parMergeSort(0, parMsRecords.size() - 1, parMsRecords, columnIndex); // parallel merge sort
        end = std::chrono::high_resolution_clock::now();
        time = std::to_string(std::chrono::duration<double>(end - start).count());
        output += "Time taken for parallel MergeSort: " + time + " seconds\n";
        recordTiming("Parallel MergeSort", vectorSize, time);

        output += "Preview of sorted records: \n";
        qsRecords.resize(30);               // limit the number of records returned for demonstration purposes
        output += vector2string(qsRecords); // limit the number of records returned for demonstration purposes

        return output;
    }

    std::string returnSchema(const std::string &table) // returns the schema of the selected table - the column names
    {
        std::lock_guard<std::mutex> lock(mtx); // lock the mutex since we are reading from shared memory
        auto it = tableSchema.find(table);     // find the table
        if (it != tableSchema.end())           // if the table is found
        {
            return singleVector2string(it->second); // return the column names
        }
        return "Table not found";
    }

    void bufferVariables(const char buffer[], std::string &tableName, std::string &columns, std::string &value, std::string &newRecord, int &columnIndex)
    // used to extract the variables from the buffer
    {
        std::istringstream ss(buffer + 1); // skip the first character which is the request code
        std::getline(ss, tableName, ':');  // get the table name
        if (buffer[0] == '1')              // code 1: for adding a record, get the columns which are sent as csv
        {
            std::getline(ss, columns, ':');
        }

        if (buffer[0] == '4' || buffer[0] == '3') // for search and sort, get the column index
        {
            std::string index;
            std::getline(ss, index, ':');
            columnIndex = stoi(index);
        }
        if (buffer[0] == '4') // for search, get the value
        {
            std::getline(ss, value, ':');
        }
    }

    bool addToTables(const std::string &tableName, const std::string &columns) // adds a new table to the database
    {
        std::lock_guard<std::mutex> lock(mtx);        // lock the mutex since we are writing to shared memory
        tableNames.push_back(tableName);              // add the table name to the list of tables
        tableSchema[tableName] = split(columns, ','); // add the column names to the table's schema

        std::ofstream file("Tables/tableNames.csv", std::ios::app); // write the table name to the tableNames.csv file
        if (file.is_open())
        {
            file << tableName << std::endl;
        }
        else
        {
            return false;
        }

        std::ofstream file2("Tables/columnNames.csv", std::ios::app); // write the column names to the columnNames.csv file
        if (file2.is_open())
        {
            file2 << columns << std::endl;
        }
        else
        {
            return false;
        }

        tableAdded.notify_all(); // notify other threads that a new table has been added
        return true;
    }

    std::string returnTablesAsString() // returns all the table names as a csv
    {
        std::lock_guard<std::mutex> lock(mtx); // lock the mutex since we are reading from shared memory
        std::ostringstream oss;
        for (const auto &table : tableNames) // iterate through the table names
        {
            oss << table << ","; // add the table name to the output
        }
        return oss.str();
    }

    void updateSchema() // used to keep threads in sync and in database object construction
    {
        std::lock_guard<std::mutex> lock(mtx); // lock the mutex since we are writing to shared memory
        // clear the tables and schema
        tableNames.clear();
        tableSchema.clear();

        std::ifstream file("Tables/tableNames.csv"); // read the table names from the tableNames.csv file
        if (file.is_open())
        {
            std::string table;
            while (std::getline(file, table)) // read each table name
            {
                tableNames.push_back(table);
            }
        }

        std::ifstream file2("Tables/columnNames.csv"); // read the column names from the columnNames.csv file
        if (file2.is_open())
        {
            std::string line;
            for (const auto &table : tableNames) // iterate through the tables and read the column names
            {
                if (std::getline(file2, line)) // each line is a csv of column names
                {
                    tableSchema[table] = split(line, ','); // split the csv and add the column names to the schema
                }
            }
        }
    }

    void recordTiming(const std::string &algorithm, const std::string &vectorSize, const std::string &time) // records the time taken for an algorithm
    {
        string fileName = "";
        if (algorithm == "QuickSort" || algorithm == "Parallel QuickSort" || algorithm == "MergeSort" || algorithm == "Parallel MergeSort") // sorting

        {
            fileName = "sortTimes.csv";
        }
        else if (algorithm == "Sequential LinearSearch" || algorithm == "Parallel LinearSearch") // searchin
        {
            fileName = "searchTimes.csv";
        }

        std::ofstream file(fileName, std::ios::app); // open the file in append mode
        if (file.is_open())
        {
            // get current date and time
            time_t now = std::time(0);
            tm *ltm = localtime(&now);
            file << std::setw(2) << std::setfill('0') << ltm->tm_mday << "-"
                 << std::setw(2) << std::setfill('0') << 1 + ltm->tm_mon << "-"
                 << 1900 + ltm->tm_year << "-"
                 << std::setw(2) << std::setfill('0') << ltm->tm_hour << "-"
                 << std::setw(2) << std::setfill('0') << ltm->tm_min << "-"
                 << std::setw(2) << std::setfill('0') << ltm->tm_sec;

            file << "," << algorithm << "," << vectorSize << "," << time << std::endl; // write the date, algorithm, vector size, and time taken
        }
    }

    std::condition_variable tableAdded;                                    // condition variable to notify other threads that a new table has been added
    std::mutex mtx;                                                        // mutex to protect shared memory between threads
    std::vector<std::string> tableNames;                                   // vector to store the table names
    std::unordered_map<std::string, std::vector<std::string>> tableSchema; // map to store the key-value table name - column names
};

#endif // DATABASE_H