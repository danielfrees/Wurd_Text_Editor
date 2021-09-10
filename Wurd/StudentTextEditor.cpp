#include "StudentTextEditor.h"
#include "Undo.h"
#include <string>
#include <vector>


#include <iostream>         //for file loading
#include <fstream>

#include <cmath>        //for std::abs

TextEditor* createTextEditor(Undo* un)
{
	return new StudentTextEditor(un);
}

StudentTextEditor::StudentTextEditor(Undo* undo)
 : TextEditor(undo) {
     m_row = 0;
     m_col = 0;
     m_doc.push_back("");
     m_rowItr = m_doc.begin();  //same as .end(). Invalid right now with no text, do not access
     m_undoSubmissions = true;
}

StudentTextEditor::~StudentTextEditor()
{
    m_doc.clear();
}

bool StudentTextEditor::load(std::string file) {
    std::ifstream  infile(file);
    if (!infile){
        return false;           //if we couldn't load the file correctly
    }
    
    //reset anything that might be in the editor
    reset();
    
    //import the text into my text continaer
    std::string s;
    while ( std::getline(infile, s) ){
        if (!s.empty() && s.back() == '\r')       //remove any carriage returns at the end of the line (\n already taken care of by getline)
            s.pop_back();
        m_doc.push_back(s);
    }
    m_row = 0;              //reset editing position (now also get an iterator to current row)
    m_rowItr = m_doc.begin();
    m_col = 0;
    return true;
}

bool StudentTextEditor::save(std::string file) {
    std::ofstream outfile(file);
    if (!outfile)
        return false;
    
    std::list<std::string>::iterator saveItr = m_doc.begin();
    for (int i = 0; i < m_doc.size() && saveItr != m_doc.end(); i++){
        outfile << *saveItr + '\n';
        saveItr++;
    }
    return true;
}

void StudentTextEditor::reset() {
    std::list<std::string>::iterator clearItr = m_doc.begin();
    while (clearItr != m_doc.end()) {
        clearItr = m_doc.erase(clearItr);
    }
    m_row = 0;
    m_rowItr = m_doc.begin();  //same as .end(). Invalid right now with no text, do not access
    m_col = 0;
    
    getUndo()->clear();
}

void StudentTextEditor::move(Dir dir) {
    switch (dir){
        case UP:{
            if (m_row > 0){            //if not on first row, move pos up one line
                m_row--;
                m_rowItr--;
            }
            //adjust appropriately if beyond the end of the line you've moved to (to just past the end)
            if (m_col > static_cast<int>(m_rowItr->size()))
                m_col = static_cast<int>(m_rowItr->size());
            break;
        }
        case DOWN:{
            if(m_row < m_doc.size() - 1){       //if not on last row, move pos down one line
                m_row++;
                m_rowItr++;
            }
            if (m_col > static_cast<int>(m_rowItr->size()))
                m_col = static_cast<int>(m_rowItr->size());
            //adjust appropriately if beyond the end of the line you've moved to (to just past the end)
            break;
        }
        case LEFT:{
            if (m_col > 0){     //if not at leftmost spot, move left
                m_col--;
            }
            else if (m_row > 0){  //otherwise move up if there is a line above
                m_row--;
                m_rowItr--;
                m_col = static_cast<int>((*m_rowItr).size());       //and place col at the end of the line past last char on the line
            }
            break;
        }
        case RIGHT:{
            if (m_col < static_cast<int>((*m_rowItr).size())){     //if not at rightmost spot (one past last char on line), then move right
                m_col++;
            }
            else if (m_row < m_doc.size() - 1){  //otherwise move down if there's a line below
                m_row++;
                m_rowItr++;
                m_col = 0;       //and place col at the start of the line
            }
            break;
        }
        case HOME:{
            m_col = 0;              //go to start of line
            break;
        }
        case END:{
            m_col = static_cast<int>((*m_rowItr).size());            //go just after last char on line
        }
    }
}

void StudentTextEditor::del() {
    if (m_col >= 0 && m_col < m_rowItr->size()){  //if on a valid spot in the string for the line
        char erased = m_rowItr->at(m_col);
        m_rowItr->erase(m_col, 1);
        
        if (m_undoSubmissions)
            getUndo()->submit(Undo::Action::DELETE, m_row, m_col, erased);
    }
    else if (m_col == m_rowItr->size() && m_row < m_doc.size()-1 ){  //if just after end of a line (AND NOT ON THE LAST LINE), then we do a row merge with the row below
        std::list<std::string>::iterator nextLine = m_rowItr;
        nextLine++;
        m_rowItr->append(*nextLine);
        m_doc.erase(nextLine);
        
        if (m_undoSubmissions)
            getUndo()->submit(Undo::Action::JOIN, m_row, m_col);
    }
}

void StudentTextEditor::backspace() {
    if (m_col > 0 && m_col < m_rowItr->size() + 1){         //if not at start of line
        char erased = m_rowItr->at(m_col-1);
        m_rowItr->erase(m_col-1, 1);           //delete previous char
        m_col--;
        
        if (m_undoSubmissions)
            getUndo()->submit(Undo::Action::DELETE, m_row, m_col, erased);
    }
    //if m_col is 0 or row is empty (it will always be 0 when row is empty but extra check doesn't hurt)
    //we join the current line with the previous line
    //nothing happens if we're on the first row
    else if ( m_row > 0 && (m_col == 0 || m_rowItr->size() == 0)){
        std::list<std::string>::iterator currentLine = m_rowItr;
        m_rowItr--;
        m_row--;
        
        m_col = static_cast<int>(m_rowItr->size());
        m_rowItr->append(*currentLine);
        m_doc.erase(currentLine);
        
        if (m_undoSubmissions)
            getUndo()->submit(Undo::Action::JOIN, m_row, m_col);
    }
}

void StudentTextEditor::insert(char ch) {
    if (ch == '\t'){
        m_rowItr->insert(m_col, "    ");
        m_col += 4;
        //submit the tab insert! (special case, we added 4 spaces)
        if (m_undoSubmissions){
            for (int i = 0; i < 4; i++)
                getUndo()->submit(Undo::Action::INSERT, m_row, m_col - 3 + i, ' ');
        }
    }
    else{
        m_rowItr->insert(m_col, std::string (1, ch));
        m_col ++;                            //adjust editing position accordingly
        
        //submit the insert
        if (m_undoSubmissions)
            getUndo()->submit(Undo::Action::INSERT, m_row, m_col, ch);
    }
}

void StudentTextEditor::enter() {
    //grab initial row, col position for undo submit
    int startRow(m_row), startCol(m_col);
    
    std::list<std::string>::iterator newLine = m_rowItr;
    newLine++;                  //set up iterator so that we can insert a new line
    
    //break the line, and insert the latter bit onto a new line, then remove that from the old line
    m_doc.insert(newLine, m_rowItr->substr(m_col, m_rowItr->size()-(m_col)));
    *m_rowItr = m_rowItr->substr(0, m_col);
    
    //adjust editing position to front of new line
    m_rowItr++;
    m_row++;
    m_col = 0;
    
    if (m_undoSubmissions)
        getUndo()->submit(Undo::Action::SPLIT, startRow, startCol);
}

void StudentTextEditor::getPos(int& row, int& col) const {
    row = m_row;
    col = m_col;
}

int StudentTextEditor::getLines(int startRow, int numRows, std::vector<std::string>& lines) const{
    if (startRow < 0 || numRows < 0 || startRow > m_doc.size())
        return -1;
    
    int linesGotten = 0;
    std::list<std::string>::iterator getLineItr = m_rowItr;
    
    lines.clear();
    int rowDiff = startRow - m_row;
    for (int i = 0; i < std::abs(rowDiff); i++){        //move our row iterator to the starting row
        if (rowDiff < 0){
            getLineItr--;
        }
        else{
            getLineItr++;
        }
    }
    //iterator is now on the start Row
    for (int i = 0; i < numRows && getLineItr != m_doc.end(); i++){       //for number of rows requested (and making sure we don't go past one line beyond our doc).
        lines.push_back(*getLineItr);
        linesGotten++;
        getLineItr++;
    }
    
    return linesGotten;
}

void StudentTextEditor::undo() {
    //set undoSubmissions to false. We will not undo undo actions!
    m_undoSubmissions = false;
    
    //set up local vars for getting the undo step
    Undo::Action action;
    int row(0), col(0), count(0);
    std::string text("");
    
    action = getUndo()->get(row, col, count, text);
    
    //determine where the iterator will have to move
    int rowDiff = row - m_row;
    bool moveUp = (rowDiff > 0);
    
    switch (action){
        case Undo::Action::INSERT:{
            
            //adjust positioning
            m_row = row;
            if (moveUp){
                for (int i = 0; i < std::abs(rowDiff); i++){
                    m_rowItr++;
                }
            }
            else{
                for (int i = 0; i < std::abs(rowDiff); i++){
                    m_rowItr--;
                }
            }
            m_col = col;
            
            m_rowItr->insert(m_col, text);
            
            break;
        }
        case Undo::Action::DELETE:{
            
            //adjust positioning
            m_row = row;
            if (moveUp){
                for (int i = 0; i < std::abs(rowDiff); i++){
                    m_rowItr++;
                }
            }
            else{
                for (int i = 0; i < std::abs(rowDiff); i++){
                    m_rowItr--;
                }
            }
            m_col = col;          //this is where we want to delete from
            
            for (int i = 0; i < count; i++){
                del();
            }
                //m_rowItr->erase(m_col, 1);
                
            
            break;
        }
        case Undo::Action::SPLIT:{
            
            //adjust positioning
            m_row = row;
            if (moveUp){
                for (int i = 0; i < std::abs(rowDiff); i++){
                    m_rowItr++;
                }
            }
            else{
                for (int i = 0; i < std::abs(rowDiff); i++){
                    m_rowItr--;
                }
            }
            m_col = col;
            
            //split the line
            enter();
            
            //move back to undo positioning
            m_rowItr--;
            m_row = row;
            m_col = col;
            
            break;
        }
        case Undo::Action::JOIN:{
            
            //adjust positioning
            m_row = row;
            if (moveUp){
                for (int i = 0; i < std::abs(rowDiff); i++){
                    m_rowItr++;
                }
            }
            else{
                for (int i = 0; i < std::abs(rowDiff); i++){
                    m_rowItr--;
                }
            }
            m_col = col;
            
            //we are at the end of the line, so can use del for joining
            del();
            
            //we will still be at the correct undo positioning for our cursor
            
            break;
        }
        case Undo::Action::ERROR:{
            m_undoSubmissions = true;       //set this back to true before returning! undo submissions happen again
            return;
        }
    }
    //adjust positioning back to the return vals
    rowDiff = row - m_row;
    moveUp = (rowDiff > 0);
    m_row = row;
    if (moveUp){
        for (int i = 0; i < std::abs(rowDiff); i++){
            m_rowItr++;
        }
    }
    else{
        for (int i = 0; i < std::abs(rowDiff); i++){
            m_rowItr--;
        }
    }
    m_col = col;
    
    m_undoSubmissions = true;
}
