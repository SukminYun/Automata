#include <stack>
#include <string>
#include <iostream>
#include <vector>

using namespace std;

// step1. Change regular expression to postfix form
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

// postfix로 변환하는 function
string RegExprToPost(string regex){
    string output;
    stack<char> oper_stack;

    for (int i = 0; i < regex.length(); i++) {
        char c = regex[i];
        // 입력 값이 operand 일 때, 그대로 output에 추가
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
            // stack에 담긴 operator가 계산 순서가 더 빠를경우 stack에서 pop
            while (!oper_stack.empty() && oper_stack.top() != '(' &&
                   Precedence(oper_stack.top()) >= Precedence(c)) {
                output += oper_stack.top();
                oper_stack.pop();
            }
            oper_stack.push(c);
        }
    }

    // stack에 남은 operator 처리
    while (!oper_stack.empty()){
        output += oper_stack.top();
        oper_stack.pop();
    }
    return output;
}


// step2. construction of NFA from postfix form 
// transition을 한번에 기억하기 위한 struct
struct Transition {
    int from;
    int to;
    char symbol;

    // generator
    Transition(int f, int t, char s) : from(f), to(t), symbol(s) {}
};

// NFA를 group단위로 관리하기 위한 NFA class
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
        
        for (vector<Transition>::iterator it = second.transitions.begin(); it != second.transitions.end(); ++it) {
            Transition& transition = *it;
        // '.' 뒤에 오는 NFA의 transition 기록을 앞 NFA에 이어 붙임
            result.addTransition(transition.from, transition.to, transition.symbol);
        }

        // first NFA와 second NFA의 concatenate를 vector에 추가
        result.addTransition(first.finalState, second.initialState, 'e');
        result.numStates += second.numStates;
        result.finalState = second.finalState;

        return result;
    }

    // '+' 연산자가 입력될 때 실행
    static NFA unite(NFA& first, NFA& second, int& new_State_num) {
        NFA result;

        // 2개의 NFA를 연결할 state 2개를 추가 => (기존) + 2
        result.numStates = first.numStates + second.numStates + 2;
        result.initialState = new_State_num++;
        result.finalState = new_State_num++;

        // 합쳐질 result NFA에 first, second NFA의 transition을 새롭게 기록
        
        for (vector<Transition>::iterator it = first.transitions.begin(); it != first.transitions.end(); ++it) {
            Transition& transition = *it;
            result.addTransition(transition.from, transition.to, transition.symbol);
        }
        for (vector<Transition>::iterator it = second.transitions.begin(); it != second.transitions.end(); ++it) {
            Transition& transition = *it;
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
        for (vector<Transition>::iterator it = nfa.transitions.begin(); it != nfa.transitions.end(); ++it) {
            Transition& transition = *it;
            result.addTransition(transition.from, transition.to, transition.symbol);
        }

        // spec에 제시된 순서에 맞게 새로운 state와의 transition 기록
        result.addTransition(result.initialState, nfa.initialState, 'e');
        result.addTransition(result.initialState, result.finalState, 'e');

        result.addTransition(nfa.finalState, result.finalState, 'e');
        result.addTransition(nfa.finalState, nfa.initialState, 'e');

        return result;
    }
};

// 입력 받은 postfix를 바탕으로 NFA를 만드는 함수
NFA constructNFA(const string& postfix) {
    // 입력받는 postfix form에 따라 생성될 NFA를 저장하는 stack
    stack<NFA> nfaStack;

    // 새롭게 생성될 state의 숫자를 지정하는 변수
    int new_State_num = 0;

    // input에 따라 앞서 정의한 NFA class의 function을 실행
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

// 최종 NFA의 transition을 spec에 맞게 출력하는 함수
void printNFA(const NFA& nfa) {
    cout << nfa.numStates << " " << nfa.transitions.size() << " " << nfa.initialState << " " << nfa.finalState << endl;

    for (vector<Transition>::const_iterator it = nfa.transitions.begin(); it != nfa.transitions.end(); ++it) {
        const Transition& transition = *it;
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