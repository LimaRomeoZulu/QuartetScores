import os
import config
import time

################ Experiment 4 #########################
#This experiment compares the processing time of a generated tree with 200 taxa to a real tree with 200 taxa. The basis for the generation was the dataset A7 
#
def experiment4():
	##### Setting the logging configuration to have one file for each Experiment #####
	#Set logging of external algortihm #
        f = open("../logging.conf", "w")
        f.write(config.logging_configuration)
        t = time.localtime()
        timestamp = time.strftime('%b-%d-%Y_%H%M', t)
        filename = "Exp04_"+ timestamp +".log"
        f.write("""FILENAME             =  "logs/exp4/""" + filename + """" """)
        f.close() 

	##### Setting the parameters #####
	referenceTree = "data/reference_200.tre"
	evaluationTree = "data/geneTrees_200.tre"
	internalMemory = "33"
	f = open("logs/exp4/" + filename, "w")
	f.write("This experiment compares the processing time of a generated tree with 200 taxa"
		"to a real tree with 200 taxa. The basis for the generation was the dataset A7")
	f.write("ReferenceTree: "+referenceTree+"\n") 	
	f.write("EvaluationTree: "+evaluationTree+"\n")

	##### Executing the experiment #####
	f.write("[run_thread] [8 threads]\n")
	f.write("[run_memory] ["+ str(2**int(internalMemory)) +" bytes]\n")
	f.write("[run_dataset] [0 generated]\n")
	f.close()
	for i in range(0,10):
		f = open("logs/exp4/" + filename, "a")
		f.write("[run_loop] ["+ str(i) +" iteration]")
		f.close()
		os.system("../bin/./QuartetScores -r "+referenceTree+" -e "+evaluationTree+" -o logs/exp4/exp4_t8_gen_"+timestamp+"_"+ str(i) +".tre -t 8 -i "+internalMemory)

	referenceTree = "data/A7.RAxML.ASTRAL.tre"
	evaluationTree = "data/A7_evaluation.tre"

	f = open("logs/exp4/" + filename, "a")
	f.write("ReferenceTree: "+referenceTree+"\n") 	
	f.write("EvaluationTree: "+evaluationTree+"\n")

	f.write("[run_thread] [8 threads]\n")
	f.write("[run_memory] ["+ str(2**int(internalMemory)) +" bytes]\n")
	f.write("[run_dataset] [1 real]\n")
	f.close()
	for i in range(0,10):
		f = open("logs/exp4/" + filename, "a")
		f.write("[run_loop] ["+ str(i) +" iteration]")
		f.close()
		os.system("../bin/./QuartetScores -r "+referenceTree+" -e "+evaluationTree+" -o logs/exp4/exp4_t8_real_"+timestamp+"_"+str(i)+".tre -t 8 -i "+internalMemory)
