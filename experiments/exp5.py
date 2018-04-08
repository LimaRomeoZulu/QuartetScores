import os
import config
import time

################ Experiment 5 #########################
#This experiment compares the running time of a version parallalized over the evaluation trees with a version prallelized over counting the quartets in each evaluation tree.  
#
def experiment5():
	##### Setting the logging configuration to have one file for each Experiment #####
	#Set logging of external algortihm #
        f = open("../logging.conf", "w")
        f.write(config.logging_configuration)
        t = time.localtime()
        timestamp = time.strftime('%b-%d-%Y_%H%M', t)
        filename = "Exp05_"+ timestamp +".log"
        f.write("""FILENAME             =  "logs/exp5/""" + filename + """" """)
        f.close() 

	##### Setting the parameters #####
	referenceTree = "data/A1.RAxML.ASTRAL.tre"
	evaluationTree = "data/A1_evaluation.tre"
	internalMemory = "33"
	f = open("logs/exp5/" + filename, "w")
	f.write("This experiment compares the running time of a version parallalized over the evaluation trees with"
		"a version prallelized over counting the quartets in each evaluation tree.")
	f.write("ReferenceTree: "+referenceTree+"\n") 	
	f.write("EvaluationTree: "+evaluationTree+"\n")

	##### Executing the experiment #####
	f.write("[run_thread] [8 threads]\n")
	f.write("[run_memory] ["+ str(2**int(internalMemory)) +" bytes]\n")
	f.write("[run_dataset] [0 paraCount]\n")
	f.close()
	for i in range(0,10):
		f = open("logs/exp5/" + filename, "a")
		f.write("[run_loop] ["+ str(i) +" iteration]")
		f.close()
		os.system("../bin/./QuartetScores -r "+referenceTree+" -e "+evaluationTree+" -o logs/exp4/exp4_t8_gen_"+timestamp+"_"+ str(i) +".tre -t 8 -i "+internalMemory)

	f = open("logs/exp5/" + filename, "a")
	f.write("ReferenceTree: "+referenceTree+"\n") 	
	f.write("EvaluationTree: "+evaluationTree+"\n")

	f.write("[run_thread] [8 threads]\n")
	f.write("[run_memory] ["+ str(2**int(internalMemory)) +" bytes]\n")
	f.write("[run_dataset] [1 paraTrees]\n")
	f.close()
	for i in range(0,10):
		f = open("logs/exp5/" + filename, "a")
		f.write("[run_loop] ["+ str(i) +" iteration]")
		f.close()
		os.system("../binParaTrees/./QuartetScores -r "+referenceTree+" -e "+evaluationTree+" -o logs/exp4/exp4_t8_real_"+timestamp+"_"+str(i)+".tre -t 8 -i "+internalMemory)
