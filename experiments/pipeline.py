import os
import config
import time

################ General Settings #########################

logging_configuration = """
* GLOBAL:
   FORMAT               =  '%msg; %datetime'
   ENABLED              =  true
   TO_FILE              =  true
   TO_STANDARD_OUTPUT   =  false
   SUBSECOND_PRECISION  =  1
"""



################ Experiment 1 #########################
# This experiment runs with a small data set and measures the overall time. 
# It is executed multiple times with an ascending amount of used threads.
#
def experiment1():
	##### Setting the logging configuration to have one file for each Experiment #####
	f = open("../logging.conf", "w")
	f.write(logging_configuration)
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
	f.write("[run_memory] ["+ str(2**33) +" bytes]\n")
	f.close()
	os.system("../bin/./QuartetScores -r "+referenceTree+" -e "+evaluationTree+" -o logs/exp1/exp1_t1_"+timestamp+".tre -t 1 -i "+internalMemory)
	f = open("logs/exp1/" + filename, "a")
	f.write("[run_thread] [2 threads]\n")
	f.close()
	os.system("../bin/./QuartetScores -r "+referenceTree+" -e "+evaluationTree+" -o logs/exp1/exp1_t2_"+timestamp+".tre -t 2 -i "+internalMemory)
	f = open("logs/exp1/" + filename, "a")
	f.write("[run_thread] [4 threads]\n")
	f.close()
	os.system("../bin/./QuartetScores -r "+referenceTree+" -e "+evaluationTree+" -o logs/exp1/exp1_t4_"+timestamp+".tre -t 4 -i "+internalMemory)
	f = open("logs/exp1/" + filename, "a")
	f.write("[run_thread] [8 threads]\n")
	f.close()
	os.system("../bin/./QuartetScores -r "+referenceTree+" -e "+evaluationTree+" -o logs/exp1/exp1_t8_"+timestamp+".tre -t 8 -i "+internalMemory)

################ Experiment 2 #########################
#This experiment compares the time of the internal algorithm with the external algorithm for 300 taxa
#300 taxa will result in an non-optimized lookup table of 16GB
#the experiment is conducted with an available RAM space of 64GB
#
def experiment2():
	##### Setting the logging configuration to have one file for each Experiment #####
	#Set logging of external algortihm #
        f = open("../logging.conf", "w")
        f.write(logging_configuration)
        t = time.localtime()
        timestamp = time.strftime('%b-%d-%Y_%H%M', t)
        filename = "Exp02_"+ timestamp +".log"
        f.write("""FILENAME             =  "logs/exp2/""" + filename + """" """)
        f.close() 
	#Set logging of internal algorithm #
        f = open("../../QuartetScores/logging.conf", "w")
        f.write(logging_configuration)
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
	f.write("[run_internal] [1 internal]\n")
	f.close()
	os.system("../../QuartetScores/bin/./QuartetScores -r "+referenceTree+" -e "+evaluationTree+" -o logs/exp2/exp8_t8_int_"+timestamp+".tre -t 8")

	f = open("logs/exp2/" + filename, "a")
	f.write("[run_thread] [8 threads]\n")
	f.write("[run_memory] ["+ str(2**33) +" bytes]\n")
	f.write("[run_external] [0 external]\n")
	f.close()
	os.system("../bin/./QuartetScores -r "+referenceTree+" -e "+evaluationTree+" -o logs/exp2/exp8_t8_ext_"+timestamp+".tre -t 8")

################ Executing experiments  #########################
mapping = {
	1 : experiment1,
	2 : experiment2 
}


for exp in config.exp_list:
	func = mapping[exp]
	func()
