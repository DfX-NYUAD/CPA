set(cuda_cpa_source_files
	pcpa-host.cpp
	pcpa-kernel.cu
	pcpa-reduction.cu)

cuda_add_library(cuda_cpa ${cuda_cpa_source_files})

