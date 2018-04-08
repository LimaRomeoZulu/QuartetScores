import os
import config
import time

################ Experiment 6 #########################
#This experiment created a time profil for a reference tree with 300 taxa
#it is necessarz to uncomment code in QuartetCounterLookup.hpp
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
	f.write("#this experiment creates a time profile based on a reference tree with 300 taxa\n")
	f.write("ReferenceTree: "+referenceTree+"\n") 	
	f.write("EvaluationTree: "+evaluationTree+"\n")

	##### Executing the experiment #####
	f = open("logs/exp6/" + filename, "a")
	f.write("[run_thread] [8 threads]\n")
	f.write("[run_memory] ["+ str(2**int(internalMemory)) +" bytes]\n")
	f.write("[run_internal] [0 external]\n")
	f.close()
	os.system("../bin/./QuartetScores -r "+referenceTree+" -e "+evaluationTree+" -o logs/exp2/exp2_t8_ext_"+timestamp+".tre -t 8 -i "+internalMemory)
	
