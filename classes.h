#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <bitset>
#include <cstdio>
#include <fstream>
#include <cstdint>
using namespace std;

class Record {
public:
    int id, manager_id;
    std::string bio, name;

    Record(vector<std::string> fields) {
        //cout << "size of fields: " << fields.size() <<endl;
        //cout << "id: " << fields[0] <<endl;
        //cout << "name: " << fields[1] <<endl;
        //cout << "bio: " << fields[2] <<endl;
        //cout << "manager_id: " << fields[3] <<endl;
        id = stoi(fields[0]);//convert string into int
        name = fields[1];
        bio = fields[2];
        manager_id = stoi(fields[3]);
    }

    void print() {
        cout << "\tID: " << id << "\n";
        cout << "\tNAME: " << name << "\n";
        cout << "\tBIO: " << bio << "\n";
        cout << "\tMANAGER_ID: " << manager_id << "\n";
    }
};


class StorageBufferManager {

private:
    
    const int BLOCK_SIZE = 4096; // initialize the  block size allowed in main memory according to the question 
    // You may declare variables based on your need
    char psize[4096];//crearte a 496 bytes page by char array 
    int freespaceptr = 0;
    int capacity = BLOCK_SIZE-8;//4byte for freespaceptr, 4 byte for #of records 
    int numRecords = 0;
    int numPage=0;
    string newFileName;

    void flushToDisk(){
        ofstream outFile(newFileName, std::ios::binary | std::ios::app);
        outFile.write(psize, sizeof(psize));
        outFile.close();
        numPage++;
    }

    // Insert new record 
    void insertRecord(Record record) {
        // No records written yet
        if (numRecords == 0) {
            // Initialize first block
            for(int x =0; x<4096; x++){//initial the page first
                psize[x]=0;
            }
            //numRecords = 0;
            freespaceptr = 0;
            capacity = BLOCK_SIZE-8; 
            memcpy(psize + 4092, &freespaceptr, sizeof(int));//store freespaceptr in the end of page
            memcpy(psize + 4088, &numRecords, sizeof(int));// store #of records just before freespaceptr
        }

        // Add record to the block
        string singleRecord = to_string(record.id) + ',' + record.name + ',' + record.bio + ',' + to_string(record.manager_id) + ',';
        int singleRecordSize = singleRecord.size();
        capacity = capacity - singleRecordSize - 4; //4bytes for record offset

        // Take neccessary steps if capacity is reached (you've utilized all the blocks in main memory)
        if(capacity<=0){
            flushToDisk();//flush to data file in disk
            numRecords = 0;
            insertRecord(record);//store the newrecord in the new page
        }
        else{
            numRecords++;//update numRecord
            memcpy(psize + 4088, &numRecords, sizeof(int));
            memcpy(psize + 4088 - (numRecords*4), psize + 4092, sizeof(int));//for each record's offset
            const char* x = singleRecord.c_str();
            memcpy(psize + freespaceptr, x, singleRecord.size());//store record
            freespaceptr = freespaceptr + singleRecordSize;//update freespaceptr
            memcpy(psize + 4092, &freespaceptr , sizeof(int));
        }

    }

public:
    StorageBufferManager(string NewFileName) {
        
        //initialize your variables
        newFileName.assign(NewFileName);
        // Create your EmployeeRelation file 
        ofstream outFile(newFileName, ios::binary);
        outFile.close();       
    }

    // Read csv file (Employee.csv) and add records to the (EmployeeRelation)
    // std::vector<std::string> dividestring(string s, char sepe){//function for divide string into substring
    //     std::stringstream ss(s);
    //     std::vector<string> col;
    //     std::string word;
    //     while(getline(ss, word, sepe)){
    //         col.push_back(word);// after we divide the original string, store in a vector of strings
    //     }
    //     return col;
    // }
    void createFromFile(string csvFName) {
        ifstream file(csvFName);
        string select;
        vector<string> fields;
        while(getline(file, select)){
            std::stringstream ss(select);
            std::vector<string> col;
            std::string word;
            while(getline(ss, word, ',')){
                col.push_back(word);// after we divide the original string, store in a vector of strings
            }
            //fields = dividestring(select, ',');//divide the string by ','
            fields = col;
            Record newRecord = Record(fields);
            insertRecord(newRecord);   
        }
        file.close();
        flushToDisk();
    }

    void loadFromDisk(int pageIndex){
        std::ifstream inFile(newFileName, std::ios::binary);
        inFile.seekg(pageIndex * BLOCK_SIZE, std::ios::beg);
        inFile.read(psize, sizeof(psize));
        inFile.close();
    }

    // Given an ID, find the relevant record and print it
    Record findRecordById(int id) {
        //search in memory's page first (use char psize[4096])
        //test();
        int numStoredRecords;
        memcpy(&numStoredRecords, psize + 4088, sizeof(int));

        for (int i = 0; i < numStoredRecords; i++) {
            int recordOffset;
            int recordOffset2;
            //int recordLength = recordOffset2-recordOffset;
            memcpy(&recordOffset, psize + 4084 - (i * 4), sizeof(int));
            memcpy(&recordOffset2, psize + 4084-((i+1)*4), sizeof(int));
            int recordLength = recordOffset2-recordOffset;
            //string storeString;
            if(recordLength == 0){// ==0 only one record in this page
                string storeString(psize , freespaceptr );//to catch the only record in page
                // Parse the stored record into fields
                std::istringstream recordStream(storeString);
                std::vector<std::string> recordFields;
                std::string field;
                while (getline(recordStream, field, ',')) {
                    //cout << field << endl;
                    recordFields.push_back(field);
                }
            

                // Create a Record object from the fields
                Record result(recordFields);

                // Check if the ID matches
                if (result.id == id) {
                    return result;
                }
            }
            else if(recordLength < 0){//last record in this page
                string storeString(psize + recordOffset, freespaceptr - recordOffset);
                // Parse the stored record into fields
                std::istringstream recordStream(storeString);
                std::vector<std::string> recordFields;
                std::string field;
                while (getline(recordStream, field, ',')) {
                    //cout << field << endl;
                    recordFields.push_back(field);
                }

                // Create a Record object from the fields
                Record result(recordFields);

                // Check if the ID matches
                if (result.id == id) {
                    return result;
                }
            }
            else{
                // Extract the stored record from the buffer
                string storeString(psize + recordOffset, recordLength);
                // Parse the stored record into fields
                std::istringstream recordStream(storeString);
                std::vector<std::string> recordFields;
                std::string field;
                while (getline(recordStream, field, ',')) {
                    //cout << field << endl;
                    recordFields.push_back(field);
                }
                
                // Create a Record object from the fields
                Record result(recordFields);

                // Check if the ID matches
                if (result.id == id) {
                    return result;
                }
            }
        }

        std::ifstream inFile(newFileName, std::ios::binary);
        int pageIndex =0;
        for(pageIndex =0; pageIndex<numPage; pageIndex++){
            loadFromDisk(pageIndex);
            memcpy(&numStoredRecords, psize + 4088, sizeof(int));
            memcpy(&freespaceptr, psize + 4092, sizeof(int));

            for (int i = 0; i < numStoredRecords; i++) {
                int recordOffset;
                int recordOffset2;
                //int recordLength = recordOffset2-recordOffset;
                memcpy(&recordOffset, psize + 4084 - (i * 4), sizeof(int));
                memcpy(&recordOffset2, psize + 4084-((i+1)*4), sizeof(int));
                int recordLength = recordOffset2-recordOffset;
                //string storeString;
                if(recordLength == 0){// ==0 only one record in this page
                    string storeString(psize , freespaceptr );//to catch the only record in page
                    // Parse the stored record into fields
                    std::istringstream recordStream(storeString);
                    std::vector<std::string> recordFields;
                    std::string field;
                    while (getline(recordStream, field, ',')) {
                        //cout << field << endl;
                        recordFields.push_back(field);
                    }
                
                    // Create a Record object from the fields
                    Record result(recordFields);

                    // Check if the ID matches
                    if (result.id == id) {
                        return result;
                    }
                }
                else if(recordLength < 0){//last record in this page
                    string storeString(psize + recordOffset, freespaceptr - recordOffset);
                    // Parse the stored record into fields
                    std::istringstream recordStream(storeString);
                    std::vector<std::string> recordFields;
                    std::string field;
                    while (getline(recordStream, field, ',')) {
                        //cout << field << endl;
                        recordFields.push_back(field);
                    }

                    // Create a Record object from the fields
                    Record result(recordFields);

                    // Check if the ID matches
                    if (result.id == id) {
                        return result;
                    }
                }
                else{
                    // Extract the stored record from the buffer
                    string storeString(psize + recordOffset, recordLength);
                    // Parse the stored record into fields
                    std::istringstream recordStream(storeString);
                    std::vector<std::string> recordFields;
                    std::string field;
                    while (getline(recordStream, field, ',')) {
                        //cout << field << endl;
                        recordFields.push_back(field);
                    }
                    
                    // Create a Record object from the fields
                    Record result(recordFields);

                    // Check if the ID matches
                    if (result.id == id) {
                        return result;
                    }
                }
            }
        }
         // test I/O function // 

        // char test[4096];
        // int c = 0;
        // FILE* file_;
        // file_ = fopen(newFileName.c_str(), "r");
        // while (!feof(file_)){ // to read file
        // // function used to read the contents of file
        //     fread(test, 4096, 1, file_);
        //     int offset = 0 ,offset2;
        //     int amount;
        //     memcpy(&amount,test + 4088 , sizeof(int));
        //     c += amount;
        //     // cout << amount << "and" << numRecords<<endl;
        //     for(int i = 1; i < amount ; i ++){
        //         memcpy(&offset2,(test +4088 - i*4),sizeof(int));
        //         string result;
        //         for(int j = offset; j <offset2 ; j++){
        //             //cout << test[j] ;
        //             result += test[j];
        //         }
        //         cout <<""<< result << endl;
        //         cout << endl;
        //         offset = offset2;
        //     }
        //     memcpy(&offset2,test+4092,sizeof(int));
        //     string result;
        //     for(int j = offset; j <offset2 ; j++){
        //             cout << test[j] ;
        //             result += test[j];
        //     }
        //     cout << ""<< result<<endl;
        //     cout << endl;
        // }

        // cout << c <<endl;
        // fclose(file_);
        

        // If not found in buffer, load data from the file (newFileName)
        
        inFile.close();

        // Close the file and return an empty Record if not found
        return Record(std::vector<std::string>{"0", "0", "0", "0"});
        //vector<string> nofound;

    }
};

