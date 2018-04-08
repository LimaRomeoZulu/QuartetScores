import os
import config
import time

################ Experiment 2 #########################
#This experiment compares the time of the internal algorithm with the external algorithm for 300 taxa
#300 taxa will result in an non-optimized lookup table of 16GB
#the experiment is conducted with an available RAM space of 64GB
#
def experiment2():
	##### Setting the logging configuration to have one file for each Experiment #####
	#Set logging of external algortihm #
        f = open("../logging.conf", "w")
        f.write(config.logging_configuration)
        t = time.localtime()
        timestamp = time.strftime('%b-%d-%Y_%H%M', t)
        filename = "Exp02_"+ timestamp +".log"
        f.write("""FILENAME             =  "logs/exp2/""" + filename + """" """)
        f.close() 
	#Set logging of internal algorithm #
        f = open("../../QuartetScores/logging.conf", "w")
        f.write(config.logging_configuration)
        t = time.localtime()
        timestamp = time.strftime('%b-%d-%Y_%H%M', t)
        filename = "Exp02_"+ timestamp +".log"
        f.write("""FILENAME             =  "logs/exp2/""" + filename + """" """)
        f.close() 

	##### Setting the parameters #####
	referenceTree = "data/reference_300.tre"
	evaluationTree = "data/geneTrees_300.tre"
	internalMemory = "33"
	f = open("logs/exp2/" + filename, "w")
	f.write("#This experiment compares the time of the internal algorithm with the external algorithm for 300 taxa\n"
		"300 taxa will result in an non-optimized lookup table of 16GB\n"
		"the experiment is conducted with an available RAM space of 64GB\n")
	f.write("ReferenceTree: "+referenceTree+"\n") 	
	f.write("EvaluationTree: "+evaluationTree+"\n")

	##### Executing the experiment #####
	f.write("[run_thread] [8 threads]\n")
	f.write("[run_internal] [1 fastLookup]\n")
	f.close()
	os.system("../../QuartetScores/bin/./QuartetScores -r "+referenceTree+" -e "+evaluationTree+" -o logs/exp2/exp2_t8_int_"+timestamp+".tre -t 8")

	f = open("logs/exp2/" + filename, "a")
	f.write("[run_thread] [8 threads]\n")
	f.write("[run_internal] [2 saveMem]\n")
	f.close()
	os.system("../../QuartetScores/bin/./QuartetScores -r "+referenceTree+" -e "+evaluationTree+" -o logs/exp2/exp2_t8_int_"+timestamp+".tre -t 8 -s")

	f = open("logs/exp2/" + filename, "a")
	f.write("[run_thread] [8 threads]\n")
	f.write("[run_memory] ["+ str(2**int(internalMemory)) +" bytes]\n")
	f.write("[run_internal] [0 external]\n")
	f.close()
	os.system("../bin/./QuartetScores -r "+referenceTree+" -e "+evaluationTree+" -o logs/exp2/exp2_t8_ext_"+timestamp+".tre -t 8 -i "+internalMemory)
	
	os.system("diff logs/exp2/exp2_t8_int_"+timestamp+".tre logs/exp2/exp2_t8_ext_"+timestamp+".tre")
