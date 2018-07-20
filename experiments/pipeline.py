import exp1
import exp2
import exp3
import exp4
import exp5
import exp6
import exp7
import exp8
import exp9
import exp10
import exp11
import exp12
import expList

################ Executing experiments  #########################
mapping = {
	1 : exp1.experiment1,
	2 : exp2.experiment2,
	3 : exp3.experiment3,
	4 : exp4.experiment4, 
	5 : exp5.experiment5,
	6 : exp6.experiment6,
	7 : exp7.experiment7,
	8 : exp8.experiment8,
	9 : exp9.experiment9,
	10 : exp10.experiment10,
	11 : exp11.experiment11,
	12 : exp12.experiment12
}


for exp in expList.exp_list:
	func = mapping[exp]
	func()
