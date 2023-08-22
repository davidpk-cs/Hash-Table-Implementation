// CMSC 341 - Fall 2022 - Project 4
// David Premkumar

#include "cache.h"
Cache::Cache(int size, hash_fn hash) {
    
    m_hash = hash;

    //if size below min or max, make cap min or max
    if (size <= MINPRIME) {
        m_currentCap = MINPRIME;
    }
    else if(size >= MAXPRIME){
        m_currentCap = MAXPRIME;
    }

    //otherwise make cap the smallestPrime that is >= size
    else {
        m_currentCap = findNextPrime(size);
    }

    //initialize the rest of the member variables
    m_currentSize = 0;
    m_currNumDeleted = 0;

    //initialize the hash array
    m_currentTable = new Person[m_currentCap];
    
    for (int i = 0; i < m_currentCap; i++) {
        m_currentTable[i] = EMPTY;
    }

    
    //initialize member variables of old table
    m_oldTable = nullptr;
    
}

Cache::~Cache() {
    
    //just need to de-allocate the dynamic memory as it is
    delete[] m_currentTable;
    delete[] m_oldTable;
}

bool Cache::insert(Person person) {

    //quick validation against accepting people when size is max and load factor is at its peak
    if (m_currentCap == MAXPRIME) {
        m_currentSize++;
        if (lambda() > 0.5) {
            m_currentSize--;
            return false;
        }
    }

    //verify that the ID is valid, return false if it isn't
    if (person.getID() < MINID || person.getID() > MAXID) {
        return false;
    }

    //transfer 25% of the nodes if need be
    if (m_reSizingPhase != 0) {
        reSize();
    }
    
    //call the function that finds the right index to put it in
    int bucketNum = findIndex(person);

    if (bucketNum != -1) {
        //if a duplicate is already there, return false
        if (m_currentTable[bucketNum] == person) {
            return false;
        }
        m_currentTable[bucketNum] = person;
    }
    else {
        //if index is -1 there is no slot to insert, so force a full transfer and make it happen
        
        //if currently in the middle of resizing, force complete it
        if (m_reSizingPhase != 0) {
            m_reSizingPhase = 4;
            reSize();
        }
        else {
            //otherwise start resizing normally (just the first 25% transferred)
            reSize();
        }

        //and find the index again, it should not be an issue now
        bucketNum = findIndex(person);

        //if a duplicate is already there, return false
        if (m_currentTable[bucketNum] == person) {
            return false;
        }
        m_currentTable[bucketNum] = person;
    }

    m_currentSize++; //increment size
    
    //not in the middle of resizing and load factor exceeds 0.5, then resize
    if (lambda() >= 0.5 && m_reSizingPhase == 0) {
        reSize();
    }
    //transfer 25% of the nodes if need be
    else if (m_reSizingPhase != 0) {
        reSize();
    }

    return true;
}

bool Cache::remove(Person person) {
    
    //verify that ID is valid, return false if it isn't
    if (person.getID() < MINID || person.getID() > MAXID) {
        return false;
    }

    //give it base hash value
    int bucketNum = m_hash(person.m_key) % m_currentCap;
    
    //for calculating with the probinb function
    int i = 0;

    //iterate and update bucketNumber until we get to an empty non-deleted key or the one we want to delete
    while (!(m_currentTable[bucketNum] == person) && !(m_currentTable[bucketNum] == EMPTY)) {

        bucketNum = ((person.getID() % m_currentCap) + (i * i)) % m_currentCap;
        i++;

    }

    //if we are on the right one, delete it and return true, otherwise search the old Table
    if(m_currentTable[bucketNum] == person) {

        m_currentTable[bucketNum] = DELETED;
        m_currNumDeleted++;

        //start the shift to a larger table if need be
        if (deletedRatio() > 0.8 && m_reSizingPhase == 0) {
            reSize();
        }
        //transfer 25% of the nodes if need be
        else if (m_reSizingPhase != 0) {
            reSize();
        }
        return true;
    }

    //otherwise repeat the same process with the old table
    if (m_oldTable == nullptr) {
        return false; //nothing in old table to check
    }

    //reset hash value
    bucketNum = m_hash(person.m_key) % m_oldCap;
    i = 0;

    while (m_oldTable[bucketNum].getID() != 0 && m_oldTable[bucketNum].getID() != person.getID() && m_oldTable[bucketNum].getKey() == DELETEDKEY) {
        bucketNum = ((person.getID() % m_oldCap) + (i * i)) % m_oldCap;
        i++;

    }

    //if we are on the right one, delete it and return true, otherwise, the Person doesn't exist, so return false
    if (m_oldTable[bucketNum].getID() == person.getID()) {

        m_oldTable[bucketNum] = DELETED;
        m_oldNumDeleted++;
        return true;
    }
    
    //person doesn't exist
    return false;

}

Person Cache::getPerson(string key, int id) const {

    //verify that ID is valid, return an empty person if it isn't
    if (id < MINID || id > MAXID) {
        return EMPTY;
    }
    
    int bucketNum = m_hash(key) % m_currentCap; //the default bucket this goes

    int i = 0; //for quadratic probing

    //iterate until an empty bucket or the right one is found
    while (m_currentTable[bucketNum].getID() != id && !(m_currentTable[bucketNum] == EMPTY)) {
        bucketNum = ((id % m_currentCap) + (i * i)) % m_currentCap;
        i++;
    }

    //if Person is found, return it
    if (m_currentTable[bucketNum] == Person(key, id)) {
        return m_currentTable[bucketNum];
    }

    //if Person isn't found, and the old table is empty, return the empty person
    else if(m_oldTable == nullptr) {
        return m_currentTable[bucketNum];
    }

    //otherwise, search for the person in the old table
    bucketNum = m_hash(key) % m_oldCap;
    i = 0;

    //iterate through the old table
    while (m_oldTable[bucketNum].getID() != id && !(m_oldTable[bucketNum] == EMPTY)) {
        bucketNum = ((id % m_oldCap) + (i * i)) % m_oldCap;
        i++;
    }

    //return the person, it would be empty if the person doesn't exist
    return m_oldTable[bucketNum];
}

float Cache::lambda() const {
    
    //convert to float so it returns as a float without rounding
    return float(float(m_currentSize) / float(m_currentCap));
}

float Cache::deletedRatio() const {

    //to prevent undefined behavior from dividing by 0
    //not in issue in lambda since m_currentCap could never be 0
    if (float(m_currentSize) == 0) {
        return 0;
    }
    
    //convert to float so it returns as a float without rounding
    return float(float(m_currNumDeleted) / float(m_currentSize));
}

void Cache::dump() const {
    cout << "Dump for the current table: " << endl;
    if (m_currentTable != nullptr)
        for (int i = 0; i < m_currentCap; i++) {
            cout << "[" << i << "] : " << m_currentTable[i] << endl;
        }
    cout << "Dump for the old table: " << endl;
    if (m_oldTable != nullptr)
        for (int i = 0; i < m_oldCap; i++) {
            cout << "[" << i << "] : " << m_oldTable[i] << endl;
        }
}

bool Cache::isPrime(int number) {
    bool result = true;
    for (int i = 2; i <= number / 2; ++i) {
        if (number % i == 0) {
            result = false;
            break;
        }
    }
    return result;
}

int Cache::findNextPrime(int current) {
    //we always stay within the range [MINPRIME-MAXPRIME]
    //the smallest prime starts at MINPRIME
    if (current < MINPRIME) current = MINPRIME - 1;
    for (int i = current; i < MAXPRIME; i++) {
        for (int j = 2; j * j <= i; j++) {
            if (i % j == 0)
                break;
            else if (j + 1 > sqrt(i) && i != current) {
                return i;
            }
        }
    }
    //if a user tries to go over MAXPRIME
    return MAXPRIME;
}

ostream& operator<<(ostream& sout, const Person& person) {
    if (!person.m_key.empty())
        sout << person.m_key << " (ID " << person.m_id << ")";
    else
        sout << "";
    return sout;
}

bool operator==(const Person& lhs, const Person& rhs) {
    return ((lhs.m_key == rhs.m_key) && (lhs.m_id == rhs.m_id));
}


//------------------------------------------------------------
//Private Functions Declared Here
//------------------------------------------------------------

int Cache::findIndex(Person person) {

    //first, search for the person in the old table and delete them if need be
    int bucketNum;
    int i = 0;

    if (m_oldTable != nullptr) {
        bucketNum = m_hash(person.getKey()) % m_oldCap;

        //iterate through the old table
        while (m_oldTable[bucketNum].getID() != person.getID() && !(m_oldTable[bucketNum] == EMPTY)) {
            bucketNum = ((person.getID() % m_oldCap) + (i * i)) % m_oldCap;
            i++;
        }

        //if the person in question is there, delete them
        if (m_oldTable[bucketNum] == person) {
            m_oldTable[bucketNum] = DELETED;
            m_oldNumDeleted++;
        }
    }


    int firstDeletedIndex = -1; //note where we would insert in a deleted index if we need to

    bucketNum = m_hash(person.getKey()) % m_currentCap; //where the has function places it

    i = 0; //for the quadratic probing function

    //loop if its not at the right person or if the bucket is deleted, and the probing cycle hasn't returned to the original
    while (m_currentTable[bucketNum].getID() != person.getID() && !(m_currentTable[bucketNum] == EMPTY)) {

        //note the first deleted Index
        if (m_currentTable[bucketNum] == DELETED) {
            if (firstDeletedIndex == -1) {
                firstDeletedIndex = bucketNum;
            }
        }

        //quadratic probing
        bucketNum = ((person.getID() % m_currentCap) + (i * i)) % m_currentCap;
        i++;
    }

    //otherwise, see if person already exists
    if (m_currentTable[bucketNum] == person) {
        //return bucketNum since person already exists
        return bucketNum;
    }

    //then see if an empty bucket is found, and there is no earlier deleted spot to insert
    else if (m_currentTable[bucketNum] == EMPTY && firstDeletedIndex == -1) {
        return bucketNum;
    }

    //otherwise, return the first deleted index
    //this will be -1 if there is no room to insert the bucket
    return firstDeletedIndex;

}


void Cache::reSize() {

    //set-up if oldTable is empty
    if (m_reSizingPhase == 0) {
        
        m_reSizingPhase++;
        m_currentIndex = 0;

        //copy current table to old table
        m_oldTable = m_currentTable;
        m_oldCap = m_currentCap;
        m_oldSize = m_currentSize;
        m_oldNumDeleted = m_currNumDeleted;

        //assign how many People is 25% of the old table
        m_quarterPush = floor(float(float(m_oldSize) / float(4.0)));


        //construct a new current Table
        m_currentCap = findNextPrime((m_currentSize - m_currNumDeleted) * 4);
        m_currentTable = new Person[m_currentCap];

        for (int i = 0; i < m_currentCap; i++) {
            m_currentTable[i] = EMPTY;
        }

        m_currentSize = 0;
        m_currNumDeleted = 0;
    }

    int i = 0; //for iterating through 25%
    
    //iterate until table end is reached or 25% of the nodes are transferred
    //if on the 4th transfer phase, iterate all the way until the end of the table
    while ((i < m_quarterPush || m_reSizingPhase == 4) && m_currentIndex < m_oldCap) {
     
        //if node not empty or deleted, transfer it
        if (!(m_oldTable[m_currentIndex] == DELETED) && !(m_oldTable[m_currentIndex] == EMPTY)) {

            altInsert(m_oldTable[m_currentIndex]);
            m_oldTable[m_currentIndex] = DELETED;

            m_oldNumDeleted++;

            i++;
        }

        //move up the hash table
        m_currentIndex++; //keep iterating through table

    }

    //if the end full old table is reached
    //push the transfer phase to 4 to trigger the deletion of the old table
    if (m_currentIndex >= m_oldCap) {
        m_reSizingPhase = 4;
    }

    //if we just transferred the last 25% of the nodes
    //then remove traces of old table
    if (m_reSizingPhase == 4) {
        delete[] m_oldTable;
        m_oldTable = nullptr;
        m_oldCap = 0;
        m_oldSize = 0;
        m_oldNumDeleted = 0;

        m_reSizingPhase = 0;
        m_quarterPush = 0;
        m_currentIndex = 0;

        //if ratio goes up restart the resizing process
        if (lambda() >= 0.5) {
            reSize();
        }
    }

    //otherwise, move transferring phase up by 1
    else {
        m_reSizingPhase++;
    }

}


void Cache::altInsert(Person person) {
    
    //same as insert, but does not look at old table
    int bucketNum = altFindIndex(person);

    //only insert if there is a slot
    if (bucketNum != -1) {
        m_currentTable[bucketNum] = person;
    }

    m_currentSize++;
}


int Cache::altFindIndex(Person person) {

    int i;
    int bucketNum;

    int firstDeletedIndex = -1; //note where we would insert in a deleted index if we need to

    bucketNum = m_hash(person.getKey()) % m_currentCap; //where the has function places it

    i = 0; //for the quadratic probing function

    //loop if its not at the right person or if the bucket is deleted, and the probing cycle hasn't returned to the original
    while (m_currentTable[bucketNum].getID() != person.getID() && !(m_currentTable[bucketNum] == EMPTY)) {

        //note the first deleted Index
        if (m_currentTable[bucketNum] == DELETED) {
            if (firstDeletedIndex == -1) {
                firstDeletedIndex = bucketNum;
            }
        }

        //quadratic probing
        bucketNum = ((person.getID() % m_currentCap) + (i * i)) % m_currentCap;
        i++;
    }

    //otherwise, see if person already exists
    if (m_currentTable[bucketNum] == person) {
        //return bucketNum since person already exists
        return bucketNum;
    }

    //then see if an empty bucket is found, and there is no earlier deleted spot to insert
    else if (m_currentTable[bucketNum] == EMPTY && firstDeletedIndex == -1) {
        return bucketNum;
    }

    //otherwise, return the first deleted index
    //this will be -1 if there is no room to insert the bucket
    return firstDeletedIndex;
}