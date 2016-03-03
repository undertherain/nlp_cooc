#!/usr/bin/env python3
from timeit import default_timer as timer
import sys
import glob
import os
import re
import fnmatch
import math
import numpy as np
from vocabulary import Vocabulary
import collections
from scipy import sparse
from scipy.sparse import * #dok_matrix
from swig.array_of_trees import ArrayOfTrees
def _my_get_index_dtype(*a, **kw):
	return np.int64
sparse.sputils.get_index_dtype = _my_get_index_dtype
sparse.compressed.get_index_dtype = _my_get_index_dtype
sparse.csr.get_index_dtype = _my_get_index_dtype
sparse.csr_matrix.get_index_dtype = _my_get_index_dtype
sparse.bsr.get_index_dtype = _my_get_index_dtype

argv = sys.argv
if len(argv) < 3:
	print ("usage: input_dir output_dir")
	exit()

name_dir_in = argv[1]
name_dir_out = argv[2]

vocab=Vocabulary()
vocab.read_from_precomputed(name_dir_out)

size_window=2
d = collections.deque(maxlen=size_window)
for i in range(size_window):
	d.append(-1)

#matrix=dok_matrix((vocab.cnt_words, vocab.cnt_words), dtype=np.int64)
#matrix=lil_matrix((vocab.cnt_words, vocab.cnt_words), dtype=np.int64)
#matrix=dok_matrix((vocab.cnt_words, vocab.cnt_words), dtype=np.int64)

m=ArrayOfTrees(vocab.cnt_words)


def get_worker_id(id_word):
	cnt_words_total=vocab.cnt_words

def accumulate(id1,id2):
	#decide which worker accumulates
	#matrix[id1,id2]+=1
	m.accumulate(id1,id2)
	pass

def process_word(word):
	id_word=vocab.get_id(word)
	if word in {".","!","?","…"}:
		if True: # options.obey_sentence_bounds
			id_word=-1
			for i in range(size_window):
				d.append(-1)
	else:
		if id_word<0:
			return
	#print("word : '{}'".format(word))
	d.append(id_word)
	for i in range(size_window-1):
		if d[-1]==-1 or d[i]==-1:
			continue
		#print("accing",d[-1],d[i])
		#print("accing",d[i],d[-1])
		accumulate(d[-1],d[i])
		accumulate(d[i],d[-1])

def process_file(name):
	print ("processing "+name)
	f=open(name, errors="replace")
	for line in f:
		s = line.strip().lower()
		re_pattern=r"[\w\-']+|[.,!?…]"
		tokens=re.findall(re_pattern, s)
		for token in tokens:
			process_word(token)

start = timer()

for root, dir, files in os.walk(name_dir_in,followlinks=True):
        for items in fnmatch.filter(files, "*"):
        	process_file(os.path.join(root,items))

end = timer()
print("done reading corpus, took",end - start)      
start=end
print("-----dumping data------")
m.dump_csr(name_dir_out,vocab.l_frequencies);
exit()
print("-----converting to COO------")
matrix_coo=matrix.tocoo()
end = timer()
print(" took",end - start)      
start=end
#matrix_coo.sort_indices()
#print(matrix)
print("-----converting to csr------")
matrix_csr=matrix_coo.tocsr()
end = timer()
print(" took",end - start)      
start=end
print("-----converting back to coo------")
matrix_coo=matrix_csr.tocoo()
end = timer()
print(" took",end - start)      
start=end
#print(matrix_coo)
cnt_words_processed=vocab.l_frequencies.sum()
#print(matrix_csr)
debug=False
if debug:
	f_out=open("bigrams_list","w")
	for i in zip(matrix_coo.row,matrix_coo.col):
		row=i[0]
		col=i[1]
		freq=matrix[i]
		v=math.log2((freq*cnt_words_processed)/(vocab.l_frequencies[col]*vocab.l_frequencies[col]))
		f_out.write("{}\t{}\t{}\t{:0.5f}\n".format(vocab.get_word_by_id(row),vocab.get_word_by_id(col),freq,v))
	f_out.close()
#print(matrix_csr.indices.dtype)

data_pmi=np.zeros(matrix_csr.data.shape[0],dtype=np.float32)
ind=0
for i in zip(matrix_coo.row,matrix_coo.col):
	row=i[0]
	col=i[1]
	freq=matrix[i]
	v=math.log2((freq*cnt_words_processed)/(vocab.l_frequencies[col]*vocab.l_frequencies[col]))
	data_pmi[ind]=v
	ind+=1
#f_out=open("bigrams.data.bin","wb")
#f_out.close()
#print(matrix_csr.indices.dtype)


matrix_csr.indices.tofile(os.path.join(name_dir_out,"bigrams.col_ind.bin"))
matrix_csr.indptr.tofile(os.path.join(name_dir_out,"bigrams.row_ptr.bin"))
data_pmi.tofile(os.path.join(name_dir_out,"bigrams.data.bin"))

