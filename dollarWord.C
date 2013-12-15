/*

While listening to NPR, I heard a story about a school teacher who
gave a problem to his students to find dollar words. That is, all 26 alphabets
get a value in cents (a = 1, b = 2, c = 3,.., z = 26). The task was to 
find all dictionary words whose characters sum upto a dollar (100 cents). 
Apparently a student whose dad was an engineer solved it for his son. And
the teacher used these words in his book. So I thought I will give the problem a try.
The dictionary used here is /usr/share/dict/american-english
*/

#include <iostream>
#include <unordered_map>
#include <string>
#include <fstream>
#include <cctype>
#include <algorithm>
#include <utility>
#include <memory>
#include <boost/algorithm/string.hpp>


const int NUML = 26;
const int DOLLAR = 100;

using namespace std;
using namespace boost::algorithm;

class Dict
{
  private:
  unordered_map<string, bool> dictMap;
  public:
  Dict () {
    ifstream file("/usr/share/dict/american-english");
    for (string word; getline(file, word); ) {
      bool alpha = true;
      for (const char &c : word) 
        alpha = alpha && isalpha(c);
      
      if (alpha) {
        to_lower(word);
        dictMap.insert(make_pair(word, true));
      } 
    }
  }
  
  bool containsWord(const string &w) const {
    return (dictMap.find(w) != dictMap.end());
  }
  
  unordered_map<string, bool> getDict () const {
    return dictMap;
  }
};

struct Node
{
  char c;
  bool isWord;
  shared_ptr<Node> chNode[NUML];
  Node () {
    isWord = false;
    for (int i = 0; i < NUML; i++) {
      chNode[i] = nullptr;
    }
  }
};


class Trie
{
  private:
  shared_ptr<Node> head;
  
  public:
  Trie (const Dict &dic) {
    const unordered_map<string, bool> &dMap = dic.getDict();
    head = make_shared<Node>();
    for (const pair<string, bool> &kV : dMap) {
      shared_ptr<Node> curr = head;
      for (const char &c: kV.first) {
        if ((c - 'a') >= NUML || ((c - 'a') < 0))
        {
          cout<<"Invalid char "<<c<<" found"<<endl;
          exit(1);
        }
        shared_ptr<Node> nN = curr->chNode[(c - 'a')];
        if (nN != nullptr) {
          curr = nN;
        } else {
          shared_ptr<Node> fN = make_shared<Node>();
          fN->c = c;
          curr->chNode[(c - 'a')] = fN;
          curr = fN;
        }
      }
      curr->isWord = true;
    }
  }
  
  bool wordStarts (const string &prefix) const {
    shared_ptr<Node> curr = head;
    for (const char &c : prefix) {
      curr = curr->chNode[(c - 'a')];
      if (curr == nullptr) 
        return false;
    }
    return true; 
  }

};


void printDollarWordHelp (string word, const Dict &d, const Trie &t, int rem) {
  if (rem == 0) {
    if (d.containsWord(word))
      cout<<word<<" is a dollarWord!"<<endl;
    return;
  }
  
  for (int i = 0; i < NUML; i++) {
    string nWord(word);
    nWord.push_back(('a' + i));
    if (t.wordStarts(nWord) && ((rem - i - 1) >= 0))
      printDollarWordHelp (nWord, d, t, rem - i - 1);
  }
}

void printDollarWord (const Dict &d, const Trie &t) {
   string cWord;
   printDollarWordHelp (cWord, d, t, DOLLAR);
}

int main()
{
  Dict d;
  Trie t(d);
  printDollarWord (d, t);
  return 0;
}
  
