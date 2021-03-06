export C_INCLUDE_PATH=/opt/intel/inteloneapi/compiler/2021.1.1/linux/lib/oclfpga/host/include/
export CPLUS_INCLUDE_PATH=/opt/intel/inteloneapi/compiler/2021.1.1/linux/lib/oclfpga/host/include/
cd apps/prk
make
./prk_swi ../../../dataset/ca-coauthors-dblp.mtx ../../../hw/opencl/prk/spmv_swi/bin/prk_swi.aocx  1 0 2 1
./prk_swi ../../../dataset/email-enron-large.mtx ../../../hw/opencl/prk/spmv_swi/bin/prk_swi.aocx  1 0 2 1
./prk_swi ../../../dataset/socfb-Georgetown15.mtx ../../../hw/opencl/prk/spmv_swi/bin/prk_swi.aocx  1 0 2 1
./prk_swi ../../../dataset/ca-hollywood-2009.mtx ../../../hw/opencl/prk/spmv_swi/bin/prk_swi.aocx  1 0 2 1
./prk_swi ../../../dataset/road-asia-osm.mtx ../../../hw/opencl/prk/spmv_swi/bin/prk_swi.aocx  1 0 2 1
./prk_swi ../../../dataset/socfb-uci-uni.mtx ../../../hw/opencl/prk/spmv_swi/bin/prk_swi.aocx  1 0 2 1
./prk_swi ../../../dataset/cit-DBLP.edges ../../../hw/opencl/prk/spmv_swi/bin/prk_swi.aocx  1 0 2 1
./prk_swi ../../../dataset/road-roadNet-CA.mtx ../../../hw/opencl/prk/spmv_swi/bin/prk_swi.aocx  1 0 2 1
./prk_swi ../../../dataset/web-google.mtx ../../../hw/opencl/prk/spmv_swi/bin/prk_swi.aocx  1 0 2 1
./prk_swi ../../../dataset/cit-patent.edges ../../../hw/opencl/prk/spmv_swi/bin/prk_swi.aocx  1 0 2 1
./prk_swi ../../../dataset/road-road-usa.mtx ../../../hw/opencl/prk/spmv_swi/bin/prk_swi.aocx  1 0 2 1
./prk_swi ../../../dataset/web-it-2004.mtx ../../../hw/opencl/prk/spmv_swi/bin/prk_swi.aocx  1 0 2 1
./prk_swi ../../../dataset/eco-everglades.edges ../../../hw/opencl/prk/spmv_swi/bin/prk_swi.aocx  1 0 2 1
./prk_swi ../../../dataset/rt-retweet-crawl.mtx ../../../hw/opencl/prk/spmv_swi/bin/prk_swi.aocx  1 0 2 1
./prk_swi ../../../dataset/web-Stanford.mtx ../../../hw/opencl/prk/spmv_swi/bin/prk_swi.aocx  1 0 2 1
./prk_swi ../../../dataset/eco-foodweb-baydry.edges ../../../hw/opencl/prk/spmv_swi/bin/prk_swi.aocx  1 0 2 1
./prk_swi ../../../dataset/socfb-A-anon.mtx ../../../hw/opencl/prk/spmv_swi/bin/prk_swi.aocx  1 0 2 1
./prk_swi ../../../dataset/web-wikipedia2009.mtx ../../../hw/opencl/prk/spmv_swi/bin/prk_swi.aocx  1 0 2 1
./prk_swi ../../../dataset/eco-stmarks.edges ../../../hw/opencl/prk/spmv_swi/bin/prk_swi.aocx  1 0 2 1
./prk_swi ../../../dataset/socfb-Duke14.mtx ../../../hw/opencl/prk/spmv_swi/bin/prk_swi.aocx  1 0 2 1
