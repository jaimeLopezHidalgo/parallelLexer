//Creado por:
//Jaime López-Hidalgo

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

//-----------------------------global variables:-------------------------------------
vector<vector<string>> table;
vector<string> frags;
vector<string> categories;

int k = 1;
int typeCol=0;

//-----------------------------auxiliary functions:-------------------------------------

void displayTable(){
    for (int i=0; i<table.size(); i++) {
        for (int j=0; j<table[i].size(); j++) {
            if (j!=table[i].size()-1) {
                cout<<table[i][j]<<",";
            }else{
                cout<<table[i][j]<<endl;
            }
        }
    }
}

long indOf(char c){
    vector<string> sigma=table[0];
    for (int i=0; i<sigma.size(); i++) {
        if (sigma[i].find(c)!=string::npos) {
            return i;
        }
    }
    return -1;
}

void displayFragsWithCategories(){
    for (int i=0; i<frags.size(); i++) {
        cout<<frags[i]<<"   "<<categories[i]<<endl;
    }
}


void fragsHtml(string fileName){
string start, end;

start = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>Document</title><style>:root{--variable:  #3cb44b;--error:  #e6194B;--simbolo:  #469990;--operador:  #911eb4;--numero:  #ffe119;--especial:  #9A6324;--reservada:  #f032e6;--comentario:  #808000;--logico:  #4363d8;}span {white-space: pre-wrap;}.variable{color: var(--variable);}.error{color: var(--error);}.simbolo{color: var(--simbolo);}.operador{color: var(--operador);}.numero{color: var(--numero);}.especial{color: var(--especial);}.reservada{color: var(--reservada);}.comentario{color: var(--comentario);}.logico{color: var(--logico);}body {background-color: rgb(27, 27, 29);}</style></head><body>";
end = "</body></html>";

for (int i=0; i<4; i++) {
    fileName.pop_back();
}
ofstream htmlFile(fileName+".html");

htmlFile << start;
for (int i=0; i<frags.size(); i++) {
        htmlFile<<"<span class=\""<<categories[i]<<"\">"<<frags[i] << "</span>";
    }
htmlFile << end;
htmlFile.close();

}

long getStateInd(string state){
    long qInd;
    if (state!="p") {
        qInd=state.find('q');
        state=state.substr(qInd+1,state.size()-1);
        return stoi(state)+k;
    }
    return table.size()-1;
}

string getCurrentCategory(string state){
    return table[getStateInd(state)][typeCol];
}


bool charIsValid(char c){
    return indOf(c)!=-1;
}

string advanceState(string currentState, char c){
    long cInd;
    string nextState;
    vector<string> currRow;
    
    currRow=table[getStateInd(currentState)]; //extract current row
    if(!charIsValid(c)){
        nextState="p";
    }else{
        cInd=indOf(c);//get index of 'c' on state table
        nextState=currRow[cInd]; //obtain next state given by 'c'
    }
    
    return nextState;
}


string getFragCategory(string frag){
    char c;
    string category;
    string currentState="q0";
    
    for (int i=0; i<frag.size(); i++) {
        c=frag[i];
        currentState=advanceState(currentState, c);
        category=getCurrentCategory(currentState);
    }
    return category;
}


bool mustBeIndividual(char c){
    string alwaysIndividualChars = "=+*/^()'; \t\n";
    return (alwaysIndividualChars.find(c) != string::npos);
}


void parseError(string frag){
    //Split error by minus ('-'):
    
    char c;
    string word;
    string currentState="q0";
    string wordCategory;
    bool fragEnded;
    
    for (int i=0; i<frag.size(); i++) {
        c=frag[i];
        fragEnded = (i==frag.size()-1);
        if (c=='-') {
            if(frag.size()){
                frags.push_back(word); //push current word
                wordCategory=getFragCategory(word); //final category of current word
                categories.push_back(wordCategory); //push category
                word.clear();
            }
            fragEnded=true;
        }
        word.push_back(c);
        
        if(fragEnded){
            frags.push_back(word); //push current word
            wordCategory=getFragCategory(word); //final category of current word
            categories.push_back(wordCategory); //push category
            word.clear();
        }
    }
}

void loadFragAndCat(string frag, string currentCategory){
    if (frag.size()) {
        if(currentCategory=="error"){//Whenever word is error
            parseError(frag);
        }else{
            frags.push_back(frag);
            categories.push_back(currentCategory);
        }
    }
}

long distanceToNext(string file, int i, string delims){
    string fileCopy = file.substr(i,file.size()-i);
    long nextDelim = fileCopy.find_first_of(delims);
    if (nextDelim==-1) {
        nextDelim=fileCopy.size();
    }
    return nextDelim;
}

//-----------------------------primary functions:-------------------------------------

void processFile(string fileName){
    fstream ifile(fileName); //open input file
    
    if(!ifile.fail()){
        char c;
        string file;
        string frag;
        string currentState="q0";
        string currentCategory;
        bool fragEnded;
        
        //Convert file to a string
        while (ifile >> noskipws >> c) {
            file+=c;
        }
        
        for (int i=0; i<file.size(); i++) {//For each char
            c=file[i];
            fragEnded=(i==file.size()-1);
            if(mustBeIndividual(c)){//If individual char
                fragEnded=true;
            }else{
                //rest of characters (normal flow):
                currentState=advanceState(currentState, c);
                currentCategory=getCurrentCategory(currentState);
                frag.push_back(c);
            }
        
            if(fragEnded){
                loadFragAndCat(frag, currentCategory);
                frag.clear();
                if(mustBeIndividual(c)){
                    frag.push_back(c);
                    currentState="q0";
                    currentCategory=getCurrentCategory(advanceState(currentState, c));
                    if (c==' ' or c=='\n' or c=='\t') { //If delimiter char (space, tab or newline)
                        if(c==' '){
                            frag = "&nbsp;";
                        }
                        else if(c=='\n'){
                            frag = "<br>";
                        }
                        else{
                            frag = "&emsp;";
                        }
                        currentCategory.clear(); //No category needed
                    }else if (c==';' or c=='\'') {
                        long dist=distanceToNext(file, i, "; \t\n"); //Where comment/symbol ends
                        if(c==';'){
                            dist=distanceToNext(file, i, "\n"); //Where comment/symbol ends
                        }
                        frag=file.substr(i,dist);
                        i+=(dist-1);
                    }
                    loadFragAndCat(frag, currentCategory);
                    frag.clear();
                }
                fragEnded=false;
            }
        }
        ifile.close();
        fragsHtml(fileName);
    }else{
        cout<<"File '"<<fileName<<"' not found"<<endl;
    }
}


void loadTable(string tablefileName, string txtFileName){
    fstream ifile(tablefileName);
    string row;
    vector<string> matRow;
    long nextComma;

    if (!ifile.fail()) {
        while (getline(ifile,row,'\n')) {
            row=row.substr(0,row.size()-1);
            matRow.clear();
            while (row.size()>0) {
                nextComma = row.find(',');
                if (nextComma!=string::npos) {
                    matRow.push_back(row.substr(0,nextComma));
                    row.erase(0,nextComma+1);
                }else{
                    matRow.push_back(row);
                    row.clear();
                }
            }
            table.push_back(matRow);
        }
        ifile.close();
        processFile(txtFileName);
    }else{
        cout<<"File '"<<tablefileName<<"' not found"<<endl;
    }
}

int main(int argc, const char * argv[]) {
    loadTable("stateTable.csv", "scheme.txt");
    return 0;
}
