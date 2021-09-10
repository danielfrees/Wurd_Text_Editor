#include "StudentUndo.h"

Undo* createUndo()
{
	return new StudentUndo;
}

void StudentUndo::submit(const Action action, int row, int col, char ch) {
    //CHECK FOR BATCHING FIRST
    //delete batching:
    if (!m_stack.empty() && m_stack.top().m_action == DELETE && m_stack.top().m_row == row
             && m_stack.top().m_col == col ){
        //add the char deleted to be added back
        m_stack.top().m_text.append(std::string(1,ch));
    }
    //backspace batching:
    else if (!m_stack.empty() && m_stack.top().m_action == DELETE && m_stack.top().m_row == row
             && m_stack.top().m_col == col+1 ){
        //add the char deleted to be added back
        m_stack.top().m_text = ch + m_stack.top().m_text;
        //set row and col to the new spot
        m_stack.top().m_row = row;
        m_stack.top().m_col = col;
    }
    //insert batching:
    else if (!m_stack.empty() && m_stack.top().m_action == INSERT && m_stack.top().m_row == row && m_stack.top().m_col == col-1){
        
        //what was inserted will determine how much to delete, so track it in string
        m_stack.top().m_text += ch;  //implicitly counts how many chars were added so we know how many to delete
        //set row and col to the new spot
        m_stack.top().m_row = row;
        m_stack.top().m_col = col;
    }
    //non-batching case
    else{
        UndoStep newUndoStep(action, row, col, ch);
        m_stack.push(newUndoStep);
    }
}

StudentUndo::Action StudentUndo::get(int& row, int& col, int& count, std::string& text) {
    if (m_stack.empty()) //return ERROR and don't change params if empty undo stack
        return Action::ERROR;
    
    row = m_stack.top().m_row;
    col = m_stack.top().m_col;
    
    switch (m_stack.top().m_action){
        case Action::INSERT:{
            count = static_cast<int>(m_stack.top().m_text.size());
            col -= count;
            text = "";
            m_stack.pop();
            return Action::DELETE;
        }
        case Action::DELETE:{
            count = 1;
            text = m_stack.top().m_text;
            m_stack.pop();
            return Action::INSERT;
        }
        case Action::SPLIT:{
            count = 1;
            text = "";
            m_stack.pop();
            return Action::JOIN;
        }
        case Action::JOIN:{
            count = 1;
            text = "";
            m_stack.pop();
            return Action::SPLIT;
        }
        case Action::ERROR:{
            m_stack.pop();
            return Action::ERROR;
            //does nothing, will never be submitted anyway. Just here to get rid of warning.
        }
    }
    return Action::ERROR; //we will never get here but needed to remove g32 warning
}

void StudentUndo::clear() {
    while (!m_stack.empty())
        m_stack.pop();
}
