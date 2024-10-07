#include <stack>
#include <string>
#include <iostream>
#include <vector>

using namespace std;


// About Regular expression to postfix form
int Precedence(char oper) {
    switch (oper)
    {
    case '*':
        return 3;
        break;
    case '.':
        return 2;
        break;
    default:
        return 1;
        break;
    }
}

string RegExprToPost(string regex){
    string output;
    stack<char> oper_stack;

    for (int i = 0; i < regex.length(); i++) {
        char c = regex[i];
        // 입력 값이 operand 일 때
        if (c == '0' || c =='1') {
            output += c;
        }

        // 입력 값이 괄호 일 때
        else if (c=='(') {
            oper_stack.push(c);
        }
        else if (c==')') {
            // 괄호가 닫힐 때 stack에 있는 연산자 모두 pop
            while (!oper_stack.empty() && oper_stack.top() != '(') {
                output += oper_stack.top();
                oper_stack.pop();
            }

            // '(' 제거
            if (!oper_stack.empty()) {
                oper_stack.pop();
            }
        }

        // 입력 값이 operator일 때
        else {
            while (!oper_stack.empty() && oper_stack.top() != '(' &&
                   Precedence(oper_stack.top()) >= Precedence(c)) {
                output += oper_stack.top();
                oper_stack.pop();
            }
            oper_stack.push(c);
        }
    }

    while (!oper_stack.empty()){
        output += oper_stack.top();
        oper_stack.pop();
    }

    return output;
}


// About construction of NFA from postfix form 
struct Transition {
    int from;
    int to;
    char symbol;

    Transition(int f, int t, char s) : from(f), to(t), symbol(s) {}
};

class NFA {
    public:
        // NFA group 안의 state의 개수
        int numStates;

        // transition의 initial state 및 final state
        int initialState;
        int finalState;

        // 해당 NFA 안에 있는 transition의 종류
        vector<Transition> transitions;

    NFA() : numStates(0), initialState(0), finalState(0)  {}

    // transition vector에 해당 transition을 추가
    void addTransition(int from, int to, char symbol) {
        transitions.push_back(Transition(from, to, symbol));
    }

    // 0 | 1 이 들어올 때, 기본 transition 생성
    static NFA createBasicNFA(char symbol, int& new_State_num) {
        NFA nfa;
        nfa.numStates = 2;
        nfa.initialState = new_State_num++;
        nfa.finalState = new_State_num++;
        nfa.addTransition(nfa.initialState, nfa.finalState, symbol);
        return nfa;
    }

    // '.' 연산자가 입력될 때 실행
    static NFA concatenate(NFA& first, NFA& second) {
        NFA result = first;
        
        // '.' 뒤에 오는 NFA의 transition 기록을 앞 NFA에 이어 붙임
        for (auto& transition : second.transitions) {
            result.addTransition(transition.from, transition.to, transition.symbol);
        }

        // first NFA와 second NFAdml concat를 vector에 추가
        result.addTransition(first.finalState, second.initialState, 'e');
        result.numStates += second.numStates;
        result.finalState = second.finalState;

        return result;
    }

    // '+' 연산자가 입력될 때 실행
    static NFA unite(NFA& first, NFA& second, int& new_State_num) {
        NFA result;

        // 2개의 NFA를 연결할 state 2개를 추가
        result.numStates = first.numStates + second.numStates + 2;
        result.initialState = new_State_num++;
        result.finalState = new_State_num++;

        // 합쳐질 result NFA에 first, second NFA의 transition을 새롭게 기록
        for (auto& transition : first.transitions) {
            result.addTransition(transition.from, transition.to, transition.symbol);
        }
        for (auto& transition : second.transitions) {
            result.addTransition(transition.from, transition.to, transition.symbol);
        }

        // 두 NFA를 연결할 새로운 두개의 State에 대한 transition 작성
        result.addTransition(result.initialState, second.initialState, 'e');
        result.addTransition(result.initialState, first.initialState, 'e');

        result.addTransition(second.finalState, result.finalState, 'e');
        result.addTransition(first.finalState, result.finalState, 'e');
        
        return result;
    }

    // '*' 연산자가 입력될 때 실행
    static NFA kleeneStar(NFA& nfa, int& new_State_num) {
        NFA result;

        // '*' 연산을 위한 state 2개 추가
        result.numStates = nfa.numStates + 2;
        result.initialState = new_State_num++;
        result.finalState = new_State_num++;

        // 기존 NFA의 transition 기록을 이어서 작성
        for (auto& transition : nfa.transitions) {
            result.addTransition(transition.from, transition.to, transition.symbol);
        }

        // e.g 순서에 맞게 새로운 state와의 transition 기록
        result.addTransition(result.initialState, nfa.initialState, 'e');
        result.addTransition(result.initialState, result.finalState, 'e');

        result.addTransition(nfa.finalState, result.finalState, 'e');
        result.addTransition(nfa.finalState, nfa.initialState, 'e');

        return result;
    }
};

NFA constructNFA(const string& postfix) {
    // 입력받는 postfix form에 따라 생성될 NFA를 저장하는 stack
    stack<NFA> nfaStack;

    // 새롭게 생성될 state의 숫자를 지정하는 변수
    int new_State_num = 0;

    // input별로 위에서 정의한 function을 실행
    for (int i = 0; i < postfix.length(); i++) {
        char c = postfix[i];

        if (c == '0' || c == '1') {
            nfaStack.push(NFA::createBasicNFA(c, new_State_num));
        } 
        
        else if (c == '.') {
            NFA second = nfaStack.top(); 
            nfaStack.pop();

            NFA first = nfaStack.top(); 
            nfaStack.pop();
            
            nfaStack.push(NFA::concatenate(first, second));
        } 
        
        else if (c == '+') {
            NFA second = nfaStack.top(); 
            nfaStack.pop();

            NFA first = nfaStack.top(); 
            nfaStack.pop();
            
            nfaStack.push(NFA::unite(first, second, new_State_num));
        } 
        
        else if (c == '*') {
            NFA operand = nfaStack.top(); 
            nfaStack.pop();
            
            nfaStack.push(NFA::kleeneStar(operand, new_State_num));
        }
    }

    // final result
    return nfaStack.top();
}

void printNFA(const NFA& nfa) {
    cout << nfa.numStates << " " << nfa.transitions.size() << " " << nfa.initialState << " " << nfa.finalState << endl;

    for (const auto& transition : nfa.transitions) {
        cout << transition.from << " " << transition.to << " " << transition.symbol << endl;
    }
}

int main() {
    int lineN;
    string input;
    cin >> lineN >> input;

    string output = RegExprToPost(input);
    NFA result = constructNFA(output);
    printNFA(result);
}