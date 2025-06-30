#ifndef JOURNAL_H
#define JOURNAL_H

#include <string>
#include <vector>
#include <iostream>

class Journal {
public:
    Journal();
    void addEntry(const std::string &title, const std::string &text);
    void listEntries();
    void viewEntry(int id);
private:
    struct JournalEntry {
        int id;
        std::string title;
        std::string text;
        bool unlocked;
    };
    std::vector<JournalEntry> entries;
    int nextId;
};

extern Journal journal;

#endif // JOURNAL_H
