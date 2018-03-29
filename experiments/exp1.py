import os
import config
import time

################ Experiment 1 #########################
# This experiment runs with a small data set and measures the overall time. 
# It is executed multiple times with an ascending amount of used threads.
#
def experiment1():
	##### Setting the logging configuration to have one file for each Experiment #####
	f = open("../logging.conf", "w")
	f.write(config.logging_configuration)
	t = time.localtime()
	timestamp = time.strftime('%b-%d-%Y_%H%M', t)
	filename = "Exp01_"+ timestamp +".log"
	f.write("""FILENAME             =  "logs/exp1/""" + filename + """" """)
	f.close()

	##### Setting the parameters #####
	referenceTree = "data/A1.RAxML.ASTRAL.tre"
	evaluationTree = "data/A1_evaluation.tre"
	internalMemory = "33"
	f = open("logs/exp1/" + filename, "w")
	f.write("# This experiment runs with a small data set and measures the overall time.\n" 
		"# It is executed multiple times with an ascending amount of used threads.\n")
	f.write("ReferenceTree: "+referenceTree+"\n") 	
	f.write("EvaluationTree: "+evaluationTree+"\n")

	##### Executing the experiment #####
	f.write("[run_thread] [1 threads]\n")
	f.write("[run_memory] ["+ str(2**int(internalMemory)) +" bytes]\n")
	f.close()
	os.system("../bin_directCalc/./QuartetScores -r "+referenceTree+" -e "+evaluationTree+" -o logs/exp1/exp1_t1_"+timestamp+".tre -t 1 -i "+internalMemory)
	f = open("logs/exp1/" + filename, "a")
	f.write("[run_thread] [2 threads]\n")
	f.close()
	os.system("../bin_directCalc/./QuartetScores -r "+referenceTree+" -e "+evaluationTree+" -o logs/exp1/exp1_t2_"+timestamp+".tre -t 2 -i "+internalMemory)
	f = open("logs/exp1/" + filename, "a")
	f.write("[run_thread] [4 threads]\n")
	f.close()
	os.system("../bin_directCalc/./QuartetScores -r "+referenceTree+" -e "+evaluationTree+" -o logs/exp1/exp1_t4_"+timestamp+".tre -t 4 -i "+internalMemory)
	f = open("logs/exp1/" + filename, "a")
	f.write("[run_thread] [8 threads]\n")
	f.close()
	os.system("../bin_directCalc/./QuartetScores -r "+referenceTree+" -e "+evaluationTree+" -o logs/exp1/exp1_t8_"+timestamp+".tre -t 8 -i "+internalMemory)
