import os
import config
import time

################ Experiment 5 #########################
#This experiment compares the time of the internal algorithm with the external algorithm for 450 taxa
#450 taxa will result in an non-optimized lookup table of 82GB
#the experiment is conducted with an available RAM space of 64GB
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
	#Set logging of internal algorithm #
        f = open("../../QuartetScores/logging.conf", "w")
        f.write(config.logging_configuration)
        t = time.localtime()
        timestamp = time.strftime('%b-%d-%Y_%H%M', t)
        filename = "Exp05_"+ timestamp +".log"
        f.write("""FILENAME             =  "logs/exp5/""" + filename + """" """)
        f.close() 

	##### Setting the parameters #####
	referenceTree = "data/reference_450.tre"
	evaluationTree = "data/geneTrees_450.tre"
	internalMemory = "35"
	f = open("logs/exp5/" + filename, "w")
	f.write("#This experiment compares the time of the internal algorithm with the external algorithm for 450 taxa\n"
		"450 taxa will result in an non-optimized lookup table of 82GB\n"
		"the experiment is conducted with an available RAM space of 64GB\n")
	f.write("ReferenceTree: "+referenceTree+"\n") 	
	f.write("EvaluationTree: "+evaluationTree+"\n")

	##### Executing the experiment #####
	f.write("[run_thread] [16 threads]\n")
	f.write("[run_internal] [1 fastLookup]\n")
	f.close()
	os.system("../../QuartetScores/bin/./QuartetScores -r "+referenceTree+" -e "+evaluationTree+" -o logs/exp5/exp5_t16_fast_"+timestamp+".tre -t 16")

	f = open("logs/exp5/" + filename, "a")
	f.write("[run_thread] [16 threads]\n")
	f.write("[run_internal] [2 saveMem]\n")
	f.close()
	os.system("../../QuartetScores/bin/./QuartetScores -r "+referenceTree+" -e "+evaluationTree+" -o logs/exp5/exp5_t16_save_"+timestamp+".tre -t 16 -s")

	f = open("logs/exp5/" + filename, "a")
	f.write("[run_thread] [16 threads]\n")
	f.write("[run_memory] ["+ str(2**int(internalMemory)) +" bytes]\n")
	f.write("[run_internal] [0 external]\n")
	f.close()
	os.system("../bin_directCalc/./QuartetScores -r "+referenceTree+" -e "+evaluationTree+" -o logs/exp5/exp5_t16_ext_"+timestamp+".tre -t 16 -i "+internalMemory)
