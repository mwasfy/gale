export C_INCLUDE_PATH=/opt/intel/inteloneapi/compiler/2021.1.1/linux/lib/oclfpga/host/include/
export CPLUS_INCLUDE_PATH=/opt/intel/inteloneapi/compiler/2021.1.1/linux/lib/oclfpga/host/include/
cd ../apps/sssp
make
./sssp_base ../../../dataset/ca-coauthors-dblp.mtx ../../../hw/opencl/sssp/sssp_base/bin/kernel_csr.aocx  1 0 2 1
./sssp_base ../../../dataset/email-enron-large.mtx ../../../hw/opencl/sssp/sssp_base/bin/kernel_csr.aocx  1 0 2 1
./sssp_base ../../../dataset/socfb-Georgetown15.mtx ../../../hw/opencl/sssp/sssp_base/bin/kernel_csr.aocx  1 0 2 1
./sssp_base ../../../dataset/ca-hollywood-2009.mtx ../../../hw/opencl/sssp/sssp_base/bin/kernel_csr.aocx  1 0 2 1
./sssp_base ../../../dataset/road-asia-osm.mtx ../../../hw/opencl/sssp/sssp_base/bin/kernel_csr.aocx  1 0 2 1
./sssp_base ../../../dataset/socfb-uci-uni.mtx ../../../hw/opencl/sssp/sssp_base/bin/kernel_csr.aocx  1 0 2 1
./sssp_base ../../../dataset/cit-DBLP.edges ../../../hw/opencl/sssp/sssp_base/bin/kernel_csr.aocx  1 0 2 1
./sssp_base ../../../dataset/road-roadNet-CA.mtx ../../../hw/opencl/sssp/sssp_base/bin/kernel_csr.aocx  1 0 2 1
./sssp_base ../../../dataset/web-google.mtx ../../../hw/opencl/sssp/sssp_base/bin/kernel_csr.aocx  1 0 2 1
./sssp_base ../../../dataset/cit-patent.edges ../../../hw/opencl/sssp/sssp_base/bin/kernel_csr.aocx  1 0 2 1
./sssp_base ../../../dataset/road-road-usa.mtx ../../../hw/opencl/sssp/sssp_base/bin/kernel_csr.aocx  1 0 2 1
./sssp_base ../../../dataset/web-it-2004.mtx ../../../hw/opencl/sssp/sssp_base/bin/kernel_csr.aocx  1 0 2 1
./sssp_base ../../../dataset/eco-everglades.edges ../../../hw/opencl/sssp/sssp_base/bin/kernel_csr.aocx  1 0 2 1
./sssp_base ../../../dataset/rt-retweet-crawl.mtx ../../../hw/opencl/sssp/sssp_base/bin/kernel_csr.aocx  1 0 2 1
./sssp_base ../../../dataset/web-Stanford.mtx ../../../hw/opencl/sssp/sssp_base/bin/kernel_csr.aocx  1 0 2 1
./sssp_base ../../../dataset/eco-foodweb-baydry.edges ../../../hw/opencl/sssp/sssp_base/bin/kernel_csr.aocx  1 0 2 1
./sssp_base ../../../dataset/socfb-A-anon.mtx ../../../hw/opencl/sssp/sssp_base/bin/kernel_csr.aocx  1 0 2 1
./sssp_base ../../../dataset/web-wikipedia2009.mtx ../../../hw/opencl/sssp/sssp_base/bin/kernel_csr.aocx  1 0 2 1
./sssp_base ../../../dataset/eco-stmarks.edges ../../../hw/opencl/sssp/sssp_base/bin/kernel_csr.aocx  1 0 2 1
./sssp_base ../../../dataset/socfb-Duke14.mtx ../../../hw/opencl/sssp/sssp_base/bin/kernel_csr.aocx  1 0 2 1
