import os
import config
import time

################ Experiment 7 #########################
# This experiment runs with a data set of 500 taxa and compares fast lookup, savemem and extern. 
#
def experiment7():
	##### Setting the logging configuration to have one file for each Experiment #####
	f = open("../logging.conf", "w")
	f.write(config.logging_configuration)
	t = time.localtime()
	timestamp = time.strftime('%b-%d-%Y_%H%M', t)
	filename = "Exp07_"+ timestamp +".log"
	f.write("""FILENAME             =  "logs/exp7/""" + filename + """" """)
	f.close()

	##### Setting the parameters #####
	referenceTree = "data/reference_400.tre"
	evaluationTree = "data/geneTrees_400.tre"
	internalMemory = "35"
	f = open("logs/exp7/" + filename, "w")
	f.write("# This experiment runs with data set of 500 taxa and compares fast lookup, savemen and extern.\n")
	f.write("ReferenceTree: "+referenceTree+"\n") 	
	f.write("EvaluationTree: "+evaluationTree+"\n")

	##### Executing the experiment #####
	f.write("[run_thread] [16 threads]\n")
	f.write("[run_internal] [1 internal]\n")
	f.close()
	os.system("../../QuartetScores/bin/./QuartetScores -r "+referenceTree+" -e "+evaluationTree+" -o logs/exp7/exp7_t16_fast_"+timestamp+".tre -t 16")

	f = open("logs/exp7/" + filename, "a")
	f.write("[run_thread] [16 threads]\n")
	f.write("[run_internal] [2 saveMem]\n")
	f.close()
	os.system("../../QuartetScores/bin/./QuartetScores -r "+referenceTree+" -e "+evaluationTree+" -o logs/exp7/exp7_t16_savemem_"+timestamp+".tre -t 16 -s")

	f = open("logs/exp7/" + filename, "a")
	f.write("[run_thread] [16 threads]\n")
	f.write("[run_internal] [3 extern]\n")
	f.close()
	os.system("../bin/./QuartetScores -r "+referenceTree+" -e "+evaluationTree+" -o logs/exp7/exp7_t16_ext_"+timestamp+".tre -t 16 -i "+internalMemory)


