import exp1
import exp2
import exp3
import exp4
import exp5
import exp6
import expList

################ Executing experiments  #########################
mapping = {
	1 : exp1.experiment1,
	2 : exp2.experiment2,
	3 : exp3.experiment3,
	4 : exp4.experiment4, 
	5 : exp5.experiment5,
	6 : exp6.experiment6
}


for exp in expList.exp_list:
	func = mapping[exp]
	func()
