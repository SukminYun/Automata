#include <stack>
#include <string>
#include <iostream>
using namespace std;

int main(){
    // 입력값과 입력값의 길이
    int linelength;
    string linput;   
    cin >> linelength >> linput;

    // the inputs of n, m q0, F
    int numState;
    int numTransition;
    int initialState;
    int finalState;
    cin >> numState >> numTransition >> initialState >> finalState;

    // transtion을 관리할 array
    int initialTable[numTransition];
    int finalTable[numTransition];
    char symbolTable[numTransition];

    // transition table 받기
    for (int i=0; i<numTransition; i++){
        int startState;
        int endState;
        char symbol;
        cin >> startState >> endState >> symbol;

        initialTable[i] = startState;
        finalTable[i] = endState;
        symbolTable[i] = symbol;
    }

    // main logic
    stack<int> curStack;
    curStack.push(initialState);

    // NFA가 수락가능한 string인지 표시하는 variable 
    bool isPossible = true;

    for (int i=0; i<linelength; i++){
        // step1. 입력 받은 현재 states에서 엡실론을 통해 이동할 state가 있는지 확인
        stack<int> epsilonStack;

        while (!curStack.empty()){
            int currentState = curStack.top();
            curStack.pop();

            // 엡실론으로 이동할 state가 있는지 확인하기
            bool isEpsilon = false;
            for (int j=0; j<numTransition; j++){
                if(initialTable[j]==currentState 
                    && symbolTable[j]=='e'){
                    // 엡실론으로 이동 가능한 state를 stack 위에 쌓기
                    curStack.push(finalTable[j]);
                    isEpsilon = true;
                }
            }

            // 더 이상 엡실론을 통해 이동할 state가 없는 경우
            // 다음 스택으로 넘기기
            if (!isEpsilon) {
                epsilonStack.push(currentState);
            }
        }

        // step2. step1을 통해 엡실론을 모두 처리한 state들 중 transition이 가능한 state가 있는지 확인
        stack<int> transStack;

        while (!epsilonStack.empty()){
            // currentState와 input symbol에 따라 transition
            int currentState = epsilonStack.top();
            epsilonStack.pop();
            char inputSymbol = linput[i];

            for (int j=0; j<numTransition; j++){
                if(initialTable[j]==currentState 
                    && symbolTable[j]==inputSymbol){
                    // transition stack에 바뀐 state를 쌓기
                    transStack.push(finalTable[j]);
                    break;
                }
            }
        }

        // step3. 바뀐 state가 있는지 확인
        // 만약 바뀐 state가 하나도 없다면 dead state로 처리
        if (transStack.empty()){
            isPossible = false;
            break;
        }
        else {
            curStack = transStack;
        }
    }

    // step4. 처음에 입력 받은 문자의 모든 값을 처리한 후, 남은 states가 엡실론을 통해 이동가능한지 확인
    stack<int> finalStack;

    while (!curStack.empty() && isPossible){
        int currentState = curStack.top();
        curStack.pop();

        // 엡실론으로 이동할 state가 있는지 확인하기
        bool isEpsilon = false;
        for (int j=0; j<numTransition; j++){
            if(initialTable[j]==currentState 
                && symbolTable[j]=='e'){
                // 엡실론으로 이동 가능한 state를 stack 위에 쌓기
                curStack.push(finalTable[j]);
                isEpsilon = true;
            }
        }

        // 더 이상 엡실론으로 이동할 state가 없는 경우
        // final stack으로 넘기기
        if (!isEpsilon) {
            finalStack.push(currentState);
        }
    }

    // step5. final stack안에 final state가 있는지 확인
    bool hasFinalstate = false;

    while (!finalStack.empty() && isPossible){
        int currentState = finalStack.top();
        finalStack.pop();

        if (currentState == finalState) {
            hasFinalstate = true;
            break;
        }
    }

    if (isPossible && hasFinalstate) {
        cout << "yes" << endl;
    }
    else {
        cout << "no" << endl;
    }
}