export C_INCLUDE_PATH=/opt/intel/inteloneapi/compiler/2021.1.1/linux/lib/oclfpga/host/include/
export CPLUS_INCLUDE_PATH=/opt/intel/inteloneapi/compiler/2021.1.1/linux/lib/oclfpga/host/include/
cd apps/mis
make
./mis_base ../../../dataset/ca-coauthors-dblp.mtx ../../../hw/opencl/mis/mis_cu4/bin/mis_cu4.aocx  1 0 2 1
./mis_base ../../../dataset/socfb-Georgetown15.mtx ../../../hw/opencl/mis/mis_cu4/bin/mis_cu4.aocx  1 0 2 1
./mis_base ../../../dataset/road-roadNet-CA.mtx ../../../hw/opencl/mis/mis_cu4/bin/mis_cu4.aocx  1 0 2 1
./mis_base ../../../dataset/eco-everglades.edges ../../../hw/opencl/mis/mis_cu4/bin/mis_cu4.aocx  1 0 2 1
./mis_base ../../../dataset/eco-foodweb-baydry.edges ../../../hw/opencl/mis/mis_cu4/bin/mis_cu4.aocx  1 0 2 1
./mis_base ../../../dataset/web-wikipedia2009.mtx ../../../hw/opencl/mis/mis_cu4/bin/mis_cu4.aocx  1 0 2 1
./mis_base ../../../dataset/eco-stmarks.edges ../../../hw/opencl/mis/mis_cu4/bin/mis_cu4.aocx  1 0 2 1
./mis_base ../../../dataset/socfb-Duke14.mtx ../../../hw/opencl/mis/mis_cu4/bin/mis_cu4.aocx  1 0 2 1
