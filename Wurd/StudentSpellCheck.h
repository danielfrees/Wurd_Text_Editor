#ifndef STUDENTSPELLCHECK_H_
#define STUDENTSPELLCHECK_H_

#include "SpellCheck.h"

#include <string>
#include <vector>

const int ALPHABET_LENGTH = 26;

class StudentSpellCheck : public SpellCheck {
public:
    StudentSpellCheck() {
        m_dictionary = new Trie();
    }
	virtual ~StudentSpellCheck();
	bool load(std::string dict_file);
	bool spellCheck(std::string word, int maxSuggestions, std::vector<std::string>& suggestions);
	void spellCheckLine(const std::string& line, std::vector<Position>& problems);

private:
    
    //Trie
    struct Trie{
        Trie();
        ~Trie();
        void insert(const std::string& word);
        bool search(const std::string& word) const;
        
    private:
        struct trieNode{
            trieNode(char charVal){
                m_char = charVal;
                for (int i = 0; i < ALPHABET_LENGTH + 1 ; i++)      //1 extr at ALPHABET_LENGTH index for '
                    m_children[i] = nullptr;
                m_wordEnd = false;
            }
            void destroyNodeAndBelow();
            
            char                            m_char;
            trieNode*                       m_children[ALPHABET_LENGTH + 1];
            bool                            m_wordEnd;
        };
        
        //private helper
        void deleteTrie(trieNode* start);
        
        //Trie member vars
        trieNode*               m_root;
    };
    
    //SpellCheck member vars
    Trie*            m_dictionary;
    
};


#endif  // STUDENTSPELLCHECK_H_
