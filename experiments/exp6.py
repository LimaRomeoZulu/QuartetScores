import os
import config
import time

################ Experiment 6 #########################
# This experiment runs with the external version and measures the time of reading the sorter and calculating the results.
# to enable the experiment 2 log lines in the code need to be activated in QuartetCounterLookup
#
def experiment6():
	##### Setting the logging configuration to have one file for each Experiment #####
	#Set logging of external algortihm #
        f = open("../logging.conf", "w")
        f.write(config.logging_configuration)
        t = time.localtime()
        timestamp = time.strftime('%b-%d-%Y_%H%M', t)
        filename = "Exp06_"+ timestamp +".log"
        f.write("""FILENAME             =  "logs/exp6/""" + filename + """" """)
        f.close() 

	##### Setting the parameters #####
	referenceTree = "data/A1.RAxML.ASTRAL.tre"
	evaluationTree = "data/A1_evaluation.tre"
	internalMemory = "33"
	f = open("logs/exp6/" + filename, "w")
	f.write("#This experiment runs with the external version and measures the time of reading the sorter and calculating the results.")
	f.write("ReferenceTree: "+referenceTree+"\n") 	
	f.write("EvaluationTree: "+evaluationTree+"\n")

	##### Executing the experiment #####
	f.write("[run_thread] [8 threads]\n")
	f.write("[run_memory] ["+ str(2**int(internalMemory)) +" bytes]\n")
	f.write("[run_internal] [0 external]\n")
	f.close()
	os.system("../bin_directCalc/./QuartetScores -r "+referenceTree+" -e "+evaluationTree+" -o logs/exp2/exp2_t8_ext_"+timestamp+".tre -t 8 -i "+internalMemory)
