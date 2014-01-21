import sys
import os
import importlib
import traceback
import inspect
import time
import multiprocessing

class TestFailException(Exception):
    """Custom Exception class for passing error messages"""
    def __init__ (self, value):
        self.val = value
    def __str__ (self):
        return repr(self.val)

def impModule (modName, methName, numArgs):
    """Basic Check of importing module and checking syntax error if any
       Also checks for number of arguments in method against given numArgs
       Raises TestFailException if any of these error occurs else returns method object
    """
    try:
        #Try to import module, handle ImportError/Syntax Error if failed
        mod = importlib.import_module(modName)
        #Try to getattr method, handle AttributeError
        meth = getattr(mod, methName)
        #Find out number of arguments the method takes and compare it to numArgs
        argC = len(inspect.getargspec(meth).args)
        if not argC == numArgs:
            raise TestFailException("Expecting %d args in %s but found %d" % (numArgs, methName, argC))
        return meth
    except ImportError:
        raise TestFailException("Can't find module %s" % (modName))
    except SyntaxError:
        raise TestFailException("Syntax Error found in module %s" % (modName))
    except AttributeError:
        raise TestFailException("Method %s not found in module %s" % (methName, modName))

def runWrapper(methName, inpArg, q):
    #This is spanned as a separate thread so that we can monitor timeout
    #Sends back result or stacktrace in case the program crashes
    #The first bool object identifies if the test ran successfully or failed
    #Time taken for the test to run is also collected and sent in case of a successful run
    try:
        t0 = time.time()
        res = methName(inpArg)
        t1 = time.time()
        sendBack = (True, res, (t1-t0))
        q.put(sendBack)
    except Exception:
        sendBack = (False, traceback.format_exc(3))
        q.put(sendBack)
        
    
def runTest (modName, methName, testMap, timeOut, userDebugPrint = False):
    """ Takes in module Name, method Name to be tested;
        testMap contains combination of inputs and outputs as key value pairs (can be empty)
        timeOut in seconds -- applicable to each test case run
        userDebugPrint -- suppresses user prints to stdout if set to True (default is False)
        Prints out a summary of all test case runs;
        No test cases are run if there is syntax error, mismatch in number of args or missing module
    """
    try:
        numArgs = 1 #Our test framework currently handles methods with only one arg 
        origStdOut = sys.stdout
        #Basic module load, syntax, number of args checking
        finMeth = impModule(modName, methName, numArgs)
        mismatchCase = {}
        successCase = {}
        crashCase = {}
        timeoutCase = {}
        if not userDebugPrint:
            #Route the stdout to /dev/null if userDebug needs to be suppressed
            sys.stdout = open (os.devnull, 'w')
        for inpArg, expOp in testMap.iteritems():
                que = multiprocessing.Queue()
                inpTuple = (finMeth, inpArg, que)
                #Spawn a new thread for a test case and get return data via Queue
                p = multiprocessing.Process(target=runWrapper, args=inpTuple)
                p.start()
                p.join(timeOut)
                if p.is_alive():
                    #Test is still running after timeout seconds elapsed, so kill it
                    #and record a message in timeoutCase
                    p.terminate()
                    p.join()
                    timeoutCase[inpArg] = (expOp, timeOut)
                else:
                    #Time completed without timeout, so if test ran successfully
                    retValue = que.get()
                    succ = retValue[0]
                    if succ:
                        #Test completed without crashing, so check result
                        res = retValue[1]
                        runTime = retValue[2]
                        if not res == expOp:
                            #Test didn't pass, record the actual result returned
                            mismatchCase[inpArg] = (expOp, res)
                        else:
                            #Test ran fine! Record the time taken for the test to run
                            successCase[inpArg] = (expOp, "%.3f" %(runTime))
                    else:
                        #Test crashed so get stack trace
                        stackTrace = retValue[1]
                        crashCase[inpArg] = (expOp, stackTrace)
        #Restore the stdout before printing summary!                       
        sys.stdout = origStdOut
        printSummary(testMap, crashCase, mismatchCase, timeoutCase, successCase)
    except TestFailException as exc:
        #Restore stdout before printing exception message
        sys.stdout = origStdOut 
        print (exc)
        print ("No tests were run!")
        
def printSummary (testMap, crashCase, mismatchCase, timeoutCase, successCase):
    #Walk through all possible failure/success cases and print out success rate
    for inp, op in crashCase.iteritems():
        print ("Test Crashed for Input : %s StackTrace = %s" % (inp, op[1]))
    for inp, op in mismatchCase.iteritems():
        print ("Test Failed for Input : %s Got : %s Expected : %s" % (inp, op[1], op[0]))
    for inp, op in timeoutCase.iteritems():
        print ("Test TimeOut for Input : %s TimeOutLimit: %d" % (inp, op[1]))
    for inp, op in successCase.iteritems():
        print ("Test Success for Input : %s Output %s in Time %s" % (inp, op[0], op[1]))
    numTests = len(testMap.keys())
    passTests = len(successCase.keys())
    if numTests > 0:
        print ("Overall : Pass %d Out of %d : Success Rate = %.2f" % (passTests, numTests, ((passTests*100)/numTests)))
    else:
        print ("No tests given but module seems to be syntax error free")

def getUserInput():
    modName = "abbreviate_name" #raw_input()
    methName = "abbreviate_name" #raw_input()
    #Supply Input:Output map
    #testMap = {}
    testMap = {"Gokul Guna" : "G. Guna", "Haran Raj Kumar" : "H. R. Kumar"}
    #methName = "mult"
    #testMap = {(3,2) : 6, (4,5) : 20}
    timeOut = 2 #Timeout in seconds for each test
    userDebugPrint = False #Switch to enable user debug printing
    runTest(modName, methName, testMap, timeOut, userDebugPrint)


if __name__ == "__main__":
    #Manually supply input if not imported as a module
    getUserInput()
    
    
    
