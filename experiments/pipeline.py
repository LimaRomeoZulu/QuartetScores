import exp1
import exp2
import expList

################ Executing experiments  #########################
mapping = {
	1 : exp1.experiment1,
	2 : exp2.experiment2 
}


for exp in expList.exp_list:
	func = mapping[exp]
	func()
