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
from mpi4py import MPI
import h5py
import datetime

size_window=2
size_buffer=100
start = timer()
#---some tweaks for scipy

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

comm = MPI.COMM_WORLD
id_worker = comm.Get_rank()
cnt_workers = comm.Get_size()
cnt_mappers = 1
if cnt_workers>3:
	cnt_mappers=2
if cnt_workers>5:
	cnt_mappers=3
cnt_reducers = cnt_workers-cnt_mappers
id_reducer = id_worker-cnt_mappers
group = comm.Get_group()
ids_reducers=list(range(cnt_mappers,cnt_workers))
#print ("lst reducers",ids_reducers)
#group_reducers.Incl(ids_reducers)
group_reducers=MPI.Group.Incl(group,ids_reducers)
comm_reducers = comm.Create(group_reducers)



name_dir_in = argv[1]
name_dir_out = argv[2]
start = timer()

d = collections.deque(maxlen=size_window)
for i in range(size_window):
	d.append(-1)

buffers = [np.zeros((size_buffer,2),dtype=np.int64) for i in range(cnt_reducers)]
pos_bufers=[0 for i in range(cnt_reducers)]

def get_start_i(N,cnt_workers,id_worker):
    if N<cnt_workers: return min(N,id_worker)
    length_of_range=((N+1)//cnt_workers)
    start = length_of_range*id_worker
    if id_worker<N%cnt_workers:
        start+=id_worker
    else:
        start+=N%cnt_workers
    return start
def get_interval(N,cnt_workers,id_worker):
    return (get_start_i(N,cnt_workers,id_worker),get_start_i(N,cnt_workers,id_worker+1))

def get_worker_id(N,cnt_workers,v):
    if N<cnt_workers: return v
    length_of_range=((N+1)//cnt_workers)
    remainder = N%cnt_workers 
    if v<remainder*(length_of_range+1):
        return v//(length_of_range+1)
    else:
        return (v-remainder*(length_of_range+1)) // length_of_range + N%cnt_workers

def rndrobin_list(l,cnt_workers,id_worker):
    r=[l[id_worker+i*cnt_workers] for i in range((len(l)+cnt_workers-1)//cnt_workers) if id_worker+i*cnt_workers<len(l)]
    return r


def enqueue(id1,id2):
	
	id_dest=get_worker_id(cnt_words,cnt_reducers,id1)
	#print ("word",id1,"goes to",id_dest)
	pos_buf=pos_bufers[id_dest]
	buffers[id_dest][pos_buf,0]=id1
	buffers[id_dest][pos_buf,1]=id2
	pos_bufers[id_dest]+=1
	if pos_bufers[id_dest]==size_buffer:
		#print("buffer is full, sending to",id_dest)
		comm.Send(buffers[id_dest], dest=id_dest+cnt_mappers, tag=1)
		pos_bufers[id_dest]=0
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
		enqueue(d[-1],d[i])
		enqueue(d[i],d[-1])

def process_file(name):
	print ("m{} processing {}".format(id_worker,name))
	f=open(name, errors="replace")
	for line in f:
		s = line.strip().lower()
		re_pattern=r"[\w\-']+|[.,!?…]"
		tokens=re.findall(re_pattern, s)
		for token in tokens:
			process_word(token)

cnt_words=0
freqs = np.empty((cnt_words), dtype=np.int64)

#if id_worker==0:  #init 

	
if id_worker<cnt_mappers:  #mapping
	vocab=Vocabulary()
	vocab.read_from_precomputed(name_dir_out)
	print("loaded vocab")
	#broadcast number of words
	cnt_words=vocab.cnt_words
	freqs=vocab.l_frequencies
	cnt_words=comm.bcast(cnt_words, root=0)
	freqs = comm.bcast(freqs, root=0)
	print("I'm mapper, processing files in ",name_dir_in)
	lst_files=[]
	for root, dir, files in os.walk(name_dir_in,followlinks=True):
		for items in fnmatch.filter(files, "*"):
			lst_files.append(os.path.join(root,items))
#			process_file(os.path.join(root,items))
	lst_files=rndrobin_list(lst_files,cnt_mappers,id_worker)
	for f in lst_files:
		process_file(f)
	#send unfinished buffers
	for id_dest in range(cnt_reducers):
		for pos_buf in range(pos_bufers[id_dest],size_buffer):
			buffers[id_dest][pos_buf][0]=-1
		comm.Send(buffers[id_dest], dest=id_dest+cnt_mappers, tag=1)
	#print (lst_files)
else:	#this is reducer
	cnt_words=comm.bcast(cnt_words, root=0)
	freqs = comm.bcast(freqs, root=0)
	rstart,rend=get_interval(cnt_words,cnt_reducers,id_reducer)
	m=ArrayOfTrees(rend-rstart)
	print ("I'm reducer {} of {} running on {}, my ownership range is from {} to {}".format(id_reducer,cnt_reducers,MPI.Get_processor_name(),rstart,rend))
	buffer = np.empty((size_buffer,2), dtype=np.int64)
	cnt_mappers_finished=0
	has_work=True
	while has_work:
   		comm.Recv(buffer,tag=1)
   		#print ("r{} recvd".format(id_reducer))
   		for i in range(size_buffer):
   			#print ("\t in buf: ", buffer[i,0],buffer[i,1])
	   		if buffer[i,0]>=0:
   				m.accumulate(int(buffer[i][0]-rstart),int(buffer[i][1]))#todo mapping for aot
   			else:
   				cnt_mappers_finished+=1
   				#print("one mapper finished")
   				if cnt_mappers_finished>=cnt_mappers:
   					has_work=False
	
	row_ptr=np.zeros((rend-rstart+1),dtype=np.int64)
	m.get_row_ptr(row_ptr)
	#print("compiling the matrix")

	col_ind=np.zeros((row_ptr[-1]),dtype=np.int64)
	m.get_col_ind(col_ind)
	#print("r{} col_ind".format(id_reducer),col_ind)

	data_pmi=np.zeros((row_ptr[-1]),dtype=np.float32)
	m.get_data_PMI(data_pmi,freqs)
	#print(data_pmi)
	#print ("r{} row_ptr".format(id_reducer),row_ptr)

	my_offset=np.array(row_ptr[-1],dtype=np.int64)
	offset_scaned=np.zeros((1),dtype=np.int64)
	comm_reducers.Exscan(my_offset, offset_scaned)
	row_ptr+=offset_scaned[0]

	size_data=row_ptr[-1]
	size_data=comm_reducers.bcast(size_data, root=cnt_reducers-1)

#	print ("r{} row_ptr".format(id_reducer),row_ptr)
	#print ("r{} size data".format(id_reducer),size_data)
	f = h5py.File(os.path.join(name_dir_out,'cooccurrence_csr.h5p'), "w",driver='mpio', comm=comm_reducers)
	
	#get data size()

	dset_data = f.create_dataset("data", (size_data,), dtype='float32')
	dset_data[row_ptr[0]:row_ptr[-1]] = data_pmi
	f.flush()

	dset_col_ind = f.create_dataset("col_ind", (size_data,), dtype='int64')
	dset_col_ind[row_ptr[0]:row_ptr[-1]] = col_ind
	f.flush()

	dset_row_ptr = f.create_dataset("row_ptr", (cnt_words+1,), dtype='int64')
	if id_reducer<cnt_reducers-1:
		dset_row_ptr[rstart:rend] = row_ptr[:-1]
	else:
		dset_row_ptr[rstart:rend+1] = row_ptr[:]
	#print ("r{} ".format(id_reducer),rend-rstart,row_ptr.shape)
	
	f.flush()

	f.close()

provenance=""
end = timer()
if comm.rank==0:
	provenance += "cooccurrences collected on " + str(datetime.datetime.now())
	provenance += "\nwith " + argv[0]
	provenance += "\nsource corpus : " + name_dir_in + "\n"
	provenance+="windows size : {}\n".format(size_window)
	provenance+="obey sentence boundaries : yes"
	#provenance+=.obey_sentence_bounds?"yes":"no";
	provenance+="\nfrequency weightening : PMI\n"
	provenance+="took {:.2f}s\n".format(end - start)      
	with open (os.path.join(name_dir_out,"provenance.txt"), "r") as myfile:
		provenance = myfile.read() +"\n" +provenance
	text_file = open(os.path.join(name_dir_out,"provenance.txt"), "w")
	text_file.write(provenance)
	text_file.close()

