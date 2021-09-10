#include "StudentSpellCheck.h"
#include <string>
#include <vector>

#include <cctype> //for toupper, isalpha etc.

#include <iostream>     //for loading dictionary
#include <fstream>

SpellCheck* createSpellCheck()
{
	return new StudentSpellCheck;
}

StudentSpellCheck::~StudentSpellCheck() {
    delete m_dictionary;
}

bool StudentSpellCheck::load(std::string dictionaryFile) {
    std::ifstream infile(dictionaryFile);
    if ( !infile ){
        return false;
    }
    //clear any old dictionary that may have been in there
    delete m_dictionary;
    m_dictionary = new Trie;
    //process the dictionary into the tree, line by line! (each line is a word). MARK: I make the words uniformly all caps here as I add them into the tree. Insert also does this but I have to parse the words to use only word chars anyway...
    std::string s("");
    std::string dicWord("");
    while ( std::getline(infile, s) ){
        for (int i = 0; i < s.size(); i++){
            if  ( std::isalpha(s[i]) )            //add only letters and apostrophes
                dicWord += std::toupper(s[i]);                          //add them all caps to make Trie simpler
            if (s[i] == '\'')
                dicWord += '\'';
        }
        if ( dicWord.size() > 0 )
            m_dictionary->insert(dicWord);
        dicWord.clear();
    }
    return true;
}

bool StudentSpellCheck::spellCheck(std::string word, int max_suggestions, std::vector<std::string>& suggestions) {
    
    if (m_dictionary->search(word))
        return true;
    else{
        suggestions.clear();
        for(int i = 0; i < word.size(); i++){                   //for each letter in the word...
            std::string testWord = word;             //reset our copy of the word
            for(int j = 0; j < ALPHABET_LENGTH + 1; j++){       //search for suggestions with only one char diff
                if ( j == ALPHABET_LENGTH)
                    testWord[i] = '\'';                         //check if ' in the char's place makes valid word
                else{   //and alpha chars too
                    if (std::isupper(word[i]))
                        testWord[i] = static_cast<char>('A' + j);
                    else
                        testWord[i] = static_cast<char>('a' + j);
                }
                if( m_dictionary->search(testWord) ){
                    suggestions.push_back(testWord);                //we've found a suggestion so add it
                    max_suggestions--;                              //count that we've added a suggestion
                }
                  
                if (max_suggestions <= 0)
                    return false;                   //return as soon as we find the max suggestions
            }
        }
    }
    return false;                   //if we reach here, we couldn't find max suggestions but the word was not in the dictionary
}


void StudentSpellCheck::spellCheckLine(const std::string& line, std::vector<SpellCheck::Position>& problems) {
    problems.clear();           //problems must be empty if we don't find any misspelled words
    
    int wordSize = 0;
    SpellCheck::Position pos;
    
    for (int s = 0; s < line.size(); s++){              //if encountering word chars keep building the word
        if (isalpha(line[s]) || line[s] == '\''){
            if ( wordSize == 0 )
                pos.start = s;
            wordSize++;
        }
        else if (wordSize != 0){             //if we had a word and hit a non word char, the word is complete
            pos.end = s - 1;                    //s-1 is the the end pos of the word
            if (! m_dictionary->search(line.substr(pos.start, wordSize)) )      //if the word is not in the dictionary
                problems.push_back(pos);                //add its position to the problems
            wordSize = 0;
        }
    }
    //another check for words ending at the end of the line
    if (wordSize != 0){             //if we had a word and hit a non word char, the word is complete
        pos.end = static_cast<int>(line.size()) - 1;                    //s-1 is the the end pos of the word
        if (! m_dictionary->search(line.substr(pos.start, wordSize)) )      //if the word is not in the dictionary
            problems.push_back(pos);                //add its position to the problems
        wordSize = 0;
    }
}

//**********TRIE STUFF*************
StudentSpellCheck::Trie::Trie(){
    m_root = new trieNode('\0');
}

StudentSpellCheck::Trie::~Trie(){
    deleteTrie(m_root);
}

void StudentSpellCheck::Trie::insert(const std::string& word){
    if ( search(word) )
        return;                         //nothing to do if the word is already in the trie
    
    //otherwise, add the word to the tree by adding new characters wherever they are not already in the trie
    trieNode* currentNode = m_root;
    
    for (int i = 0; i < word.size(); i++){
        char charToAdd = std::toupper(word[i]);
        int alphaIndex;
        if (charToAdd == '\'')
            alphaIndex = ALPHABET_LENGTH;
        else
            alphaIndex = charToAdd - 'A';
        if ( currentNode->m_children[alphaIndex] == nullptr ){               //only add a letter at each level if it wasn't there before..
            currentNode->m_children[alphaIndex] = new trieNode(charToAdd);
        }
        currentNode = currentNode->m_children[alphaIndex];
    }
    currentNode->m_wordEnd = true;     //MARK: Note when a word is terminated!
}

bool StudentSpellCheck::Trie::search(const std::string& word) const{
    int alphaIndex;                  //holds the child to check for each letter, letters always added to their dictionary index
    
    trieNode* currentNode = m_root;
    for (int i = 0; i < word.size(); i++){
        char searchChar = std::toupper(word[i]);
        if ( searchChar == '\'' )
            alphaIndex = ALPHABET_LENGTH;
        else
            alphaIndex = searchChar - 'A';
        if (currentNode->m_children[alphaIndex] == nullptr)
            return false;                                   //if missing any letters along the way, word is not in dictionary
        currentNode = currentNode->m_children[alphaIndex];
    }
    //if we are now at the end of a word we found it!
    return (currentNode->m_wordEnd);
}

void StudentSpellCheck::Trie::deleteTrie(trieNode* start){
    if (start == nullptr)               //don't delete empty nodes
        return;
    for (int childNum = 0; childNum < ALPHABET_LENGTH + 1; childNum++){         //delete all child subtrees
        deleteTrie(start->m_children[childNum]);
    }
    //then this node
    delete start;
}

//Recursive trieNode Destruction method
//recursively destroys the current trie node and all nodes below it
void StudentSpellCheck::Trie::trieNode::destroyNodeAndBelow(){
    int numChildren = ALPHABET_LENGTH + 1;
    for (int i = 0; i < ALPHABET_LENGTH + 1; i++){              //check if a leaf node
        if (m_children[i] == nullptr)
            numChildren--;
    }
    if (numChildren == 0)                   //if leaf node, just delete
        delete this;
    
    //otherwise delete children first
    for ( int i = 0; i < ALPHABET_LENGTH + 1; i++ ){
        if (m_children[i] != nullptr)
            m_children[i]->destroyNodeAndBelow();
    }
    //then delete this
    delete this;
}
