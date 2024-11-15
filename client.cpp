#include <iostream>
#include "HeaderFiles/TCPClient.h"
#include "HeaderFiles/Helpers.h"
using namespace std;

#include <sstream>

void clearTerminal()
{
    cout << "\033[2J\033[1;1H";
}

int getAction() // gets the action from the user
{
    int action;
    do
    {
        cout << "Select Action: " << endl;
        cout << "1. Add Record" << endl;
        cout << "2. Read Records" << endl;
        cout << "3. Sort and Read Records" << endl;
        cout << "4. Search Records" << endl;
        cout << "5. Exit" << endl;
        cout << "Action: ";
        cin >> action;
    } while (action < 1 || action > 5);
    return action;
}

void displayAvailableTables(const vector<string> &tableNames) // displays the available tables
{
    cout << "Choose table: " << endl;
    for (int i = 0; i < tableNames.size(); i++)
    {
        cout << i + 1 << ". " << tableNames[i] << endl; // 1 based indexing
    }
}

int getUserChoice(int maxChoice) // gets the user choice
{
    int choice;
    while (true)
    {
        cout << "Choice: ";
        cin >> choice;
        if (choice > 0 && choice <= maxChoice) // valid choice
        {
            break;
        }
        cout << "Invalid choice. Please try again: ";
    }
    return choice - 1; // back to 0 based indexing
}

void handleNewTable(TCPClient &client) // creation of new table
{
    string tableName;
    cout << "Enter table name: ";
    cin >> tableName;
    string columnValues = ""; // column values will be sent as csv
    while (true)              // add column names to the table
    {
        string columnName;
        cout << "Enter column name (type 'q' to stop): ";
        cin >> columnName;
        if (columnName == "q")
        {
            break;
        }
        columnValues += columnName + ","; // csv format
    }

    if (!client.sendMessage("0" + tableName + ":" + columnValues + ":")) // send request to server to create new table
    // Code: 0TatbleName:Columns:
    {
        std::cerr << "Failed to send message\n";
    }
    cout << "Message sent" << endl;
}

int getColumnIndex(const vector<string> &columnNames) // get the table's column index
{
    int columnIndex;
    cout << "Select column to search/sort by: " << endl;
    for (int i = 0; i < columnNames.size(); i++)
    {
        cout << i + 1 << ". " << columnNames[i] << endl; // 1 based indexing
    }
    cout << "Choice: ";
    cin >> columnIndex;
    return columnIndex - 1; // 0 based indexing
}

void handleAction(TCPClient &client, int action, const string &tableName, const vector<string> &columnNames)
// handles action for selected existing table
{
    int columnIndex;
    string record; // record sent as a csv
    string value;  // value: used as search, temporary etc

    switch (action) // switch selected action
    {
    case 1:                                 // add record
        for (const auto &col : columnNames) // ask for values for each column
        {
            cout << "Enter value for " << col << ": ";
            cin >> value;
            record += value + ",";
        }
        if (!client.sendMessage(to_string(action) + tableName + ":" + record + ":")) // Code: 1TableName:ColumnValues:
        {
            std::cerr << "Failed to send message\n";
        }
        cout << "Message sent" << endl;
        break;

    case 2:                                                           // read all records
        if (!client.sendMessage(to_string(action) + tableName + ":")) // Code: 2TableName:
        {
            std::cerr << "Failed to send message\n";
        }
        break;

    case 3:                                                                                          // sort and read records
        columnIndex = getColumnIndex(columnNames);                                                   // get index of column to sort by
        if (!client.sendMessage(to_string(action) + tableName + ":" + to_string(columnIndex) + ":")) // Code: 3TableName:
        {
            std::cerr << "Failed to send message\n";
        }
        break;

    case 4:                                        // search records
        columnIndex = getColumnIndex(columnNames); // get index of column to search by
        cout << "Enter value to search: ";
        cin >> value;
        if (!client.sendMessage(to_string(action) + tableName + ":" + to_string(columnIndex) + ":" + value + ":")) // Code: 4TableName:ColumnName:Value:
        {
            std::cerr << "Failed to send message\n";
        }
        break;

    case 5: // exit
        if (!client.sendMessage("c"))
        {
            std::cerr << "Failed to send message\n";
        }
        break;

    default:
        break;
    }
}

void handleExistingTable(TCPClient &client)
{
    if (!client.sendMessage("9")) // Code to ask server for all existing tables in the database
    {
        std::cerr << "Failed to send message\n";
    }
    string tables = client.receiveMessage();        // receive tables from server
    vector<string> tableNames = csv2vector(tables); // tables come as csv, so convert to vector

    displayAvailableTables(tableNames);
    int tableIndex = getUserChoice(tableNames.size()); // get user's choice
    clearTerminal();

    cout << "Selected Table: " << endl
         << tableNames[tableIndex] << endl;
    cout << "-------------------" << endl;

    // Ask server for selected table's columns
    if (!client.sendMessage("8" + tableNames[tableIndex] + ":")) // Code: 8TableIndex:
    {
        std::cerr << "Failed to send message\n";
    }
    string columns = client.receiveMessage();         // receive columns from server as a csv
    vector<string> columnNames = csv2vector(columns); // convert to vector

    // Display table schema
    cout << "Table Columns (Schema) : " << endl;
    for (const auto &col : columnNames)
    {
        cout << col << ",";
    }
    cout << endl
         << "-----------------------" << endl;

    int action = getAction(); // get user's action from otpions: add record, read all  records etc
    handleAction(client, action, tableNames[tableIndex], columnNames);
}

// Server Codes
//  0 - New Table 0NewTableName:Columns:
//  1 - Add Record 1TableName:ColumnValues:
//  2 - Read all Records of a Table 2TableName:
//  3 - Sort and Read records - 3TableName:ColumnIndex:
//  4 - Search Records 4TableName:ColumnIndex:Value:
//  8 - Return Table Columns (Schema) 5TableIndex:
//  9 - Return All Tables 6
int main()
{
    cout << "Client on the same machine as server?" << endl;
    cout << "1. Yes" << endl;
    cout << "2. No" << endl;
    int choice = getUserChoice(2); // max 2 options, returns as 0 based index
    if (choice == 1)               // not on the same machine
    {
        cout << "Enter server IP: ";
        string ip;
        cin >> ip;
        TCPClient client(ip, 8888);
    }

    TCPClient client("127.0.0.1", 8888); // client object using ipv4 and port 8888
    if (!client.connectToServer())
    {
        std::cerr << "Failed to connect to server\n";
        return 1;
    }

    while (true) // loop to keep the client running - maintain connection
    {
        clearTerminal();
        cout << "Select Table: " << endl;
        cout << "1. New Table" << endl;
        cout << "2. Existing Table" << endl;
        int tableChoice = getUserChoice(2); // max 2 options

        if (tableChoice == 0) // new table
        {
            handleNewTable(client);
        }
        else // existing table
        {
            handleExistingTable(client);
        }

        string response;
        do
        {
            response = client.receiveMessage(); // receive response from server
        } while (response.empty()); // keep waiting if the response is empty
        cout << response << endl;

        std::cout << "Press enter to continue.\n";
        // ignore \n and wait for user to press enter
        std::cin.ignore();
        std::cin.get();
    }

    return 0;
}