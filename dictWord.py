import sys
import string
import time

with open("/usr/share/dict/words") as word_file:
    english_words = set(word.strip().lower() for word in word_file)

def is_english_word(word):
    return word.lower() in english_words


def findShortPath (src, dest):
    if is_english_word(src) and is_english_word(dest):
        bfsList = [src]
        pathDict = {}
        pathDict[src] = ""
        doBFS(bfsList, pathDict, dest, src)
    else:
        print ("Both words should be valid english words")

def foundWord (pathDict, dest, src):
    pathList = []
    pathList.insert(0, dest)
    #print (dest)
    #time.sleep(1)
    while True:
        prevWord = pathDict[dest]
        pathList.insert(0, prevWord)
        if prevWord == src:
            strin = " ".join(pathList)
            print (strin)
            sys.exit(0)
        dest = prevWord
        

def doBFS (l, d, dest, src):
    while len(l) != 0:
        word = l.pop()
        if word == dest:
            foundWord(d, dest, src)
            
        potWord = []
        for idx in range(len(word)):
            for c in string.lowercase[:26]:
                if not c == word[idx]:
                    potWord.append(word[:idx] + c + word[idx+1:])
                    
        for nword in potWord:
            if is_english_word(nword) and not d.has_key(nword):
                #print ("Adding Word %s from %s" % (nword, word))
                #time.sleep(0.5)
                d[nword] = word
                l.insert(0, nword)
    print ("Could not find a valid path between words")

def getWordsFromUser():
    srcWord = raw_input()
    desWord = raw_input()
    findShortPath(srcWord.lower(), desWord.lower())

if __name__ == "__main__":
    getWordsFromUser()
