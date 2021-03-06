#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <cmath>

using namespace std;

#define PASSLOCATION "pass"
#define DEFAULTPASS {(char)1}
#define ENCRPTEDSTRMAX 15

#define C_COPYPASS 'c'
#define C_REUSEPASS 'r'
#define C_QUIT 'q'

#define CHAR_TOUSE "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define STR_PROMPT "Click 'c' to copy it to clipboard.\nClick 'r' to reuse the program.\nClick 'q' to quit the program.\n"

string ctousestr = CHAR_TOUSE;

void splitarg(vector<string> *args, char* str);
void copyToClipboard(string &str);
int _getch();

template<typename t> void fillBuffer(t* buffer, int len, t what){
  for(int str_i = 0; str_i < len; str_i++)
    buffer[str_i] = what;
}

template<typename type, typename casttype = type>
  void ArrayToVector(type *arrayptr, int arraysize, vector<casttype> *newvectorptr){
    for(int i = 0; i < arraysize; i++)
      newvectorptr->insert(newvectorptr->end(), (casttype)arrayptr[i]);
  }

/**
 * strlength is a length of the password
 * 
 * Encryption math (for every c1 in str1 with nested for every c2 in str2)
    (pow2(c1*c2) +cr) % CHAR_TOUSE.len

    cr: current char in res
 */
string encryptString(string str, string base, int strlength){
  char buffer[strlength+1];
  fillBuffer(buffer, strlength, ctousestr[0]);
  int buffer_iter = 0;
  for(int str_i = 0; str_i < str.length(); str_i++){
    char currentc1 = str[str_i];
    for(int base_i = 0; base_i < base.length(); base_i++){
      char currentc2 = base[base_i];
      char *currchar = &buffer[buffer_iter % strlength];
      *currchar = ctousestr[((int)pow(currentc1 * currentc2, 2) + *currchar) % ctousestr.length()];
      buffer_iter++;
    }
  }

  buffer[strlength] = '\0';
  return string(buffer);
}

void storePass(string pass){
  ofstream ofs;
  ofs.open(PASSLOCATION, ofs.trunc);
  string e_pass = encryptString(pass, string(DEFAULTPASS), pass.length());

  ofs.write(e_pass.c_str(), e_pass.length());
  ofs.close();
}

string getEPass(){
  string res = "";
  ifstream ifs;
  ifs.open(PASSLOCATION, ifs.binary);
  char c = 0;
  ifs.read(&c, 1);
  while(!ifs.eof()){
    res += c;
    ifs.read(&c, 1);
  }

  return res;
}

string promptPass(vector<string> *args){
  for(int arg_i = 0; arg_i < args->size(); arg_i++){
    if(args->operator[](arg_i)[0] == '-'){
      switch(args->operator[](arg_i)[1]){
        
        //to save/change the main password
        case 'm':{
          string str = "";
          for(int str_i = 2; str_i < args->operator[](arg_i).length(); str_i++)
            str += args->operator[](arg_i)[str_i];

          storePass(str);
        }
      }
    }
  }
  
  {
    ifstream fs;
    fs.open(PASSLOCATION, fs.ate);
    if(fs.tellg() <= 0){
      cout << "\nYour main password isn't stored yet.\nWrite your password here: " << flush;
      string pass = "";
      char c = '\0';
      while((c = _getch()) != '\r' && c != '\n')
        pass += c;
      
      storePass(pass);
      cout << endl;
    }
  }

  cout << "Enter domain: " << flush;
  string anchorstr = "";
  char c = 0;
  while((c = getchar()) != '\n')
    anchorstr += c;
  
  string epass = getEPass();
  return encryptString(anchorstr, epass, ENCRPTEDSTRMAX);
}

void splitarg(vector<string> *vstr, char *str){
  int str_i = 0;
  bool stillspace = false, doloop = true;
  string tmp = "";
  while(doloop){
    switch(str[str_i]){
      case ' ':{
        if(!stillspace){
          vstr->push_back(tmp);
          tmp = "";
        }
        
        stillspace = true;
        break;
      }

      case '\0':{
        vstr->push_back(tmp);
        doloop = false;
        break;
      }

      default:{
        tmp += str[str_i];
        stillspace = false;
        break;
      }
    }

    str_i++;
  }
}

void domain(vector<string> *str){
    bool doloop = true;
    while(doloop){
      string newpass = promptPass(str);
      cout << STR_PROMPT << endl;

      bool doloop1 = true;
      while(doloop1){
        char c = _getch();
        switch(c){
          case 'r':{
            doloop1 = false;
            break;
          }

          case 'q':{
            doloop = doloop1 = false;
            break;
          }

          case 'c':{
            copyToClipboard(newpass);
            cout << "Password copied to clipboard." << endl;
          }
        }
      }
    }
}

#if defined(_WIN32)
  #include <conio.h>
  #include <windows.h>

  void copyToClipboard(string &str){
    HGLOBAL hg;
    if(!OpenClipboard(NULL)){
      cerr << "Clipboard cannot be opened." << endl;
      throw;
    }

    EmptyClipboard();
    hg = GlobalAlloc(GMEM_MOVEABLE, str.length()+1 * sizeof(TCHAR));
    if(hg == NULL){
      cerr << "Cannot be copied to clipboard." << endl;
      CloseClipboard();
      throw;
    }

    char* strptr = (char*)GlobalLock(hg);
    str.copy(strptr, str.length());
    strptr[str.length()] = '\0';
    GlobalUnlock(hg);

    SetClipboardData(CF_TEXT, hg);
    CloseClipboard();
  }

  int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow){
    vector<string> str;
    splitarg(&str, pCmdLine);
    domain(&str);
    return 0;
  }

#elif defined(__linux__)
  #define filename_tmp ".passtmp.tmp"
  #define CLIP_CMD "xclip -sel clip"

  #if defined(__TERMUX__)
    #define CLIP_CMD "termux-clipboard-set"
  #endif

  #include <termios.h>
  static struct termios old, current;

  int _getch(){
    tcgetattr(0, &old);
    current = old;
    current.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(0, TCSANOW, &current);
    int key = getchar();
    tcsetattr(0, TCSANOW, &old);
    return key;
  }

  void initTmpFile(){
    ofstream ofs;
    ofs.open(filename_tmp);
    ofs.close();
  }

  void deleteTmpFile(){
    string command = string("rm ") + filename_tmp;
    system(command.c_str());
  }

  void copyToClipboard(string &str){
    ofstream ofs;
    ofs.open(filename_tmp, ofs.trunc);
    ofs.write(str.c_str(), str.size());
    ofs.close();

    string command = string("cat ") + filename_tmp + " | " + CLIP_CMD;
    system(command.c_str());
  }

  int main(int argi, char* argcs[]){
    initTmpFile();
    vector<string> strv;
    ArrayToVector<char*, string>(argcs, argi, &strv);
    domain(&strv);
    deleteTmpFile();
  }

#endif
