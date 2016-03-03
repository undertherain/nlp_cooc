import numpy as np
import sys
import os

sys.path.append("swig")

from ternary_tree import TernaryTree

class Vocabulary(TernaryTree):
	def __init__(self):
		super().__init__()
		pass
		#tree=TernaryTree()
	def read_from_precomputed(self,path):
		self.l_frequencies = np.fromfile(open(os.path.join(path,"freq_per_id")),dtype=np.int64)
		self.cnt_words = self.l_frequencies.shape[0]
		self.lst_words=[""]*self.cnt_words
		f=open(os.path.join(path,"ids"), encoding='utf-8', errors='replace')
		lines=f.readlines()
		for line in lines:
			tokens=line.split("\t")
			self.set_id(tokens[0],int(tokens[-1]))
			self.lst_words[np.int64(tokens[-1])]=tokens[0]
		f.close()
	def get_word_by_id(self,i):
		return(self.lst_words[i])

if __name__ == "__main__":	
	v=Vocabulary()
	print(v.get_id("три"))
