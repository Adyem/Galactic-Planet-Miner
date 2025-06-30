#include "journal.h"

Journal::Journal() : nextId(1) {}

void Journal::addEntry(const std::string &title, const std::string &text) {
    JournalEntry entry;
    entry.id = nextId++;
    entry.title = title;
    entry.text = text;
    entry.unlocked = true;
    entries.push_back(entry);
    std::cout << "New journal entry unlocked: [" << entry.id << "] " << entry.title << std::endl;
}

void Journal::listEntries() {
    std::cout << "Journal Entries:" << std::endl;
    for (const auto &entry : entries) {
        if (entry.unlocked)
            std::cout << "[" << entry.id << "] " << entry.title << std::endl;
    }
}

void Journal::viewEntry(int id) {
    for (const auto &entry : entries) {
        if (entry.id == id && entry.unlocked) {
            std::cout << "Journal Entry [" << entry.id << "] " << entry.title << ":\n";
            std::cout << entry.text << "\n";
            return;
        }
    }
    std::cout << "Journal entry not found or locked." << std::endl;
}

Journal journal;

