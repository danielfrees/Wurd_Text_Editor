#ifndef STUDENTUNDO_H_
#define STUDENTUNDO_H_

#include "Undo.h"

#include <stack>
#include <string>

class StudentUndo : public Undo {
public:

	void submit(Action action, int row, int col, char ch = 0);
	Action get(int& row, int& col, int& count, std::string& text);
	void clear();

private:
    struct UndoStep{
        UndoStep(const Action& action, const int& row, const int& col, const char& ch)
        {
            m_action = action;
            m_row = row;
            m_col = col;
            m_text = "";
            m_text += ch;
        }

        Action          m_action;
        int             m_row;
        int             m_col;
        std::string     m_text;
    };
    
    std::stack<UndoStep>        m_stack;
};

#endif // STUDENTUNDO_H_
