//CMSC 341 - Project 4
// mytest.cpp
// David Premkumar
// davidp3@umbc.edu

#include "cache.h"
#include <random>
#include <vector>
#include <algorithm> //solely for using the count() function 1 time
const int MINSEARCH = 0;
const int MAXSEARCH = 7;
// the following array defines sample search strings for testing
string searchStr[MAXSEARCH + 1] = { "c++","python","java","scheme","prolog","c#","c","js" };
enum RANDOM { UNIFORMINT, UNIFORMREAL, NORMAL };
class Random {
public:
    Random(int min, int max, RANDOM type = UNIFORMINT, int mean = 50, int stdev = 20) : m_min(min), m_max(max), m_type(type)
    {
        if (type == NORMAL) {
            //the case of NORMAL to generate integer numbers with normal distribution
            m_generator = std::mt19937(m_device());
            //the data set will have the mean of 50 (default) and standard deviation of 20 (default)
            //the mean and standard deviation can change by passing new values to constructor 
            m_normdist = std::normal_distribution<>(mean, stdev);
        }
        else if (type == UNIFORMINT) {
            //the case of UNIFORMINT to generate integer numbers
            // Using a fixed seed value generates always the same sequence
            // of pseudorandom numbers, e.g. reproducing scientific experiments
            // here it helps us with testing since the same sequence repeats
            m_generator = std::mt19937(10);// 10 is the fixed seed value
            m_unidist = std::uniform_int_distribution<>(min, max);
        }
        else { //the case of UNIFORMREAL to generate real numbers
            m_generator = std::mt19937(10);// 10 is the fixed seed value
            m_uniReal = std::uniform_real_distribution<double>((double)min, (double)max);
        }
    }
    void setSeed(int seedNum) {
        // we have set a default value for seed in constructor
        // we can change the seed by calling this function after constructor call
        // this gives us more randomness
        m_generator = std::mt19937(seedNum);
    }

    int getRandNum() {
        // this function returns integer numbers
        // the object must have been initialized to generate integers
        int result = 0;
        if (m_type == NORMAL) {
            //returns a random number in a set with normal distribution
            //we limit random numbers by the min and max values
            result = m_min - 1;
            while (result < m_min || result > m_max)
                result = m_normdist(m_generator);
        }
        else if (m_type == UNIFORMINT) {
            //this will generate a random number between min and max values
            result = m_unidist(m_generator);
        }
        return result;
    }

    double getRealRandNum() {
        // this function returns real numbers
        // the object must have been initialized to generate real numbers
        double result = m_uniReal(m_generator);
        // a trick to return numbers only with two deciaml points
        // for example if result is 15.0378, function returns 15.03
        // to round up we can use ceil function instead of floor
        result = std::floor(result * 100.0) / 100.0;
        return result;
    }

private:
    int m_min;
    int m_max;
    RANDOM m_type;
    std::random_device m_device;
    std::mt19937 m_generator;
    std::normal_distribution<> m_normdist;//normal distribution
    std::uniform_int_distribution<> m_unidist;//integer uniform distribution
    std::uniform_real_distribution<double> m_uniReal;//real uniform distribution

};


//function parameter for the hash function
unsigned int hashCode(const string str);

//the hash values from the provided hash function for each string, if the 
//size of the hash table is 101 (the minimum size)
int MIN_CPP_HASH = 92;
int MIN_PYTHON_HASH = 58;
int MIN_JAVA_HASH = 45;
int MIN_SCHEME_HASH = 74;
int MIN_PROLOG_HASH = 3;
int MIN_CSHARP_HASH = 70;
int MIN_C_HASH = 99;
int MIN_JS_HASH = 78;


class Tester {

public:

    //test insertion for a normal case
    bool testInsertionNormal();

    //test find for an Error case
    bool testGetPersonError();

    //test find with non-colliding keys
    bool testGetPersonNonCollide();

    //test find with colliding keys
    bool testGetPersonCollide();

    //test remove with non-colliding keys
    bool testRemoveNonCollide();

    //test remove with colliding keys
    bool testRemoveCollide();

    //test rehash triggering after many insertions
    bool testTriggerRehashInsertion();

    //test rehash completion after many insertions
    bool testRehashCompletionInsertion();

    //test rehash trigger after many deletions
    bool testTriggerRehashDeletion();

    //tesh rehash completion after many deletions
    bool testRehashCompletionDeletion();


private:

	//declare a usable Cache for any normal case in testing
    //can also return a vector of everything inserted if you pass in a pointer to it
	void makeRandomNormalTable(Cache* cache, int size = 101, int amount = 49, vector <Person>* dataList = nullptr, int exclude = 0);


};


int main() {
    
    //initiate tester object and call all the tests

    cout << "Initiating tests for the Cache Class\n";

    Tester cacheTester;

    cacheTester.testInsertionNormal();

    cacheTester.testGetPersonError();

    cacheTester.testGetPersonNonCollide();

    cacheTester.testGetPersonCollide();

    cacheTester.testRemoveNonCollide();

    cacheTester.testRemoveCollide();

    cacheTester.testTriggerRehashInsertion();

    cacheTester.testRehashCompletionInsertion();

    cacheTester.testTriggerRehashDeletion();

    cacheTester.testRehashCompletionDeletion();

    cout << "\nEnd of Testing\n";

	return 0;
}


bool Tester::testInsertionNormal() {

    cout << "\nNow testing Insertion for a normal case\n";
    cout << "Inserting a few non-colliding keys\n";


    Cache testCache = Cache(MINPRIME, hashCode); //to test

    //define some non-colliding people
    Person cppPerson = Person("c++", 1001);
    Person pythonPerson = Person("python", 1002);
    Person javaPerson = Person("java", 1003);
    Person cPerson = Person("c", 1004);

    //insert these people
    testCache.insert(Person(cppPerson));
    testCache.insert(Person(pythonPerson));
    testCache.insert(Person(javaPerson));
    testCache.insert(Person(cPerson));

    //make sure the size was changed properly
    if (testCache.m_currentSize != 4) {
        cout << "Size did not increment properly, test failed\n"; 
        return false;
    }

    //now make sure the people are in the correct buckets
    if (testCache.altFindIndex(cppPerson) != MIN_CPP_HASH) {
        cout << "Person not inserted in the right location, test failed\n";
        return false;
    }
    if (testCache.altFindIndex(pythonPerson) != MIN_PYTHON_HASH) {
        cout << "Person not inserted in the right location, test failed\n";
        return false;
    }
    if (testCache.altFindIndex(javaPerson) != MIN_JAVA_HASH) {
        cout << "Person not inserted in the right location, test failed\n";
        return false;
    }
    if (testCache.altFindIndex(cPerson) != MIN_C_HASH) {
        cout << "Person not inserted in the right location, test failed\n";
        return false;
    }

    //otherwise they all inserted properly so test is passed
    cout << "All people inserted in the correction location, test passed\n";
    return true;
}


bool Tester::testGetPersonError() {

    cout << "\nNow testing getPerson for an error case\n";

    //create a Cache with random data, excluce the ID 5995 so it can't be inserted
    Cache testCache = Cache(101, hashCode);
    
    //final parameter is an ID that won't be inserted randomly
    makeRandomNormalTable(&testCache, 101, 511, nullptr, 5995);

    //check if trying to grab a person who doesn't exist returns an empty person
    if (testCache.getPerson("C++", 5995) == EMPTY) {
        cout << "Error case person not found, test passed" << endl;
        return true;
    }
    else {
        cout << "Get Person did not return an empty Person, test failed\n";
        return false;
    }

}


bool Tester::testGetPersonNonCollide() {

    cout << "\nNow testing getPerson with only non-colliding keys\n";

    Cache testCache = Cache(MINPRIME, hashCode); //to test

    //define some non-colliding people
    Person cppPerson = Person("c++", 1001);
    Person pythonPerson = Person("python", 1002);
    Person javaPerson = Person("java", 1003);
    Person cPerson = Person("c", 1004);

    //insert these people
    testCache.insert(Person(cppPerson));
    testCache.insert(Person(pythonPerson));
    testCache.insert(Person(javaPerson));
    testCache.insert(Person(cPerson));

    //try to find one of the cases
    if ((cppPerson == testCache.getPerson("c++", 1001))) {
        cout << "Get Person Works with a normal non-colliding case, test passed\n";
        return true;
    }
    else {
        cout << "Get Person does not work with a normal non - colliding case, test failed\n";
        return false;
    }
}


bool Tester::testGetPersonCollide() {

    cout << "\nNow Testing find with many colliding cases\n";

    //creating a cache with a bunch of colliding cases

    Cache testCache = Cache(101, hashCode);

    //loop and put data with unique IDs, but the same colliding key
    for (int i = 1000; i < 1511; i++) {

        testCache.insert(Person("Python", i));
    }

    //try to find one of them
    if (testCache.getPerson("Python", 1433) == Person("Python", 1433)) {

        cout << "Get Person continues to function with many colliding cases, test passed\n";
        return true;
    }
    else {
        
        cout << "Get Person does not work with many colliding cases, test failed\n";
        return false;
    }

}


bool Tester::testRemoveNonCollide() {

    cout << "\nNow Testing remove() with a few non-colliding cases\n";

    Cache testCache = Cache(MINPRIME, hashCode); //to test

    //define some non-colliding people
    Person cppPerson = Person("c++", 1001);
    Person pythonPerson = Person("python", 1002);
    Person javaPerson = Person("java", 1003);
    Person cPerson = Person("c", 1004);

    //insert these people
    testCache.insert(Person(cppPerson));
    testCache.insert(Person(pythonPerson));
    testCache.insert(Person(javaPerson));
    testCache.insert(Person(cPerson));


    cout << "Trying to delete a person\n";

    //cut the java person out
    testCache.remove(javaPerson);

    //make sure the slot that it occupied is marked as deleted
    if (testCache.m_currentTable[testCache.findIndex(javaPerson)] == DELETED) {
        cout << "Person successfully deleted in a non-collding case, test passed\n" << endl;
        return true;
    }
    else {
        cout << "Person unsuccessfully deleted in a non-collding case, test failed\n" << endl;
        return false;
    }
}


bool Tester::testRemoveCollide() {

    cout << "\nNow Testing find with many colliding cases\n";

    //creating a cache with a bunch of colliding cases
    Cache testCache = Cache(101, hashCode);

    //loop and put data with unique IDs, but the same colliding key
    for (int i = 1000; i < 1511; i++) {

        testCache.insert(Person("Python", i));
    }


    //remove a specific slot in between
    cout << "Deleting a person from a table with many collding cases\n";

    testCache.remove(Person("Python", 1433));

    //make sure the slot that it occupied is marked as deleted
    if (testCache.m_currentTable[testCache.findIndex(Person("Python", 1433))] == DELETED) {
        cout << "Person successfully deleted in a colliding case, test passed\n" << endl;
        return true;
    }
    else {
        cout << "Person unsuccessfully deleted in a colliding case, test failed\n" << endl;
        return false;
    }


}


bool Tester::testTriggerRehashInsertion() {

    cout << "\nNow testing the rehash trigger after many insertions\n";

    Cache testCache = Cache(MINPRIME, hashCode); //for testing

    vector <Person> personList; //for holding all the people inserted

    //pass the testCache by reference and fill it up with just enough to trigger a rehash
    makeRandomNormalTable(&testCache, 101, 51, &personList);

    cout << "Attempting to trigger a rehash\n";

    //if the table has things, verify that it holds what it needs to
    //if the table is empty then the test fails
    if (testCache.m_oldTable) {

        int idealNum = 39; //the number of elements that should be in the old Table
        int actualNum = 0; //the actual number, to be imcremented when searching the Old table

        for (int i = 0; i < 101; i++) {

            //search the old table and count the number of elements
            if (!(testCache.m_oldTable[i] == DELETED) && !(testCache.m_oldTable[i] == EMPTY)) {
                actualNum++;
            }
        }

        //see if the old table has the right amount
        if (actualNum != idealNum) {
            cout << "Rehash triggered, but the old table didn't lose 25% of its contents properly\n";
            return false;
        }
        else {
            cout << "Old table properly lost 25% of its contents\n";
        }
    }
    else {
        cout << "Rehashing not triggered properly, test failed\n";
        return false;
    }

    //see if the new table has the right amount
    if (testCache.m_currentSize == 12) {
        cout << "The new table has the correct amount of elements\n";
    }
    else {
        cout << "The new table doesn't have the correct amount of elements, test failed\n";
        return false;
    }


    //verify that size of the new table is correct
    if (testCache.m_currentCap == testCache.findNextPrime((51) * 4)) {
        cout << "The new table's capactity is correct\n";
    }
    else {
        cout << "The new table's capacity is wrong, test failed\n";
        return false;
    }


    //and verify that every element still exists and can be found
    for (vector <Person>::iterator i = personList.begin(); i != personList.end(); i++) {

        if (!(testCache.getPerson(i->getKey(), i->getID()) == *i)) {
            cout << "Person who should be in the Cache isn't, test failed\n";
            return false;
        }
    }

    //if it goes through the loop and the loop terminates, the test passes
    //because no Person got lost
    cout << "Cache has everyone in it, test passed\n";
    return true;
}


bool Tester::testRehashCompletionInsertion() {

    cout << "\nNow testing the completion of the rehashing operation after many insertions\n";


    //pulled from the driver.cpp, generate a a large cache which should trigger a few of rehashes
    //make random normal table isn't used since that uses run time checking for duplicates
    vector<Person> dataList;
    Random RndID(MINID, MAXID);
    Random RndStr(MINSEARCH, MAXSEARCH);
    Cache cache(MINPRIME, hashCode);

    bool result = true;
    for (int i = 0; i < 811; i++) { //811 will trigger many insertions
        // generating random data
        Person dataObj = Person(searchStr[RndStr.getRandNum()], RndID.getRandNum());

        // saving data for later use
        dataList.push_back(dataObj);

        // inserting data in to the Cache object
        cache.insert(dataObj);
    }

    //iterate and verify that all the people are in the cache
    for (vector<Person>::iterator it = dataList.begin(); it != dataList.end(); it++) {
        result = result && (*it == cache.getPerson((*it).getKey(), (*it).getID()));
    }

    //determine if the test passes
    if (result)
        cout << "All people exist after many insertions and rehashes, test passed\n";
    else
        cout << "Some people are missing in the Cache object after many insertions and rehashes, test failed\n";

    return result;
}


bool Tester::testTriggerRehashDeletion() {

    cout << "\nNow testing the triggering of a rehash after many deletions\n";

    Cache testCache = Cache(MINPRIME, hashCode); //for testing

    vector <Person> personList; //for holding all the people inserted

    //pass the testCache by reference and fill it up with quite a few people
    makeRandomNormalTable(&testCache, 101, 40, &personList);

    cout << "Attempting to trigger a rehash\n";

    //delete 80% of them to trigger a rehash
    for (int i = 0; i < 32; i++) {
        testCache.remove(personList[i]);
    }
   
    //now the old table and all the deleted things should be discarded
    //the current table should have 8 elements

    //see if the new table has the right amount
    if (testCache.m_currentSize == 8) {
        cout << "The new table has the correct amount of elements\n";
    }
    else {
        cout << "The new table doesn't have the correct amount of elements, test failed\n";
        return false;
    }


    //verify that size of the new table is correct
    if (testCache.m_currentCap == testCache.findNextPrime((8) * 4)) {
        cout << "The new table's capactity is correct\n";
    }
    else {
        cout << "The new table's capacity is wrong, test failed\n";
        return false;
    }


    //and verify that every element still exists and can be found
    for (int i = 32; i < 40; i++) {

        if (!(testCache.getPerson(personList[i].getKey(), personList[i].getID()) == personList[i])) {
            cout << "Person who should be in the Cache isn't, test failed\n";
            return false;
        }
    }

    //if it goes through the loop and the loop terminates, the test passes
    //because no Person got lost
    cout << "Cache has everyone not deleted in it, test passed\n";
    return true;

}


bool Tester::testRehashCompletionDeletion() {

    cout << "\nNow testing the completion of a rehash after many deletions\n";

    Cache testCache = Cache(MINPRIME, hashCode); //for testing

    vector <Person> personList; //for holding all the people inserted

    //pass the testCache by reference and fill it up with quite a few people
    makeRandomNormalTable(&testCache, 101, 80, &personList);

    cout << "Attempting to trigger and complete rehash\n";

    //delete 80% of them to trigger a rehash
    for (int i = 0; i < 64; i++) {
        testCache.remove(personList[i]);
    }

    //now the old table and all the deleted things should be discarded
    //the current table should have 8 elements

    //see if the new table has the right amount
    if (testCache.m_currentSize == 16) {
        cout << "The new table has the correct amount of elements\n";
    }
    else {
        cout << "The new table doesn't have the correct amount of elements, test failed\n";
        return false;
    }


    //verify that size of the new table is correct
    if (testCache.m_currentCap == testCache.findNextPrime((16) * 4)) {
        cout << "The new table's capactity is correct\n";
    }
    else {
        cout << "The new table's capacity is wrong, test failed\n";
        return false;
    }


    //and verify that every element still exists and can be found
    for (int i = 64; i < 80; i++) {

        if (!(testCache.getPerson(personList[i].getKey(), personList[i].getID()) == personList[i])) {
            cout << "Person who should be in the Cache isn't, test failed\n";
            return false;
        }
    }

    //if it goes through the loop and the loop terminates, the test passes
    //because no Person got lost
    cout << "Cache has everyone not deleted in it, test passed\n";
    return true;

}


//
//Helper Functions and Hash Functions Declared Below
//
unsigned int hashCode(const string str) {
    unsigned int val = 0;
    const unsigned int thirtyThree = 33;  // magic number from textbook
    for (unsigned int i = 0; i < str.length(); i++)
        val = val * thirtyThree + str[i];
    return val;
}


void Tester::makeRandomNormalTable(Cache* cache, int size, int amount, vector <Person>* dataList, int exclude) {

    Random RndID(MINID, MAXID);
    Random RndStr(MINSEARCH, MAXSEARCH);
  
    for (int i = 0; i < amount; i++) {
        // generating random data

        int ID = RndID.getRandNum();

        if (ID != exclude) {
            Person dataObj = Person(searchStr[RndStr.getRandNum()], ID);

            // saving data for later use
            //if a vector is passed, it will prevent duplicates
            if (dataList) {
                if (!(count(dataList->begin(), dataList->end(), dataObj))) {
                    dataList->push_back(dataObj);

                    // inserting data in to the Cache object
                    cache->insert(dataObj);
                }
            }
            else {
                // inserting data in to the Cache object
                cache->insert(dataObj);
            }
        }
 
    }
}