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
	f.write("""FILENAME             =  "logs/Exp1_"""+ timestamp +""".log" """)
	f.close() 	

	##### Executing the experiment #####
	os.system("../bin/./QuartetScores -r data/A1.RAxML.ASTRAL.tre -e data/A1_evaluation.tre -o logs/exp1_t1.tre -t 1")


################ Executing experiments  #########################
mapping = {
	1 : experiment1 
}


for exp in config.exp_list:
	func = mapping[exp]
	func()
