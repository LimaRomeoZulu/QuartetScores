import os
import config
import time

################ Experiment 7 #########################
# This experiments compares the scaling of the parts of the algotihm (counting, sorting, reducing and computing) with four different data sets of 100, 200, 300 and 400 taxa.
#
def experiment7():
	##### Setting the logging configuration to have one file for each Experiment #####
	#Set logging of external algortihm #
        f = open("../logging.conf", "w")
        f.write(config.logging_configuration)
        t = time.localtime()
        timestamp = time.strftime('%b-%d-%Y_%H%M', t)
        filename = "Exp07_"+ timestamp +".log"
        f.write("""FILENAME             =  "logs/exp7/""" + filename + """" """)
        f.close() 

	##### Setting the parameters #####
	internalMemory = "33"
	f = open("logs/exp7/" + filename, "w")
	f.write("This experiments compares the scaling of the parts of the algotihm (counting, sorting, reducing and computing) with four different data sets of 100, 200, 300 and 400 taxa.")
	f.close()

	for i in range(1,5,1):
		n = str(i * 100) #taxa of reference tree
		referenceTree = "data/reference_"+n+".tre"
		evaluationTree = "data/geneTrees_"+n+".tre"
		
		f = open("logs/exp7/" + filename, "a")
		f.write("ReferenceTree: "+referenceTree+"\n") 	
		f.write("EvaluationTree: "+evaluationTree+"\n")
		f.write("[run_thread] [16 threads]\n")
		f.write("[run_memory] ["+ str(2**int(internalMemory)) +" bytes]\n")
		f.write("[run_dataset] ["+n+" taxa]\n")
		f.close()
		
		os.system("/usr/bin/time -av ../bin_directCalc/./QuartetScores -r "+referenceTree+" -e "+evaluationTree+" -o logs/exp7/exp7_t16_"+n+"taxa_"+timestamp+"_"+ str(i) +".tre -t 16 -i "+internalMemory+" > logs/exp7/stderr/"+n+"taxa_"+filename+" 2>&1")
	

