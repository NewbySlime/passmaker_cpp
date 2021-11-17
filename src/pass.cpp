#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <cmath>

using namespace std;

#define PASSLOCATION "pass"
#define DEFAULTPASS {(char)1}
#define ENCRPTEDSTRMAX 15

#define CHAR_TOUSE "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"

string ctousestr = CHAR_TOUSE;

void splitarg(vector<string> *args, char* str);
void copyToClipboard(string str);

template<typename t> void fillBuffer(t* buffer, int len, t what){
  for(int str_i = 0; str_i < len; str_i++)
    buffer[str_i] = what;
}

//strlength is a length of the password
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
  ofs.open(PASSLOCATION, ios::openmode::_S_trunc);
  string e_pass = encryptString(pass, string(DEFAULTPASS), pass.length());

  ofs.write(e_pass.c_str(), e_pass.length());
  ofs.close();
}

string getEPass(){
  string res = "";
  ifstream ifs;
  ifs.open(PASSLOCATION, ios::openmode::_S_bin);
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
        
        //untuk masukkan/ganti password yang sebelumnya dipakai
        case 'm':{
          string str = "";
          for(int str_i = 2; str_i < args->operator[](arg_i).length(); str_i++)
            str += args->operator[](arg_i)[str_i];

          storePass(str);
        }
      }
    }
  }

  cout << "Masukkan domain: " << flush;
  string anchorstr = "";
  char c = 0;
  while((c = getchar()) != '\n')
    anchorstr += c;
  
  string epass = getEPass();
  return encryptString(anchorstr, epass, ENCRPTEDSTRMAX);
}

#if defined(_WIN32)
  #include <windows.h>

  void splitarg(vector<string> *vstr, LPSTR str){
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

  void copyToClipboard(string str){
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
    string newpass = promptPass(&str);
    copyToClipboard(newpass);
    cout << "Copied to clipboard." << endl;
    return 0;
  }

#elif defined(__linux__)

#endif