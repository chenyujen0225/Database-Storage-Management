/*
Skeleton code for storage and buffer management
*/

#include <string>
#include <ios>
#include <fstream>
#include <vector>
#include <string>
#include <string.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include "classes.h"
using namespace std;


int main(int argc, char* const argv[]) {
    // Create the EmployeeRelation file from Employee.csv
    StorageBufferManager manager("EmployeeRelation");
    manager.createFromFile("Employee.csv");
    int input;
    // Loop to lookup IDs until user is ready to quit
    while(true){
        cout<<"Please enter the employee ID to find employee's detail"<<endl;
        cin>> input;
        Record employee = manager.findRecordById(input);
        employee.print();
        if(input==-1){
            break;
        }
    }
    
    return 0;
}
